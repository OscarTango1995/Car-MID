#ifndef GPS_H
#define GPS_H

struct GPSData
{
    int sat;
    int speed;
    int fix;
};

void initGPS();
double updateGPSAndCalculateDistance();
double calculateDistance(double lat1, double lon1, double lat2, double lon2);
String getCurrentTimeStamp();
GPSData getGPSData();
String getCurrentDate();
#endif // GPS_H
