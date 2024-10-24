#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include"gps.h"


// Create GPS object
TinyGPSPlus gps;

// Use HardwareSerial to communicate with the GPS module
HardwareSerial gpsSerial(1); // UART1
int RXPin = 17;              // Connect this to TX of GPS
int TXPin = 5;               // Connect this to RX of GPS (if needed)

// Variables to store the last known position
double lastLatitude = 0.0;
double lastLongitude = 0.0;

// Variable to hold the last distance traveled during the current minute
double lastDistance = 0.0;

const long TIMEZONE_OFFSET = 5 * 3600; // 5 hours in seconds

void initGPS()
{
    gpsSerial.begin(9600, SERIAL_8N1, RXPin, TXPin); // Start GPS communication
}

double calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    const double earthRadiusKm = 6371.0; // Earth's radius in kilometers

    // Convert degrees to radians
    double dLat = (lat2 - lat1) * (M_PI / 180.0);
    double dLon = (lon2 - lon1) * (M_PI / 180.0);

    // Haversine formula to calculate the distance
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * (M_PI / 180.0)) * cos(lat2 * (M_PI / 180.0)) *
                   sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return earthRadiusKm * c; // Distance in kilometers
}

double updateGPSAndCalculateDistance()
{
    // Read data from GPS module
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());

        // If new location data is available, calculate the distance
        if (gps.location.isUpdated())
        {
            double currentLatitude = gps.location.lat();
            double currentLongitude = gps.location.lng();

            // Calculate distance only if we have a previous position
            if (lastLatitude != 0.0 && lastLongitude != 0.0)
            {
                // Calculate distance from the last known position
                lastDistance = calculateDistance(lastLatitude, lastLongitude, currentLatitude, currentLongitude);
            }

            // Update the last known position
            lastLatitude = currentLatitude;
            lastLongitude = currentLongitude;

            // Return the distance traveled in this update (not cumulative)
            return lastDistance; // Returning distance in kilometers for this interval
        }
    }

    return 0.0; // Return 0 if no new location is updated
}

String getCurrentTimeStamp()
{
    // Read GPS data
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    if (gps.time.isValid())
    {
        // Get current UTC time from GPS
        unsigned long utcTime = gps.time.hour() * 3600 + gps.time.minute() * 60 + gps.time.second();

        // Apply the timezone offset
        unsigned long localTime = utcTime + TIMEZONE_OFFSET;

        // Calculate hours, minutes, and seconds for local time
        int localHour = (localTime / 3600) % 24;
        int localMinute = (localTime % 3600) / 60;
        int localSecond = localTime % 60;

        // Handle date change if timezone adjustment changes the day
        int day = gps.date.day();
        int month = gps.date.month();
        int year = gps.date.year();

        if (localHour < 0)
        {
            localHour += 24;
            day -= 1; // Go back a day if local time goes negative
        }
        else if (localHour >= 24)
        {
            localHour -= 24;
            day += 1; // Move to the next day if the time crosses midnight
        }

        // Format the time and date as hh:mm - day-month-year
        char timeStamp[30];
        snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d - %02d-%02d-%04d", localHour, localMinute, day, month, year);

        return String(timeStamp);
    }
    else
    {
        Serial.println("GPS date or time not valid yet.");
        return "";
    }
}

GPSData getGPSData()
{
    GPSData gpsData;
    gpsData.sat = 0;
    gpsData.fix = 0;
    gpsData.speed = 0;
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    if (gps.satellites.isValid())
    {
        gpsData.sat = gps.satellites.value();
        gpsData.fix = gps.location.isValid() ? (gps.satellites.value() >= 4 ? 3 : 2) : 0;
        gpsData.speed = gps.speed.kmph();
        return gpsData;
    }
    else
    {
        return gpsData;
    }
}