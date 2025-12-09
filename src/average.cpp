#include <Arduino.h>
#include "average.h"
#include "sd.h"
#include "fuel_level.h"

const int MAX_SIZE = 10;
static bool entriesFull = false;

Average calculateAverageFuelConsumption()
{
    Average lastAvg = readLastAverageData();
    Total tripTotal = readTripTotalData();

    if (tripTotal.totalDistanceTraveled == 0)
    {
        Serial.println("Error: Total distance is zero, cannot calculate average.");
        return lastAvg;
    }
    if (tripTotal.totalFuelUsed == 0)
    {
        Serial.println("Error: Total fuel used is zero, can't compute average.");
        return lastAvg;
    }

    float currentFuel = readCurrentFuel();
    Average newAvg;
    newAvg.average = (tripTotal.totalDistanceTraveled) / (tripTotal.totalFuelUsed);
    newAvg.distanceTraveled = tripTotal.totalDistanceTraveled;
    newAvg.fuelUsed = tripTotal.totalFuelUsed;
    newAvg.fuelRemaining = currentFuel - tripTotal.totalFuelUsed;
    newAvg.dte = newAvg.fuelRemaining * newAvg.average;

    // write average to csv
    bool isWritten = writeAvgToCSV(newAvg);
    if (isWritten)
    {
        resetLineCount();
        resetAvgReady();
        return newAvg;
    }

    else
    {
        Serial.println("Failed to write Average Data to csv");
        return lastAvg;
    }
}

bool isAvgReady()
{
    int lineCount = readLineCount();

    if (lineCount >= 15)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void resetAvgReady()
{
    entriesFull = false;
}

Average resetTrip()
{
    Average avg = {0.0, 0.0, 0.0, 0.0, 0.0};
    resetAvgReady();
    deleteAllFiles();
    writeAvgToCSV(avg);
    writeCurrentFuel(getFuel());

    return avg;
}