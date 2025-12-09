#ifndef ESPNOW_H
#define ESPNOW_H
#include <Arduino.h>

struct StatusData
{
  bool isConnected;
  int rssi;
  int batteryLevel;
  float immobilizerTemperature;
};

void initESPNow();
void sendStatusRequest();
void sendParkStatus();
#endif // AVERAGE_H
