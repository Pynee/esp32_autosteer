#include "UDPPacketManager.h"

UDPPacketManager::UDPPacketManager(void gnssSendData(uint8_t *data, size_t len))
{
  this->gnssSendData = gnssSendData;
}

void UDPPacketManager::steerSettingsInit()
{
  // for PWM High to Low interpolator
  receivedData.highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;
}

bool UDPPacketManager::initUDP()
{
  // sendDatafn = *sendData;
  //  Supress Debug information
  wfm.setDebugOutput(false);

  if (!wfm.autoConnect("ESP32AGOpenGPS"))
  {
    // Did not connect, print error message
    // Serial.println("failed to connect and hit timeout");
    Serial.println("failed to connect and hit timeout");

    // Reset and try again
    ESP.restart();
    delay(1000);
  }

  WiFi.useStaticBuffers(true);
  esp_wifi_set_ps(WIFI_PS_NONE);

  myip = WiFi.localIP();
  // Connected!
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Serial.println("WiFi connected");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  if (udp.listen(AOGAutoSteerPort))
  {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    // Serial.print("UDP Listening on IP: ");
    // Serial.println(WiFi.localIP());
    udp.onPacket([=](AsyncUDPPacket packet)
                 { autoSteerPacketParser(packet); });

    if (ntrip.listen(AOGNtripPort))
    {
      Serial.print("UDP Listening on IP: ");
      Serial.println(WiFi.localIP());
      // Serial.print("UDP Listening on IP: ");
      // Serial.println(WiFi.localIP());
      ntrip.onPacket([=](AsyncUDPPacket packet)
                     { ntripPacketProxy(packet); });
    }
  }
  xTaskCreate(this->startWorkerImpl, "UdpSendDataTask", 3096, NULL, 3, NULL);
  return true;
}

void UDPPacketManager::startWorkerImpl(void *_this)
{
  ((UDPPacketManager *)_this)->sendDataTask(_this);
}

void UDPPacketManager::ntripPacketProxy(AsyncUDPPacket packet)
{
  if (packet.length() > 0)
  {
    // Serial2.write(packet.data(), packet.length());
    gnssSendData(packet.data(), packet.length());
  }
}

void UDPPacketManager::autoSteerPacketParser(AsyncUDPPacket udpPacket)
{
  if (ipDesIsSet && udpPacket.remoteIP() != ipDes || udpPacket.length() <= 0)
  {
    return;
  }

  buffer = udpPacket.data();
  for (int i = 0; i < udpPacket.length(); i++)
  {
    data[bufferIndex] = buffer[i];
    bufferIndex++;
  }
  if (bufferIndex < 4)
  {
    return;
  }
  for (int i = 0; i < bufferIndex; i++)
  {
    if (data[i] == 128 && data[i + 1] == 129)
    {
      int lenght = data[i + 4] + 6;
      if (lenght > i + bufferIndex + 1) //???
      {
        break;
      }
      for (int x = 0; x < lenght; x++)
      {
        data[x] = data[i + x];
      }

      bufferIndex -= lenght;
      i = -1;
      byte CK_A = 0;
      for (int j = 2; j < lenght - 1; j++)
      {
        CK_A += data[j];
      }

      if (data[lenght - 1] != (byte)CK_A)
      {
        Serial.print("Packet: CRC error: ");
        Serial.println(CK_A);
        // Serial.print("Packet: CRC error: ");
        // Serial.print(CK_A);
        printLnByteArray(data, lenght);
        break;
      }

      parsePacket(data, lenght, udpPacket);
    }
  }
}

void UDPPacketManager::parsePacket(uint8_t *packet, int size, AsyncUDPPacket udpPacket)
{

  if (packet[0] == 0x80 && packet[1] == 0x81 && packet[2] == 0x7F) // Data
  {
    switch (packet[3])
    {
    case 0xFE: // Steer Data
    {
      parseSteerData(packet);
      break;
    }
    case 252: // Steer settings
    {         // 0xFC
      parseSteerSettings(packet);
      break;
    }
    case 251: // SteerConfig
    {
      parseSteerConfig(packet);
      break;
    }
    case 200: // Hello Sent To Module
    {

      int16_t sa = (int16_t)(dataToSend.currentSteerAngle * 100);

      helloFromAutoSteer[5] = (uint8_t)sa;
      helloFromAutoSteer[6] = sa >> 8;

      helloFromAutoSteer[7] = (uint8_t)dataToSend.helloSteerPosition;
      helloFromAutoSteer[8] = dataToSend.helloSteerPosition >> 8;
      helloFromAutoSteer[9] = hardwareSwitches.switchByte;
      QueueItem item = {helloFromAutoSteer, sizeof(helloFromAutoSteer)};
      xQueueSend(sendQueue, (void *)&item, (TickType_t)0);
      // QueueItem item = {helloFromIMU, sizeof(helloFromIMU)};
      // xQueueSend(sendQueue, (void *)&item, (TickType_t)0);
    }
    case 202: // Scan Request
    {
      // make really sure this is the reply pgn
      if (packet[4] == 3 && packet[5] == 202 && packet[6] == 202)
      {
        ipDes = udpPacket.remoteIP();
        ipDesIsSet = true;
        // hello from AgIO
        uint8_t scanReply[] = {128, 129, 126, 203, 7,
                               myip[0], myip[1], myip[2], myip[3],
                               myip[0], myip[1], myip[2], 23};

        QueueItem item = {scanReply, sizeof(scanReply)};
        xQueueSend(sendQueue, (void *)&item, (TickType_t)0);
      }
    }
    }
  }
  else
  {
    Serial.print("Unknown packet!!!");
    // Serial.print("Unknown packet!!!");
    printLnByteArray(packet, size);
  }
}

void UDPPacketManager::sendDataTask(void *z)
{
  if (sendQueue == NULL)
  {
    Serial.print("queue creation failed!!");
    // Serial.print("queue creation failed!!");
    //  queue creation failed!!
  }
  for (;;)
  {
    while (uxQueueMessagesWaiting(sendQueue))
    {
      QueueItem *queueItem = nullptr;

      if (xQueueReceive(sendQueue, &queueItem, 0) == pdTRUE)
      {
        AsyncUDPMessage m = AsyncUDPMessage(queueItem->length);
        int16_t CK_A = 0;
        for (uint8_t i = 2; i < queueItem->length - 1; i++)
        {
          CK_A = (CK_A + queueItem->data[i]);
        }
        queueItem->data[queueItem->length - 1] = CK_A;

        m.write(queueItem->data, queueItem->length);
        udp.sendTo(m, ipDes, DestinationPort);
        delete queueItem;
      }
    }
  }
}

void UDPPacketManager::printLnByteArray(uint8_t *data, uint8_t datalen)
{
  for (int i = 0; i < datalen; i++)
  {
    Serial.print(data[i]);
    Serial.print(" ");
    // Serial.print(data[i]);
    // Serial.print(" ");
  }
  // Serial.println();
  Serial.println();
}

void UDPPacketManager::parseSteerData(uint8_t *packet)
{
  receivedData.gpsSpeed = ((float)(packet[5] | packet[6] << 8)) * 0.1;
  gpsSpeedUpdateTimer = 0;

  // prev = guidanceStatus;
  receivedData.guidanceStatusByte = packet[7];
  state.guidanceStatus = bitRead(receivedData.guidanceStatusByte, 0);
  // guidanceStatus = packet[7];
  // guidanceStatusChanged = (guidanceStatus != prevGuidanceStatus);

  // Bit 8,9    set point steer angle * 100 is sent
  receivedData.steerTargetAngle = ((float)(packet[8] | ((int8_t)packet[9]) << 8)) * 0.01; // combine high and low bytes

  // Bit 10 Tram
  tram = packet[10];

  // Bit 11
  relay = packet[11];

  // Bit 12
  relayHi = packet[12];

  //----------------------------------------------------------------------------
  // Serial Send to agopenGPS

  int16_t sa = (int16_t)(dataToSend.currentSteerAngle * 100);

  PGN_253[5] = (uint8_t)sa;
  PGN_253[6] = sa >> 8;

  // heading
  PGN_253[7] = (uint8_t)9999;
  PGN_253[8] = 9999 >> 8;

  // roll
  PGN_253[9] = (uint8_t)8888;
  PGN_253[10] = 8888 >> 8;

  PGN_253[11] = hardwareSwitches.switchByte;
  PGN_253[12] = (uint8_t)dataToSend.pwmDisplay;

  QueueItem item = {PGN_253, sizeof(PGN_253)};
  xQueueSend(sendQueue, (void *)&item, (TickType_t)0);

  // Steer Data 2 -------------------------------------------------
  if (steerConfig.PressureSensor || steerConfig.CurrentSensor)
  {
    if (aog2Count++ > 2)
    {
      // Send fromAutosteer2
      PGN_250[5] = (steerConfig.PressureSensor) ? (byte)(sensors.loadSensor.getValue() * 0.25) : (byte)(abs(775 - sensors.loadSensor.getValue()) * 0.5);

      QueueItem item = {PGN_250, sizeof(PGN_250)};
      xQueueSend(sendQueue, (void *)&item, (TickType_t)0);
      aog2Count = 0;
    }
  }
}

void UDPPacketManager::parseSteerSettings(uint8_t *packet)
{
  // PID values
  steerSettings.Kp = ((float)packet[5]);   // read Kp from AgOpenGPS
  steerSettings.highPWM = packet[6];       // read high pwm
  steerSettings.lowPWM = (float)packet[7]; // read lowPWM from AgOpenGPS
  steerSettings.minPWM = packet[8];        // read the minimum amount of PWM for instant on
  float temp = (float)steerSettings.minPWM * 1.2;
  steerSettings.lowPWM = (byte)temp;
  steerSettings.steerSensorCounts = packet[9];  // sent as setting displayed in AOG
  steerSettings.wasOffset = (packet[10]);       // read was zero offset Lo
  steerSettings.wasOffset |= (packet[11] << 8); // read was zero offset Hi
  steerSettings.AckermanFix = (float)packet[12] * 0.01;

  // crc
  // autoSteerUdpData[13];

  // store in EEPROM
  EEPROM.put(10, steerSettings);
  EEPROM.commit();
  // Re-Init steer settings
  steerSettingsInit();
}
void UDPPacketManager::parseSteerConfig(uint8_t *packet)
{
  uint8_t sett = packet[5]; // setting0
  steerConfig.InvertWAS = bitRead(sett, 0) ? 1 : 0;
  steerConfig.IsRelayActiveHigh = bitRead(sett, 1) ? 1 : 0;
  steerConfig.MotorDriveDirection = bitRead(sett, 2) ? 1 : 0;
  steerConfig.SingleInputWAS = bitRead(sett, 3) ? 1 : 0;
  steerConfig.CytronDriver = bitRead(sett, 4) ? 1 : 0;
  steerConfig.SteerSwitch = bitRead(sett, 5) ? 1 : 0;
  steerConfig.SteerButton = bitRead(sett, 6) ? 1 : 0;
  steerConfig.ShaftEncoder = bitRead(sett, 7) ? 1 : 0;
  steerConfig.PulseCountMax = packet[6];
  // was speed
  // autoSteerUdpData[7];
  sett = packet[8]; // setting1 - Danfoss valve etc
  steerConfig.IsDanfoss = bitRead(sett, 0) ? 1 : 0;
  steerConfig.PressureSensor = bitRead(sett, 0) ? 1 : 0;
  steerConfig.CurrentSensor = bitRead(sett, 0) ? 1 : 0;
  steerConfig.IsUseY_Axis = bitRead(sett, 0) ? 1 : 0;

  // crc
  // autoSteerUdpData[13];

  EEPROM.put(40, steerConfig);
  // Re-Init
}