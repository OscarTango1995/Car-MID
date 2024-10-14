#ifndef DISPLAY_H
#define DISPLAY_H
#include "temperature.h"
#include "altitude.h"

// Function declarations for initializing and updating displays
void initDisplays();
// Function to display the main menu
void drawMenu(int selectedItem);

// Function to display temperatures on the OLED
void drawTemperaturesScreen(Temperatures temp, bool update, int coolantTemp);
void drawAltitudeScreen(Altitude altitude, Temperatures temp, bool update);
void updateTemperaturesScreen(Temperatures temp);
void drawAvgScreen(bool update, float avg, float dis, int fuel, int dte);
void drawGPSScreen(bool update);
void drawServiceScreen();

#endif // DISPLAY_H
