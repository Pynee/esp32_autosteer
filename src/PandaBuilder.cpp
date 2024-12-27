#include "PandaBuilder.h"

PandaBuilder::PandaBuilder(PGNCommManager *PGNManager)
{
    this->pgnmanager = PGNManager;
}

void PandaBuilder::buildPanda(NMEAMessage *nmeaMessage)
{
    // Don't send nmea if connection isn't ready
    if (!destinationIPSet)
    {
        return;
    }
    /*if (dataToSend.imuAvailable)
    {
        imuHandler->calculateEuler();
    }
    else
    {
        itoa(65535, imuHandler->imuHeading, 10);
    } // Get IMU data ready*/
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
    strcat(nmea, std::to_string(dataToSend.heading).c_str());
    strcat(nmea, ",");

    // 13
    strcat(nmea, std::to_string(dataToSend.roll).c_str());
    strcat(nmea, ",");

    // 14
    strcat(nmea, std::to_string(dataToSend.pitch).c_str());
    strcat(nmea, ",");

    // 15
    strcat(nmea, std::to_string(dataToSend.yawRate).c_str());

    strcat(nmea, "*");

    calculateChecksum(nmea);

    strcat(nmea, "\r\n");

    int len = strlen(nmea);

    QueueItem item = {(uint8_t *)nmea, strlen(nmea)};
    xQueueSend(pgnmanager->managerSendQueue, (void *)&item, (TickType_t)0);
}
// Calculate nmea checksum and append it to the sentence
uint8_t PandaBuilder::calculateChecksum(char *packet)
{

    char *packetptr = packet;
    const char *asciiHex = "0123456789ABCDEF";
    int8_t checksum = 0;
    // Iterate over nmea string XORing to checksum until we reach '*' character
    while (*++packetptr != '*')
    {
        checksum ^= *packetptr;
    }

    char checksumHexString[3] = {asciiHex[checksum >> 4], asciiHex[checksum & 15], '\0'};
    strcat(packet, checksumHexString);
    return checksum;
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