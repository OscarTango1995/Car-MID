#ifndef AVERAGE_H
#define AVERAGE_H
#include <Arduino.h>

struct Average
{
    float average;
    int distance;
    float fuel;
    float dte;
};

Average calculateAverageFuelConsumption();
void logAverageData(float fuelFlow, double distance);
bool isArrayFull();
void resetArray();
Average resetTrip();

#endif // AVERAGE_H
