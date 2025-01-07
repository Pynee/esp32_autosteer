#include "PGNParser.h"
// Forward declaration
#include "PGNCommManager.h"

PGNParser::PGNParser(PGNCommManager *commManager)
{
    this->pgnCommManager = commManager;
}
void PGNParser::steerSettingsInit()
{
    // for PWM High to Low interpolator
    receivedData.highLowPerDeg = ((float)(steerSettings.highPWM - steerSettings.lowPWM)) / LOW_HIGH_DEGREES;
}

void PGNParser::packetParserTask(void *z)
{
    if (parseQueue == NULL)
    {
        Serial.print("queue creation failed!!");
        // Serial.print("queue creation failed!!");
        //  queue creation failed!!
    }
    for (;;)
    {
        // while (uxQueueMessagesWaiting(parseQueue))
        //{
        QueueItem queueItem;
        // std::string *receivedString = nullptr;

        if (xQueueReceive(parseQueue, &queueItem, portMAX_DELAY) == pdTRUE)
        {
            // Serial.println("Received!!");
            /*printLnByteArray(queueItem->data, queueItem->length);
            if (queueItem->data[0] == 128 && queueItem->data[1] == 129)
            {
                if (queueItem->data[queueItem->length - 1] != calculateChecksum((uint8_t *)queueItem->data, 2, queueItem->length - 1))
                {
                    Serial.print("Packet: CRC error: ");
                    Serial.println(calculateChecksum((uint8_t *)queueItem->data, 2, queueItem->length - 1));
                    printLnByteArray((uint8_t *)queueItem->data, queueItem->length);
                }
                else
                {
                    Serial.println("Parsing...");
                    parsePacket((uint8_t *)queueItem->data, queueItem->length);
                }
            }*/
            // delete receivedString;
        }
        //}
    }
}

void PGNParser::parsePacket(uint8_t *packet, unsigned int size)
{
    size = min(size, (unsigned int)packet[4] + 6);

    if (packet[0] != 128 || packet[1] != 129)
    {
        return;
    }

    if (packet[size - 1] != 71 && packet[size - 1] != calculateChecksum((uint8_t *)packet, 2, size))
    {
        Serial.println(min(size, (unsigned int)packet[4] + 6));
        Serial.print("Packet ''CRC'' error: correct = ");
        Serial.print(packet[size - 1]);
        Serial.print(", calculated = ");
        Serial.println(calculateChecksum((uint8_t *)packet, 2, size));
        printLnByteArray((uint8_t *)packet, size);
    }

    if (packet[0] == 0x80 && packet[1] == 0x81 && packet[2] == 0x7F) // Data
    {
        switch (packet[3])
        {
        case 254: // Steer Data
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

            int16_t convertedSteerAngle = (int16_t)(dataToSend.currentSteerAngle * 100); // convert value to int16_t and multiply by 100 so for example 14.32° converts to 1432
            helloFromAutoSteer[5] = (uint8_t)convertedSteerAngle;
            helloFromAutoSteer[6] = convertedSteerAngle >> 8;

            helloFromAutoSteer[7] = (uint8_t)dataToSend.helloSteerPosition;
            helloFromAutoSteer[8] = dataToSend.helloSteerPosition >> 8;
            helloFromAutoSteer[9] = hardwareSwitches.switchByte;
            QueueItem item = {helloFromAutoSteer, sizeof(helloFromAutoSteer)};
            xQueueSend(pgnCommManager->managerSendQueue, &item, (TickType_t)0);
            if (dataToSend.imuAvailable)
            {
                // Heart beat hello AgIO

                QueueItem imuitem = {helloFromIMU, sizeof(helloFromIMU)};
                // Serial.println(reinterpret_cast<intptr_t>(helloFromIMU + 11));
                // Serial.println(helloFromIMU[10]);
                xQueueSend(pgnCommManager->managerSendQueue, &imuitem, (TickType_t)0);
            }
            // QueueItem item = {helloFromIMU, sizeof(helloFromIMU)};
            // xQueueSend(sendQueue, (void *)&item, (TickType_t)0);
            break;
        }
        case 201: // Subnet changed not doing anything with this yet

            break;

        case 202: // Scan Request
        {
            // Serial.println("Scan Request");
            //  make really sure this is the reply pgn
            if (packet[4] == 3 && packet[5] == 202 && packet[6] == 202)
            {

                // hello from AgIO
                for (int i = 0; i < 4; i++)
                {
                    scanReply[i + 5] = deviceIP[i];
                    scanReply[i + 9] = deviceIP[i];
                }
                scanReply[12] = 23;
                QueueItem item = {(uint8_t *)scanReply, sizeof(scanReply)};
                xQueueSend(pgnCommManager->managerSendQueue, &item, (TickType_t)0);
                break;
            }
        }
        case 100: // Corrected location fix2fix lat lon but we ignore this atleast for now
            break;

        case 229: // section data for machine
        case 239: // machine data
            break;

        default:
            Serial.println("Unknown packet!!!");
            printLnByteArray(packet, size);
            break;
        }
    }
    else
    {
        Serial.println("Unknown packet!");
        // Serial.print("Unknown packet!!!");
        printLnByteArray(packet, size);
    }
}

void PGNParser::printLnByteArray(uint8_t *data, uint8_t datalen)
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

void PGNParser::parseSteerData(uint8_t *packet)
{

    receivedData.gpsSpeed = ((float)(packet[5] | packet[6] << 8)) * 0.1;
    // gpsSpeedUpdateTimer = 0;

    // prev = guidanceStatus;
    receivedData.guidanceStatusByte = packet[7];
    state.guidanceStatus = bitRead(receivedData.guidanceStatusByte, 0);
    // guidanceStatus = packet[7];
    // guidanceStatusChanged = (guidanceStatus != prevGuidanceStatus);

    // Bit 8,9    set point steer angle * 100 is sent
    receivedData.steerTargetAngle = ((float)(packet[8] | ((int8_t)packet[9]) << 8)) * 0.01; // combine high and low bytes

    // Bit 10 Tram
    // receivedData.sectionRelayBytes = packet[10];

    receivedData.sectionRelayBytes = (packet[11] | (packet[12]) << 8); // Store sectionRelay states;
    state.lastAOGUpdate = millis();
    //----------------------------------------------------------------------------
    // Serial Send to agopenGPS

    int16_t convertedSteerAngle = (int16_t)(dataToSend.currentSteerAngle * 100);
    // fromAutoSteerData FD 253 - ActualSteerAngle*100 -5,6, SwitchByte-7, pwmDisplay-8

    PGN_253[5] = (uint8_t)convertedSteerAngle;
    PGN_253[6] = convertedSteerAngle >> 8;

    // heading
    PGN_253[7] = (uint8_t)dataToSend.heading;
    PGN_253[8] = (uint8_t)dataToSend.heading >> 8;

    // roll
    PGN_253[9] = (uint8_t)dataToSend.roll;
    PGN_253[10] = (uint8_t)dataToSend.roll >> 8;

    PGN_253[11] = hardwareSwitches.switchByte;
    PGN_253[12] = (uint8_t)dataToSend.pwmDisplay;

    PGN_253[sizeof(PGN_253) - 1] = calculateChecksum(PGN_253, 2, sizeof(PGN_253)); // Add checksum to packet

    QueueItem item = {(uint8_t *)PGN_253, sizeof(PGN_253)};

    xQueueSend(pgnCommManager->managerSendQueue, &item, (TickType_t)0);

    // Steer Data 2 -------------------------------------------------
    if (steerConfig.PressureSensor || steerConfig.CurrentSensor)
    {
        // fromAutoSteerData FA 250 - sensor values etc

        // if (aog2Count++ > 2)
        //{
        //  Send fromAutosteer2
        PGN_250[5] = (steerConfig.PressureSensor) ? (uint8_t)(sensors.loadSensor.getValue() * 0.25) : (uint8_t)(abs(775 - sensors.loadSensor.getValue()) * 0.5); //???
        PGN_250[sizeof(PGN_250) - 1] = calculateChecksum(PGN_250, 2, sizeof(PGN_250));                                                                           // Add checksum to packet
        QueueItem item = {PGN_250, sizeof(PGN_250)};
        xQueueSend(pgnCommManager->managerSendQueue, &item, (TickType_t)0);
        // aog2Count = 0;
        // }
    }
}

void PGNParser::parseSteerSettings(uint8_t *packet)
{
    // PID values
    steerSettings.Kp = ((float)packet[5]);   // read Kp from AgOpenGPS
    steerSettings.highPWM = packet[6];       // read high pwm
    steerSettings.lowPWM = (float)packet[7]; // read lowPWM from AgOpenGPS
    steerSettings.minPWM = packet[8];        // read the minimum amount of PWM for instant on
    float temp = (float)steerSettings.minPWM * 1.2;
    steerSettings.lowPWM = (byte)temp;
    steerSettings.steerSensorCounts = packet[9];                            // sent as setting displayed in AOG
    steerSettings.wasOffset = ((packet[10]) | ((uint16_t)packet[11] << 8)); // read was zero offset Lo  read was zero offset Hi
    steerSettings.AckermanFix = (float)packet[12] * 0.01;

    // crc
    // autoSteerUdpData[13];

    // store in EEPROM
    EEPROM.put(10, steerSettings);
    EEPROM.commit();
    // Re-Init steer settings
    steerSettingsInit();
}
void PGNParser::parseSteerConfig(uint8_t *packet)
{
    uint8_t settingByte = packet[5]; // setting0
    steerConfig.InvertWAS = bitRead(settingByte, 0) ? 1 : 0;
    steerConfig.IsRelayActiveHigh = bitRead(settingByte, 1) ? 1 : 0;
    steerConfig.MotorDriveDirection = bitRead(settingByte, 2) ? 1 : 0;
    steerConfig.SingleInputWAS = bitRead(settingByte, 3) ? 1 : 0;
    steerConfig.CytronDriver = bitRead(settingByte, 4) ? 1 : 0;
    steerConfig.SteerSwitch = bitRead(settingByte, 5) ? 1 : 0;
    steerConfig.SteerButton = bitRead(settingByte, 6) ? 1 : 0;
    steerConfig.ShaftEncoder = bitRead(settingByte, 7) ? 1 : 0;
    steerConfig.PulseCountMax = packet[6];
    // was speed
    // autoSteerUdpData[7];
    settingByte = packet[8]; // setting1 - Danfoss valve etc
    steerConfig.IsDanfoss = bitRead(settingByte, 0) ? 1 : 0;
    steerConfig.PressureSensor = bitRead(settingByte, 1) ? 1 : 0;
    steerConfig.CurrentSensor = bitRead(settingByte, 2) ? 1 : 0;
    steerConfig.IsUseY_Axis = bitRead(settingByte, 3) ? 1 : 0;

    // crc
    // autoSteerUdpData[13];

    EEPROM.put(40, steerConfig);
    // Re-Init
}
uint8_t PGNParser::calculateChecksum(uint8_t *packet, int startPos, int stopPos)
{
    // checksum
    int16_t CK_A = 0;
    // Serial.print("CRC: ");
    for (uint8_t i = startPos; i < stopPos - 1; i++)
    {
        CK_A = (CK_A + packet[i]);
        // Serial.print(CK_A);
        // Serial.print(", ");
    }
    // Serial.println();
    return (uint8_t)CK_A;
}