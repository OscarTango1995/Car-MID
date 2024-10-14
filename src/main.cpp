#include <Arduino.h>
#include"temperature.h"
#include"display.h"
#include"logger.h"
#include"altitude.h"
#include"buzzer.h"
#include"remote.h"
#include"elm.h"

bool selected=false;
unsigned long lastInteractionMillis = 0; // Timer for user interaction
unsigned long previousTempMillis = 0; // Stores last time temp readings were updated
unsigned long previousAvgMillis = 0; // Stores last time avg readings were updated
const long tempInterval = 30000; // Interval for displaying temperature readings (30 seconds)
const long tempSelection = 10000; // Interval for displaying temperature
const long avgInterval = 900000; // Interval for displaying the average screen (30 seconds)
int currentMenu = 0; 
static bool isStarted=true;
bool isMenuActive = true;
bool menuDrawn = false;               // Flag to check if the menu has already been drawn

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    delay(1000);

// initialize Sensors & Displays
    initializeBluetooth(); // Initialize Bluetooth
    initAltitude(); 
    initDisplays();
    initTempSensors(); 
    //initLogging();
    initIRSensor();
    initBuzzer();
    turnBuzzerOn(true);
    Serial.print("init done");
    delay(1000);
}

void loop() {
   decodeIR();
   unsigned long currentMillis = millis(); // Get the current time
    if (!selected && currentMillis - tempSelection<=0 ){
        currentMenu=2;
        selected=true;

   
    switch (currentMenu)
    {
        case 0:
            isStarted=true;
            if (!menuDrawn) {
                   drawMenu(0);  
                   drawAvgScreen(false,false);       // Call drawMenu only once when currentMenu is 0
                   menuDrawn = true;   // Set flag to prevent repeated calls
               }
        break;
        case 1:
            if (isStarted){ 
                drawGPSScreen(false);
            }
            else if(currentMillis - previousTempMillis >= tempInterval) {
                previousTempMillis = currentMillis;
                drawGPSScreen(true);
            }
        break;
        case 2:
        if (isStarted){
            // float coolantTemp=readCoolantTemp();
            float coolantTemp=10.0; 

            delay(750);
            Temperatures temp = getTemperatures();
            drawTemperaturesScreen(temp,false,coolantTemp);
            isStarted=false;
            }else if(currentMillis - previousTempMillis >= tempInterval) {
                previousTempMillis = currentMillis;
                // float coolantTemp=readCoolantTemp();
                float coolantTemp=10.0; 

                delay(750);
                Temperatures temp = getTemperatures();
                drawTemperaturesScreen(temp,false,coolantTemp);
            }
        break;
        case 3:
        if (isStarted){
                Altitude altitude = getAltitude(); 
                Temperatures temp = getTemperatures();
                drawAltitudeScreen(altitude,temp,false);
                isStarted=false;
            }
            else if(currentMillis - previousTempMillis >= tempInterval) {
                previousTempMillis = currentMillis;
                Altitude altitude = getAltitude();
                Temperatures temp = getTemperatures();
                drawAltitudeScreen(altitude,temp,true);
            }
        break;
        case 4:
        if (isStarted){
                drawAvgScreen(true,false);
                isStarted=false;
            }
            else if(currentMillis - previousAvgMillis >= avgInterval) {
                drawAvgScreen(true,true);
            }
        break;
        case 5:
        if (isStarted){
                drawServiceScreen();
                isStarted=false;
            }
        break;

    default:
        if(!menuDrawn) {
                    drawMenu(0);         
                    menuDrawn = true;
                }
    break;
    }
   
    if (currentMillis - previousAvgMillis >= avgInterval) {
        previousAvgMillis = currentMillis; 
        // getAvg();yet to be implemented
        drawAvgScreen(false,false);
    }

}