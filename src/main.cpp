#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "temperature.h"
#include "display.h"
#include "logger.h"
#include "altitude.h"
#include "buzzer.h"
#include "remote.h"
#include "elm.h"

bool firstAltitudeReading = true;
bool selected = false;
bool averageUpdated = false;
unsigned long lastInteractionMillis = 0; // Timer for user interaction
unsigned long previousTempMillis = 0;    // Stores last time temp readings were updated
unsigned long previousAvgMillis = 0;     // Stores last time avg readings were updated
const long tempInterval = 30000;         // Interval for displaying temperature readings (30 seconds)
const long tempSelection = 5000;         // Interval for displaying temperature
const long avgInterval = 75000;          // Interval for displaying the average screen (30 seconds)
int currentMenu = 0;
static bool isStarted = true;
bool isMenuActive = true;
bool menuDrawn = false; // Flag to check if the menu has already been drawn

void setup()
{
    Serial.begin(9600);
    delay(1000);

    // initialize Sensors & Displays & Mutex
    // initializeBluetooth(); // Initialize Bluetooth
    initAltitude();
    initDisplays();
    initTempSensors();
    // initLogging();
    initIRSensor();
    initBuzzer();
    turnBuzzerOn(true);
    Serial.print("init done");
    delay(1000);
}

void loop()
{
    decodeIR();
    unsigned long currentMillis = millis(); // Get the current time
    unsigned long currentAvgMillis = millis();
    if (!selected && currentMillis >= tempSelection)
    {
        currentMenu = 2;
        selected = true;
    }
    switch (currentMenu)
    {
    case 0:
        isStarted = true;
        if (!menuDrawn)
        {
            drawMenu(0);
            delay(150);
            drawAvgScreen(false, 16.25, 17, 30, 500); // Call drawMenu only once when currentMenu is 0
            menuDrawn = true;                         // Set flag to prevent repeated calls
        }
        break;
    case 1:
        if (isStarted)
        {
            drawGPSScreen(false);
            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            // list sat funs

            if (averageUpdated)
            {
                drawGPSScreen(false);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                drawGPSScreen(true);
            }
        }
        break;
    case 2:
        if (isStarted)
        {
            // float coolantTemp=readCoolantTemp();
            float coolantTemp = 10.0;
            delay(200);
            Temperatures temp = getTemperatures();
            drawTemperaturesScreen(temp, false, coolantTemp);

            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            Temperatures temp = getTemperatures();
            // float coolantTemp=readCoolantTemp();
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
                Serial.println("updating al");
                drawAltitudeScreen(altitude, temp, false);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                Serial.println("updating alt");
                Altitude altitude = getAltitude();
                drawAltitudeScreen(altitude, temp, true);
            }
        }
        break;
    case 4:
        // if (isStarted){
        //
        //         isStarted=false;
        //     }
        //     else if(currentMillis - previousAvgMillis >= avgInterval) {
        //
        //     }
        break;
    case 5:
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

    if (currentAvgMillis - previousAvgMillis >= avgInterval)
    {
        previousAvgMillis = currentAvgMillis;
        // float fuelFlow=readFuelFlow();
        // getAvg();yet to be implemented
        drawAvgScreen(true, 18, 19, 20, 300);
        averageUpdated = true;
        Serial.println("avg updated");
    }
}