#ifndef DISPLAY_H
#define DISPLAY_H
#include "temperature.h"
#include "altitude.h"
#include"espnow.h"

// Function declarations for initializing and updating displays
void initDisplays();
// Function to display the main menu
void drawMenu(int selectedItem);

// Function to display temperatures on the OLED
void drawTemperaturesScreen(Temperatures temp, bool update, int coolantTemp);
void drawAltitudeScreen(Altitude altitude, int temp,float pitch, bool update);
void updateTemperaturesScreen(Temperatures temp);
void drawAvgScreen(float avg, float dis, float fuel, float dte);
void drawGPSScreen(bool update,int sats, int speed,int fix);
void drawEngineScreen(int rpm, int coolantTemp,int speed,float fuel,float voltage, bool update);
void drawHUDScreen(int brightnessLevel);
void drawKeyFobScreen(StatusData lastStatus);
void drawWarningScreen(int batt, const char *title, bool coolant);
void drawServiceMainSubmenu(int selected);
void drawServiceSubmenuItems(int selected);
void drawServiceItemScreen(int serviceIndex);

#endif // DISPLAY_H
