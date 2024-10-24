#include <Arduino.h>
#include <IRremote.hpp>
#include "remote.h"
#include "display.h"
#include "average.h"
#include "buzzer.h"
#include"sd.h"

// Pin for the IR receiver
#define IR_RECEIVE_PIN 16

// Hex values for remote buttons
#define BUTTON_UP 0xBF4052AD
#define BUTTON_DOWN 0xBE4152AD
#define BUTTON_BACK 0xBD4252AD
#define BUTTON_SELECT 0xE61952AD
#define BUTTON_SELECT_2 0xD32C50AF
#define BUTTON_RESET 0xE51A52AD
#define BUTTON_DELETE 0xE51A52AD

// Variables for menu control
extern int currentMenu;
extern bool menuDrawn;
extern bool isStarted;
extern bool averageUpdated;
int selectedItem = 0;  // Initially, the first menu item is selected
int menuItemCount = 4; // Total number of menu items

// Create an IR receiver object
decode_results results;

// Initialize the buzzer pin
void initIRSensor()
{
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
}

void handleMenuNavigation(unsigned long irCode)
{
    // Check for the reset button first
    if (irCode == BUTTON_RESET)
    {
        Average avg = resetTrip();
        drawAvgScreen(avg.average, avg.distance, avg.fuel, avg.dte);
        averageUpdated = true;
        turnBuzzerOn(true);
        return;
    }

    // if (irCode == BUTTON_DELETE)
    // {
    //     Average avg = resetTrip();
    //     drawAvgScreen(avg.average, avg.distance, avg.fuel, avg.dte);
    //     averageUpdated = true;
    //     deleteAverageFile();
    //     turnBuzzerOn(true);
    //     return;
    // }

    if (currentMenu == 0)
    { 
        switch (irCode)
        {
        case BUTTON_UP: // Up button
            selectedItem--;
            if (selectedItem < 0)
            {
                selectedItem = menuItemCount - 1; // Wrap around to the last item
            }
            drawMenu(selectedItem); // Redraw the menu with the updated selection
            break;
        case BUTTON_DOWN: // Down button
            selectedItem++;
            if (selectedItem >= menuItemCount)
            {
                selectedItem = 0; // Wrap around to the first item
            }
            drawMenu(selectedItem); // Redraw the menu with the updated selection
            break;
        case BUTTON_SELECT: // Select button
        case BUTTON_SELECT_2:
            // Handle the selected item action
            handleMenuAction(selectedItem);
            break;
        case BUTTON_BACK:           // Back button
            currentMenu = 0;        // Set to main menu
            selectedItem = 0;       // Reset selected item to the first item
            drawMenu(selectedItem); // Redraw the main menu
            break;
        }
    }
    else
    { // In a sub-menu, check for specific actions if needed
        switch (irCode)
        {
        case BUTTON_BACK:
            currentMenu = 0;        // Go back to the main menu
            selectedItem = 0;       // Reset selection
            drawMenu(selectedItem); // Redraw the main menu
            break;
            // Add more cases if needed for specific sub-menu actions
        }
    }
}

void handleMenuAction(int selectedItem)
{
    switch (selectedItem)
    {
    case 0:
        // Action for "GPS" menu item
        currentMenu = 1;
        turnBuzzerOn(true);
        Serial.println("GPS selected");
        break;
    case 1:
        // Action for "Temperatures" menu item
        currentMenu = 2;
        turnBuzzerOn(true);
        Serial.println("Temperatures selected");
        break;
    case 2:
        // Action for "Altitude" menu item
        currentMenu = 3;
        turnBuzzerOn(true);
        Serial.println("Altitude selected");
        break;
    case 3:
        // Action for "Average" menu item
        currentMenu = 4;
        turnBuzzerOn(true);
        Serial.println("Service selected");
        break;
    }
}

void decodeIR()
{
    if (IrReceiver.decode())
    {
        unsigned long irCode = IrReceiver.decodedIRData.decodedRawData;
        Serial.println(irCode, HEX);
        handleMenuNavigation(irCode); // Handle regular menu navigation
        IrReceiver.resume();          // Enable receiving of the next value
    }
}
