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
#include "fuel_level.h"
#include "relay.h"
#include "hud.h"
#include "espnow.h"
#include "accelerometer.h"

bool firstAltitudeReading = true;
bool selected = false;
bool averageUpdated = false;
unsigned long lastInteractionMillis = 0; // Timer for user interaction
unsigned long previousTempMillis = 0;    // Stores last time temp readings were updated
unsigned long previousGPSMillis = 0;     // Stores last time temp readings were updated
unsigned long previousAltMillis = 0;     // Stores last time temp readings were updated
unsigned long previousEngMillis = 0;     // Stores last time temp readings were updated
unsigned long previousCoolantMillis = 0; // Stores last time coolant readings were updated
unsigned long previousHUDMillis = 0;     // Stores last time speed readings were updated
unsigned long previousAvgMillis = 0;     // Stores last time avg readings were updated
unsigned long previousLogMillis = 0;     // Stores last time data was logged
const long averageInterval = 60000;      // Interval for logging data (1 minute)
unsigned long previousfuelMillis = 0;    // Stores last time data was logged
unsigned long fuelRetryStartMillis = 0;  // new
const long fuelReadingInterval = 5000;   // Interval for logging data (5 seconds)
const long tempInterval = 30000;         // Interval for displaying temperature readings (30 seconds)
const long hudInterval = 1000;           // Interval for displaying temperature readings (1 second)
const long gpsInterval = 10000;          // Interval for displaying temperature readings (10 seconds)
const long coolantInterval = 30000;      // Interval for displaying temperature readings (10 seconds)
const long tempSelection = 5000;         // Interval for displaying temperature
const long fuelRetryTimeout = 180000;    // 3 minutes
static unsigned long lastStatusRequest = 0;
int currentMenu = 0;
extern int serviceSubSelected;
static bool isStarted = true;
bool carStarted = true;
bool isMenuActive = true;
bool menuDrawn = false;  // Flag to check if the menu has already been drawn
int unLockRelayPin = 14; // Replace with the GPIO pin you're using for the relay
int lockRelayPin = 12;   // Replace with the GPIO pin you're using for the relay
int hallSensorPin = 35;
int toggleButtonPin = 26;
int lockStatusButtonPin = 27;
bool actionTriggered = false; // Flag to track if toggle has been triggered
bool carUnlocked = true;
bool lastButtonState = HIGH; // Assume the button starts unpressed (HIGH)
bool currentButtonState = HIGH;
int lockStatus = 0;
int hudBrightness = 0;
bool fuelStable = false;
float currentFuel = 0.0;
float previousFuel = 0.0;
bool retryStarted = false;
bool fuelReadingNeeded = false;
StatusData lastStatus;
bool displayStatusReceived = false;
bool checkStatusReceived = false;
bool isPark = false;
static bool coolantWarningDrawn = false;

unsigned long menuDelayMillis = 0;
unsigned long coolantDelayMillis = 0;
unsigned long altitudeDelayMillis = 0;
unsigned long gpsDelayMillis = 0;
unsigned long tempDelayMillis = 0;
unsigned long fuelDelayMillis = 0;

bool isWarningActive = false;
String currentWarningMessage = "";
int warningValue = 0;
bool warningPersistent = false;
unsigned long warningStartMillis = 0;
unsigned long warningDisplayMillis = 0;
static bool parkStatusSent = false;
static unsigned long lastParkStatusSend = 0;
const unsigned long parkStatusCooldown = 3000; // 10 seconds or whatever you prefer
int serviceSelected = 0;                       // for navigating SERVICE submenu: 0 = E-OIL, 1 = G-OIL, 2 = COOLANT
int serviceState = 0;                          // 0 = submenu screen, 1 = detail screen
bool inServiceMenu = false;

void setup()
{
    Serial.begin(9600);
    delay(1000);
    // initialize Sensors & Displays
    initSDCard(true);
    initializeBluetooth();
    initAltitude();
    initAccelerometer();
    initDisplays();
    initTempSensors();
    initIRSensor();
    initFuelLevelSensor();
    initGPS();
    initBuzzer();
    turnBuzzerOn(true);
    initRelays();
    pinMode(hallSensorPin, INPUT);
    pinMode(toggleButtonPin, INPUT_PULLUP);
    pinMode(lockStatusButtonPin, INPUT_PULLUP);
    initHUD();
    initESPNow();
    Serial.println("Initialization Completed.....");
    delay(150);
}

bool waitForMillis(unsigned long delayTime, unsigned long &previousMillis)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= delayTime)
    {
        previousMillis = currentMillis; // Reset the timer
        return true;
    }
    return false;
}

void checkCentralLocking()
{
    int currentButtonState = digitalRead(lockStatusButtonPin);
    if (digitalRead(hallSensorPin) == LOW || digitalRead(toggleButtonPin) == LOW) // Assuming LOW means detected
    {
        if (digitalRead(hallSensorPin) == LOW && !parkStatusSent)
        {
            unsigned long now = millis();
            if (now - lastParkStatusSend >= parkStatusCooldown)
            {
                isPark = true;
                sendParkStatus();
                parkStatusSent = true;
                lastParkStatusSend = now;
            }
        }

        unLockCar();
        return;
    }

    if (currentButtonState != lastButtonState)
    {
        if (currentButtonState == LOW)
        {
            Serial.println("Car Is Unlocked");
            carUnlocked = true;
            lockStatus = 0;
        }
        else
        {
            Serial.println("Car Is Locked");
            carUnlocked = false;
            lockStatus = 1;
        }

        lastButtonState = currentButtonState;
    }

    if (carUnlocked && lockStatus == 0)
    {
        lockCar();
        parkStatusSent = false;
    }
}

void loop()
{
    // Serial.println(getFuel());
    // Serial.println(carAngle());
    // Serial.println(carLeveled());
    // delay(5000);

    if (carStarted)
    {

        writeCurrentFuel(getFuel());
        carStarted = false;
    }
        int currentButtonState = digitalRead(lockStatusButtonPin);

        Serial.println(currentButtonState);
        beep(2);

    checkCentralLocking();
    decodeIR();
    unsigned long currentAvgMillis = millis();
    unsigned long currentMillis = millis(); // Get the current time

    if (isWarningActive)
    {
        if (waitForMillis(200, warningDisplayMillis))
        {
            drawWarningScreen(warningValue, currentWarningMessage.c_str(), warningPersistent); // this shows only on OLED 2
        }

        // Auto-dismiss non-persistent warnings after 10 seconds
        if (!warningPersistent && millis() - warningStartMillis >= 10000)
        {
            isWarningActive = false;
            turnBuzzerOff();
            currentMenu = 2;
            selected = true;
            warningStartMillis = 0;
        }

        // If the remote button is pressed, clear warning
        if (remoteDismissPressed())
        {
            isWarningActive = false;
            turnBuzzerOff();
            currentMenu = 2;
            selected = true;
            warningStartMillis = 0;
        }
    }

    if (isWarningActive && currentMenu != 0)
    {
        // Skip all OLED 2 screen updates (HUD, temp, GPS, etc.)
    }
    else
    {
        switch (currentMenu)
        {
        case 0:
            isStarted = true;
            if (!menuDrawn && waitForMillis(100, menuDelayMillis))
            {
                if (digitalRead(hallSensorPin) == LOW)
                {
                    isPark = true;
                }
                sendParkStatus();
                sendStatusRequest();
                Average lastAvg = readLastAverageData();
                previousFuel = lastAvg.fuelRemaining;
                drawAvgScreen(lastAvg.average, lastAvg.distanceTraveled, lastAvg.fuelRemaining, lastAvg.dte);
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
            else if (currentMillis - previousGPSMillis >= gpsInterval)
            {
                if (averageUpdated)
                {
                    GPSData gpsData = getGPSData();
                    drawGPSScreen(false, gpsData.sat, gpsData.speed, gpsData.fix);
                    averageUpdated = false;
                }
                else
                {
                    previousGPSMillis = currentMillis;
                    GPSData gpsData = getGPSData();
                    drawGPSScreen(true, gpsData.sat, gpsData.speed, gpsData.fix);
                }
            }
            break;
        case 2:
            if (isStarted)
            {
                float coolantTemp = readCoolantTemp();
                if (waitForMillis(200, coolantDelayMillis))
                { // Non-blocking delay
                    Temperatures temp = getTemperatures();
                    drawTemperaturesScreen(temp, false, coolantTemp);
                    isStarted = false;
                }
            }
            else if (currentMillis - previousTempMillis >= tempInterval)
            {
                Temperatures temp = getTemperatures();
                float coolantTemp = readCoolantTemp();
                if (waitForMillis(200, coolantDelayMillis))
                {
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
            }
            break;
        case 3:
            if (isStarted)
            {
                Altitude altitude = getAltitude();
                Temperatures temp = getTemperatures();

                float pitch = carAngle();
                if (waitForMillis(3000, altitudeDelayMillis))
                { // Non-blocking delay
                    drawAltitudeScreen(altitude, temp.iat, pitch, false);
                    isStarted = false;
                }
            }
            else if (currentMillis - previousAltMillis >= tempInterval)
            {
                if (averageUpdated)
                {
                    float pitch = carAngle();
                    Altitude altitude = getAltitude(); // Get the second reading
                    Temperatures temp = getTemperatures();
                    if (waitForMillis(150, altitudeDelayMillis))
                    { // Non-blocking delay
                        drawAltitudeScreen(altitude, temp.iat, pitch, false);
                        averageUpdated = false;
                    }
                }
                else
                {
                    previousAltMillis = currentMillis;
                    Altitude altitude = getAltitude();
                    Temperatures temp = getTemperatures();
                    float pitch = carAngle();
                    drawAltitudeScreen(altitude, temp.iat, pitch, true);
                }
            }
            break;
        case 4:
            if (isStarted)
            {
                EngineInfo engine = getEngineInfo();
                float fuel = getFuel();
                float vin = readVoltage();
                drawEngineScreen(engine.rpm, engine.coolantTemp, engine.speed, fuel, vin, false);
                isStarted = false;
            }
            else if (currentMillis - previousEngMillis >= gpsInterval)
            {
                EngineInfo engine = getEngineInfo();
                float fuel = getFuel();
                float vin = readVoltage();
                if (averageUpdated)
                {

                    drawEngineScreen(engine.rpm, engine.coolantTemp, engine.speed, fuel, vin, false);
                    averageUpdated = false;
                }
                else
                {
                    drawEngineScreen(engine.rpm, engine.coolantTemp, engine.speed, fuel, vin, true);
                    previousEngMillis = currentMillis;
                }
            }
            break;
        case 5:

            if (isStarted)
            {
                if (averageUpdated)
                {
                    drawServiceMainSubmenu(serviceSubSelected);
                    averageUpdated = false;
                }
                else
                {
                    drawServiceMainSubmenu(serviceSubSelected);
                }
                isStarted = false;
            }

            break;
        case 6:
            if (isStarted)
            {
                int brightnessLevel = readHUDBrightness();
                hudBrightness = brightnessLevel;
                Serial.println(hudBrightness);

                if (averageUpdated)
                {
                    drawHUDScreen(brightnessLevel);
                    averageUpdated = false;
                }
                else
                {
                    drawHUDScreen(brightnessLevel);
                }
                isStarted = false;
            }
            break;
        case 7:
            if (isStarted && displayStatusReceived)
            {
                sendStatusRequest();

                if (displayStatusReceived && averageUpdated)
                {
                    displayStatusReceived = false;
                    drawKeyFobScreen(lastStatus);
                }

                else if (displayStatusReceived)
                {
                    displayStatusReceived = false;
                    drawKeyFobScreen(lastStatus);
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
    }
    if (!selected && currentMillis >= tempSelection)
    {
        currentMenu = 2;
        selected = true;
    }

    if (currentMillis - previousLogMillis >= averageInterval)
    {
        previousLogMillis = currentMillis;

        if (isAvgReady())
        {
            Average avg = calculateAverageFuelConsumption();
            drawAvgScreen(avg.average, avg.distanceTraveled, avg.fuelRemaining, avg.dte);
            averageUpdated = true;
            resetAvgReady();
        }
    }

    if (currentMillis - previousCoolantMillis >= coolantInterval)
    {

        previousCoolantMillis = currentMillis;
        float coolantTemp = readCoolantTemp();

        if (waitForMillis(200, coolantDelayMillis))
        {
            if (coolantTemp >= 100 && !isWarningActive)
            {
                warningValue = coolantTemp;
                currentWarningMessage = "TEMP HIGH";
                warningPersistent = true; // won't auto-dismiss
                isWarningActive = true;
                warningStartMillis = millis();
                turnBuzzerOn(false);
            }
        }
    }

    if (currentMillis - previousHUDMillis >= hudInterval)
    {
        previousHUDMillis = currentMillis;
        int speed = getSpeed();
        hudSpeed(speed);
    }

    if (millis() - lastStatusRequest > 60000)
    {
        sendStatusRequest();
        lastStatusRequest = millis();
    }

    // If new status received
    if (checkStatusReceived)
    {
        checkStatusReceived = false;

        if (lastStatus.batteryLevel < 25)
        {
            warningValue = lastStatus.batteryLevel;
            currentWarningMessage = "KEY BATT LOW";
            warningPersistent = false; // will auto-dismiss
            isWarningActive = true;
            warningStartMillis = millis();
            beep(2);
        }

        if (!lastStatus.isConnected)
        {
            warningValue = -1;
            currentWarningMessage = "KEY DSC";
            warningPersistent = false; // will auto-dismiss
            isWarningActive = true;
            warningStartMillis = millis();
            beep(2);
        }
    }
}
