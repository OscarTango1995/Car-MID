#include "elm.h"
#include <Arduino.h>
#include "sd.h"

BluetoothSerial SerialBT;
ELM327 myELM327;

void initializeBluetooth()
{
    if (!SerialBT.begin("ArduHUD", true))
    {
        Serial.println("Failed to initialize Bluetooth");
        while (1)
            ;
    }
    SerialBT.setPin("1234");
    connectToOBD();
}

void connectToOBD()
{
    if (!SerialBT.connect("OBDII"))
    {
        Serial.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }
    if (!myELM327.begin(SerialBT, false, 2000))
    {
        Serial.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }
    Serial.println("Connected to ELM327");
}

int readCoolantTemp()
{
    float coolant = myELM327.engineCoolantTemp();
    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
        return (uint32_t)coolant;
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }
    return 0;
}

float readFuelFlow()
{
    float engineLoad = myELM327.engineLoad(); //%
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    uint8_t manifoldPressure = myELM327.manifoldPressure(); // kpa
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float throttlePosition = myELM327.throttle(); //%
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float rpm = myELM327.rpm(); // int
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float K = 5772751.26;

    float fuelFlow = (engineLoad * throttlePosition * manifoldPressure * rpm) / K;

    return fuelFlow / 60; // L/min
}

EngineInfo getEngineInfo()
{
    float rpm = myELM327.rpm();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float coolant = myELM327.engineCoolantTemp();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float speed = myELM327.kph();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    EngineInfo engine;
    engine.coolantTemp = coolant;
    engine.rpm = rpm;
    engine.speed = speed;
    return engine;
}

int getSpeed()
{
    float speed = myELM327.kph();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }
    return speed;
}

int getRPM()
{
    float rpm = myELM327.rpm();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }
    return rpm;
}