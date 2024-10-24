#ifndef ELM_H
#define ELM_H

#include "ELMduino.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;
extern ELM327 myELM327;

struct EngineInfo
{
    int rpm;
    int coolantTemp;
    int speed;
};

void initializeBluetooth();
void connectToOBD();
int readCoolantTemp();
float readFuelFlow();
int getSpeed();
int getRPM();
EngineInfo getEngineInfo();

#endif
