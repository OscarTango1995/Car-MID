#ifndef SD_H
#define SD_H
#include <Arduino.h>
#include "average.h"

struct Total
{
    float totalFuelUsed;
    float totalDistanceTraveled;
};
struct GPS
{
    double latitude;
    double longitude;
    double distance;
};
struct ServiceEntry {
  String type;        // ENG, GEAR, CLT, TIM, BAT
  unsigned long mileage; // If available, e.g. 183000
  String date;        // Date in YYYY-MM-DD
};

void initSDCard(bool firstTime);
void initLineCountFile();
void initAverageFile();
void initGPSFile();
void initHUDFile();
void initServiceFile();
void initCurrentFuelFile();
void initTripTotalFile();

bool writeAvgToCSV(Average avg);
void writeLineCount();
void writeCurrentFuel(float currentFuel);
void writeGPSToCSV(double latitude, double longitude, double distance);
void writeHUDBrightness(int brightnessLevel);
void updateServiceEntry(int index);
void updateTripTotalToCSV(float fuelUsed);

Average readLastAverageData();
int readLineCount();
GPS readLastCoordinates();
int readHUDBrightness();
void readServiceData();
float readCurrentFuel();
Total readTripTotalData();

void resetAverageFile();
void resetLineCount();
void resetGPSFile();
void resetCurrentFuel();
void resetTripTotalFile();

void deleteAllFiles();
int getKmAdder(int index);

#endif // LOGGER_H
