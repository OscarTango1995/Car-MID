#include <Arduino.h>
#include "average.h"
#include "sd.h"
#include"fuel_level.h"

const int MAX_SIZE = 14;
static float fuelFlowArray[MAX_SIZE];
static double distanceArray[MAX_SIZE];
static int currentIndex = 0;
static int totalEntries = 0;
bool arrayFull = false;
static float totalFuelFlow = 0;
static double totalDistance = 0;
static float average = 0.0;
float R_empty;
float R_full;
float tankCapacity;

Average calculateAverageFuelConsumption()
{
    Serial.println(average);
    if (totalDistance == 0)
    {
        Serial.println("Error: Total distance is zero, cannot calculate average.");
        return Average();
    }

    float totalFuelInLiters = totalFuelFlow / totalEntries;
    float newConsumption = totalDistance / totalFuelInLiters;
    average = (newConsumption - average) / totalEntries;
    Average avg;
    avg.average = average;
    avg.distance = totalDistance;
    avg.fuel = getFuelAvailable(); // get fuel quantity in tank in percentage
    avg.dte = getFuelAvailable() * average;

    // write average to csv
    bool isWritten = writeAvgToCSV(avg);
    if (isWritten)
    {
        return avg;
    }
    else
    {
        Serial.println("Failed to write Average Data to csv");
        return Average();
    }
}

// Function to store values in arrays and check if they are full
void logAverageData(float fuelFlow, double distance)
{
    fuelFlowArray[currentIndex] = fuelFlow;
    distanceArray[currentIndex] = distance;

    totalFuelFlow += fuelFlow;
    totalDistance += distance;
    currentIndex++;
    totalEntries++;

    if (currentIndex == MAX_SIZE)
    {
        arrayFull = true; // Set flag to true when array is full
    }
    else
    {
        arrayFull = false; // Reset the flag if the array is not full
    }
}

bool isArrayFull()
{
    return arrayFull;
}

void resetArray()
{
    arrayFull = false;
    currentIndex = 0;
    average = 0.0;
}

Average resetTrip()
{
    arrayFull = false;
    currentIndex = 0;
    totalFuelFlow = 0;
    totalDistance = 0;
    totalEntries = 0;
    average = 0.0;
    Average avg;
    avg.average = 0.0;
    avg.distance = 0;
    avg.dte = 0.0;
    avg.fuel = 0.0;

    return avg;
}