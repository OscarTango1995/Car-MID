#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "temperature.h"
#include "display.h"
#include "sd.h"
#include "altitude.h"
#include "buzzer.h"
#include "remote.h"
#include "elm.h"
#include "gps.h"
#include "average.h"
#include"fuel_level.h"

bool firstAltitudeReading = true;
bool selected = false;
bool averageUpdated = false;
unsigned long lastInteractionMillis = 0; // Timer for user interaction
unsigned long previousTempMillis = 0;    // Stores last time temp readings were updated
unsigned long previousCoolantMillis = 0; // Stores last time coolant readings were updated
unsigned long previousAvgMillis = 0;     // Stores last time avg readings were updated
unsigned long previousLogMillis = 0;     // Stores last time data was logged
const long logInterval = 60000;          // Interval for logging data (1 minute)
const long tempInterval = 30000;         // Interval for displaying temperature readings (30 seconds)
const long tempSelection = 5000;         // Interval for displaying temperature
const long avgInterval = 30000;          // Interval for displaying the average screen (30 seconds)
int currentMenu = 0;
static bool isStarted = true;
bool isMenuActive = true;
bool menuDrawn = false; // Flag to check if the menu has already been drawn
void setup()
{
    Serial.begin(9600);
    delay(1000);

    // initialize Sensors & Displays & Mutex
    initSDCard();
    // initializeBluetooth(); 
    initAltitude();
    initDisplays();
    initTempSensors();
    initIRSensor();
    // initFuelLevelSensor();
    // initGPS();
    initBuzzer();
    turnBuzzerOn(true);
    Serial.print("init done");
    delay(150);
}

void loop()
{
    // getFuelAvailable();
    // delay(1000);
    decodeIR();
    unsigned long currentMillis = millis(); // Get the current time
    unsigned long currentAvgMillis = millis();

    switch (currentMenu)
    {
    case 0:
        isStarted = true;
        if (!menuDrawn)
        {
            drawMenu(0);
            delay(100);
            Average lastAvg = readLastAverageData();
            drawAvgScreen(lastAvg.average, lastAvg.distance, lastAvg.fuel, lastAvg.dte);
            menuDrawn = true;
        }
        break;
    case 1:
        if (isStarted)
        {
            GPSData gpsData = getGPSData();
            drawGPSScreen(false, gpsData.sat, gpsData.speed, gpsData.fix);
            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            if (averageUpdated)
            {
                GPSData gpsData = getGPSData();
                drawGPSScreen(false, gpsData.sat, gpsData.speed, gpsData.fix);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                GPSData gpsData = getGPSData();
                drawGPSScreen(true, gpsData.sat, gpsData.speed, gpsData.fix);
            }
        }
        break;
    case 2:
        if (isStarted)
        {
            // float coolantTemp = readCoolantTemp();
            float coolantTemp = 10.0;
            delay(200);
            Temperatures temp = getTemperatures();
            drawTemperaturesScreen(temp, false, coolantTemp);

            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            Temperatures temp = getTemperatures();
            // float coolantTemp = readCoolantTemp();
            delay(200);
            float coolantTemp = 10.0;
            if (averageUpdated)
            {
                drawTemperaturesScreen(temp, false, coolantTemp);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                Temperatures temp = getTemperatures();
                drawTemperaturesScreen(temp, true, coolantTemp);
            }
        }
        break;
    case 3:
        if (isStarted)
        {
            Altitude altitude = getAltitude();
            Temperatures temp = getTemperatures();
            drawAltitudeScreen(altitude, temp, false);
            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            Temperatures temp = getTemperatures();
            if (averageUpdated)
            {
                Altitude altitude = getAltitude(); // Get the second reading
                delay(150);
                drawAltitudeScreen(altitude, temp, false);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                Altitude altitude = getAltitude();
                drawAltitudeScreen(altitude, temp, true);
            }
        }
        break;
    case 4:
        if (isStarted)
        {
            if (averageUpdated)
            {
                drawServiceScreen();
                averageUpdated = false;
            }
            else
            {
                drawServiceScreen();
            }
            isStarted = false;
        }
        break;

    default:
        if (!menuDrawn)
        {
            drawMenu(0);
            menuDrawn = true;
        }
        break;
    }

    if (!selected && currentMillis >= tempSelection)
    {
        currentMenu = 2;
        selected = true;
    }

    if (currentMillis - previousLogMillis >= logInterval)
    {
        previousLogMillis = currentMillis;
        if (isArrayFull())
        {
            Serial.print("Calculating Average Data");
            Average avg = calculateAverageFuelConsumption();
            drawAvgScreen(avg.average, avg.distance, avg.fuel, avg.dte);
            averageUpdated = true;
            resetArray();
        }

        else
        {
            float fuelFlow = readFuelFlow();
            double distance = updateGPSAndCalculateDistance();
            logAverageData(fuelFlow,distance);
        }
    }

    if (currentMillis - previousCoolantMillis >= tempInterval)
    {
        previousCoolantMillis = currentMillis;
        // float coolantTemp = readCoolantTemp();
        delay(200);
        float coolantTemp = 10.0;
        if (coolantTemp > 98)
        {
           turnBuzzerOn(false);
        }
    }
}