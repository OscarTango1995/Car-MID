#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "sd.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW // FC16 module type
#define MAX_DEVICES 4                     // Total number of 8x8 matrices

#define CS_PIN 2   // Chip Select
#define CLK_PIN 0  // Clock
#define DIN_PIN 25 // Data In

MD_Parola matrixDisplay = MD_Parola(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void hello()
{
    matrixDisplay.displayClear();
    matrixDisplay.setTextAlignment(PA_CENTER); // Align text to the right
    char speedText[8];                         // Buffer for "120" + null terminator
    matrixDisplay.print("CIVIC!");
}

void initHUD()
{
    int brightnessLevel = readHUDBrightness();
    delay(150);
    matrixDisplay.begin();
    matrixDisplay.setIntensity(brightnessLevel); // Adjust brightness (0-15)
    matrixDisplay.displayClear();

    // **Mirror the text for HUD mode**
    matrixDisplay.setZoneEffect(0, true, PA_FLIP_UD);
    hello();
    delay(1000);
}
void hudSpeed(int speed)
{
    matrixDisplay.displayClear();
    matrixDisplay.setTextAlignment(PA_CENTER); // Align text to the right
    char speedText[8];                         // Buffer for "120" + null terminator
    sprintf(speedText, "%2d KM", speed);          // Ensure 2-digit spacing
    matrixDisplay.print(speedText);
}
