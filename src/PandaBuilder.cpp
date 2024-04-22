#include "PandaBuilder.h"

PandaBuilder::PandaBuilder(UDPPacketManager *packetmanager, IMUHandler *imuHandler)
{
    this->packetManager = packetManager;
    this->imuHandler = imuHandler;
}

void PandaBuilder::buildPanda(NMEAMessage *nmeaMessage)
{
    if (imuHandler->useBNO08x)
    {
        imuHandler->calculateEuler();
    }
    else
    {
        itoa(65535, imuHandler->imuHeading, 10);
    } // Get IMU data ready
    strcpy(nmea, "");

    strcat(nmea, "$PANDA,");

    strcat(nmea, nmeaMessage->fixTime);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->latitude);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->latNS);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->longitude);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->lonEW);
    strcat(nmea, ",");

    // 6
    strcat(nmea, nmeaMessage->fixQuality);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->numSats);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->HDOP);
    strcat(nmea, ",");

    strcat(nmea, nmeaMessage->altitude);
    strcat(nmea, ",");

    // 10
    strcat(nmea, nmeaMessage->ageDGPS);
    strcat(nmea, ",");

    // 11
    strcat(nmea, nmeaMessage->speedKnots);
    strcat(nmea, ",");

    // 12
    strcat(nmea, imuHandler->imuHeading);
    strcat(nmea, ",");

    // 13
    strcat(nmea, imuHandler->imuRoll);
    strcat(nmea, ",");

    // 14
    strcat(nmea, imuHandler->imuPitch);
    strcat(nmea, ",");

    // 15
    strcat(nmea, imuHandler->imuYawRate);

    strcat(nmea, "*");

    CalculateChecksum();

    strcat(nmea, "\r\n");

    int len = strlen(nmea);
    {
        /* Send a pointer to a struct AMessage object.  Don't block if the
        queue is already full. */
        QueueItem item = {(uint8_t *)nmea, strlen(nmea)};
        xQueueSend(packetManager->sendQueue, (void *)&item, (TickType_t)0);
    }
    //(*sendDatafn)((uint8_t *)nmea, len);
}

void PandaBuilder::CalculateChecksum(void)
{
    int16_t sum = 0;
    int16_t inx = 0;
    char tmp;

    // The checksum calc starts after '$' and ends before '*'
    for (inx = 1; inx < 100; inx++)
    {
        tmp = nmea[inx];

        // * Indicates end of data and start of checksum
        if (tmp == '*')
        {
            break;
        }

        sum ^= tmp; // Build checksum
    }

    byte chk = (sum >> 4);
    char hex[2] = {asciiHex[chk], 0};
    strcat(nmea, hex);

    chk = (sum % 16);
    char hex2[2] = {asciiHex[chk], 0};
    strcat(nmea, hex2);
}

/*
  $PANDA
  (1) Time of fix
  position
  (2,3) 4807.038,N Latitude 48 deg 07.038' N
  (4,5) 01131.000,E Longitude 11 deg 31.000' E
  (6) 1 Fix quality:
    0 = invalid
    1 = GPS fix(SPS)
    2 = DGPS fix
    3 = PPS fix
    4 = Real Time Kinematic
    5 = Float RTK
    6 = estimated(dead reckoning)(2.3 feature)
    7 = Manual input mode
    8 = Simulation mode
  (7) Number of satellites being tracked
  (8) 0.9 Horizontal dilution of position
  (9) 545.4 Altitude (ALWAYS in Meters, above mean sea level)
  (10) 1.2 time in seconds since last DGPS update
  (11) Speed in knots
  FROM IMU:
  (12) Heading in degrees
  (13) Roll angle in degrees(positive roll = right leaning - right down, left up)
  (14) Pitch angle in degrees(Positive pitch = nose up)
  (15) Yaw Rate in Degrees / second
  CHKSUM
*/

/*
  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M ,  ,*47
   0     1      2      3    4      5 6  7  8   9    10 11  12 13  14
        Time      Lat       Lon     FixSatsOP Alt
  Where:
     GGA          Global Positioning System Fix Data
     123519       Fix taken at 12:35:19 UTC
     4807.038,N   Latitude 48 deg 07.038' N
     01131.000,E  Longitude 11 deg 31.000' E
     1            Fix quality: 0 = invalid
                               1 = GPS fix (SPS)
                               2 = DGPS fix
                               3 = PPS fix
                               4 = Real Time Kinematic
                               5 = Float RTK
                               6 = estimated (dead reckoning) (2.3 feature)
                               7 = Manual input mode
                               8 = Simulation mode
     08           Number of satellites being tracked
     0.9          Horizontal dilution of position
     545.4,M      Altitude, Meters, above mean sea level
     46.9,M       Height of geoid (mean sea level) above WGS84
                      ellipsoid
     (empty field) time in seconds since last DGPS update
     (empty field) DGPS station ID number
      47          the checksum data, always begins with
  $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
  0      1    2   3      4    5      6   7     8     9     10   11
        Time      Lat        Lon       knots  Ang   Date  MagV
  Where:
     RMC          Recommended Minimum sentence C
     123519       Fix taken at 12:35:19 UTC
     A            Status A=active or V=Void.
     4807.038,N   Latitude 48 deg 07.038' N
     01131.000,E  Longitude 11 deg 31.000' E
     022.4        Speed over the ground in knots
     084.4        Track angle in degrees True
     230394       Date - 23rd of March 1994
     003.1,W      Magnetic Variation
      6A          The checksum data, always begins with
  $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
    VTG          Track made good and ground speed
    054.7,T      True track made good (degrees)
    034.4,M      Magnetic track made good
    005.5,N      Ground speed, knots
    010.2,K      Ground speed, Kilometers per hour
     48          Checksum
*/