#include "elm.h"
#include "Arduino.h"

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
    if (!myELM327.begin(SerialBT, true, 2000))
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
        Serial.print("coolant: ");
        Serial.println((uint32_t)coolant);
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
    float engineLoad = myELM327.engineLoad();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    uint8_t manifoldPressure = myELM327.manifoldPressure();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float throttlePosition = myELM327.throttle();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float rpm = myELM327.rpm();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float K = 0.5; // Adjust this based on your engine's specifications

    float fuelFlow = (engineLoad / 100.0) * (manifoldPressure / 100.0) * (throttlePosition / 100.0) * (rpm / 2.0) * K;

    return fuelFlow / 60; // Return fuel flow in L/min
}
