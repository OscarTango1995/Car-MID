#ifndef AVERAGE_H
#define AVERAGE_H
#include <Arduino.h>

struct Average
{
    float average;
    float distanceTraveled;
    float fuelUsed;
    float fuelRemaining;
    float dte;
};

Average calculateAverageFuelConsumption();
bool isAvgReady();
void resetAvgReady();
Average resetTrip();

#endif // AVERAGE_H
