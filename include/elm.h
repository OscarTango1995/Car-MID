#ifndef ELM_H
#define ELM_H

#include "ELMduino.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;
extern ELM327 myELM327;

void initializeBluetooth();
void connectToOBD();
int readCoolantTemp();
float readFuelFlow();
float calculateAverageFuelConsumption(float fuelFlow);


#endif
