#include <Arduino.h>
#include <IRremote.hpp>
#include "remote.h"
#include "display.h"
#include "average.h"
#include "buzzer.h"
#include "sd.h"
#include "hud.h"
#include "gps.h"

// Pin for the IR receiver
#define IR_RECEIVE_PIN 16

// Hex values for remote buttons
#define BUTTON_UP 0xBF4052AD
#define BUTTON_DOWN 0xBE4152AD
#define BUTTON_BACK 0xBD4252AD
#define BUTTON_SELECT 0xE61952AD
#define BUTTON_SELECT_2 0xD32C50AF
#define BUTTON_RESET 0xE51A52AD
#define BUTTON_DELETE 0xA75852AD
#define IR_DISMISS_CODE 0xF20D52AD

// Variables for menu control
extern int currentMenu;
extern bool menuDrawn;
extern bool isStarted;
extern bool averageUpdated;
int selectedItem = 0;  // Initially, the first menu item is selected
int menuItemCount = 7; // Total number of menu items
extern int hudBrightness;
unsigned long lastIRCode = 0;
// Add these below other globals
bool inServiceSubmenu = false; // Whether we are in the service submenu
int serviceSubSelected = 0;    // Selected submenu index: 0=ENG, 1=GEAR, 2=COOLANT
int engServiceSelected = 0;    // 0 = no highlight, 1 = highlight "UPDATE"
int updateServiceSelected = 0; // 0 = no highlight, 1 = highlight "UPDATE"

int gearServiceSelected = 0;    // 0 = no highlight, 1 = highlight "UPDATE"
int coolantServiceSelected = 0; // 0 = no highlight, 1 = highlight "UPDATE"
int batteryServiceSelected = 0;
int timingServiceSelected = 0;
extern ServiceEntry services[5]; // Actual definition

// Create an IR receiver object
decode_results results;

// Initialize the buzzer pin
void initIRSensor()
{
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
}

bool remoteDismissPressed()
{
    if (lastIRCode == IR_DISMISS_CODE)
    {
        lastIRCode = 0; // reset after handling
        return true;
    }
    return false;
}

void handleMenuNavigation(unsigned long irCode)
{
    // Check for the reset button first
    if (irCode == BUTTON_RESET)
    {
        Average avg = resetTrip();
        drawAvgScreen(avg.average, avg.distanceTraveled, avg.fuelRemaining, avg.dte);
        averageUpdated = true;
        turnBuzzerOn(true);
        return;
    }

    if (irCode == BUTTON_DELETE)
    {
        Average avg = resetTrip();
        drawAvgScreen(avg.average, avg.distanceTraveled, avg.fuelRemaining, avg.dte);
        averageUpdated = true;
        turnBuzzerOn(true);
        return;
    }

    // Check for brightness adjustment mode
    if (currentMenu == 6) // Ensure you define BRIGHTNESS_MENU properly
    {
        switch (irCode)
        {
        case BUTTON_UP:
            hudBrightness = (hudBrightness == 15) ? 5 : hudBrightness + 5;
            drawHUDScreen(hudBrightness);
            break;

        case BUTTON_DOWN:
            hudBrightness = (hudBrightness == 5) ? 15 : hudBrightness - 5;
            drawHUDScreen(hudBrightness);
            break;

        case BUTTON_SELECT: // Select button
        case BUTTON_SELECT_2:
            // saveBrightnessToSD(brightnessLevel);
            writeHUDBrightness(hudBrightness);
            turnBuzzerOn(true);
            initHUD();
            break;

        case BUTTON_BACK: // Exit brightness menu
            currentMenu = 0;
            drawMenu(selectedItem);
            break;
        }
        return;
    }

    if (currentMenu == 5)
    {
        switch (irCode)
        {
        case BUTTON_UP:
            serviceSubSelected = (serviceSubSelected - 1 + 5) % 5; // Total 5 items, wrap on up
            drawServiceSubmenuItems(serviceSubSelected);
            break;

        case BUTTON_DOWN:
            serviceSubSelected = (serviceSubSelected + 1) % 5; // Wrap on down
            drawServiceSubmenuItems(serviceSubSelected);
            break;

        case BUTTON_SELECT:
        case BUTTON_SELECT_2:
            // Handle submenu selection
            switch (serviceSubSelected)
            {
            case 0:               // ENG
                currentMenu = 50; // Create new menu ID for ENG screen
                drawServiceItemScreen(0); // You implement this
                break;
            case 1:               // GEAR
                currentMenu = 51; // GEAR screen
                drawServiceItemScreen(1); // You implement this
                break;
            case 2: // COOLANT
                currentMenu = 52;
                drawServiceItemScreen(2); // You implement this
                break;
            case 3: // TIMING
                currentMenu = 53;
                drawServiceItemScreen(3); // You implement this
                break;
            case 4: // BATTERY
                currentMenu = 54;
                drawServiceItemScreen(4); // You implement this
                break;
            }
            turnBuzzerOn(true);
            break;

        case BUTTON_BACK:
            currentMenu = 0;
            selectedItem = 0;
            drawMenu(selectedItem);
            break;
        }

        return; // Exit early to avoid falling into other logic
    }

    if (currentMenu >= 50 && currentMenu <= 54)
    {
        int serviceIndex = currentMenu - 50; // Map menu ID to service index

        switch (irCode)
        {
        case BUTTON_UP:
        case BUTTON_DOWN:
            updateServiceSelected = !updateServiceSelected;
            drawServiceItemScreen(serviceIndex);
            break;

        case BUTTON_SELECT:
        case BUTTON_SELECT_2:
            if (updateServiceSelected == 1)
            {
                String date = getCurrentDate();
                if (date != " ")
                {
                    services[serviceIndex].mileage += getKmAdder(serviceIndex);
                    services[serviceIndex].date = date; // Assuming you have a method like this
                    updateServiceEntry(serviceIndex);   // Save updated CSV

                    Serial.printf("%s service updated.\n", services[serviceIndex].type.c_str());
                    turnBuzzerOn(true);
                }
                updateServiceSelected = 0;
                drawServiceItemScreen(serviceIndex);
            }
            break;

        case BUTTON_BACK:
            currentMenu = 5;
            serviceSubSelected = 0;
            drawServiceMainSubmenu(0);
            break;
        }

        return; // Prevent fallthrough
    }
    /*
        if (currentMenu == 50 )
        {
            switch (irCode)
            {
            case BUTTON_UP:
            case BUTTON_DOWN:
                updateServiceSelected = !updateServiceSelected; // Toggle between 0 and 1
                drawServiceItemScreen(0);
                break;

            case BUTTON_SELECT:
            case BUTTON_SELECT_2:
                if (updateServiceSelected == 1)
                {
                    // Perform the update
                    // You should implement actual update logic here:
                    // - Add 5000 to oil reading
                    // - Update change mileage
                    // - Reset remaining

                    Serial.println("Engine oil service updated.");
                    turnBuzzerOn(true);

                    // Optionally reset selection after update
                    updateServiceSelected = 0;
                    drawServiceItemScreen(0);
                }
                break;

            case BUTTON_BACK:
                currentMenu = 5;
                serviceSubSelected = 0;
                drawServiceMainSubmenu(0);
                break;
            }
        }

        if (currentMenu == 51)
        {
            switch (irCode)
            {
            case BUTTON_UP:
            case BUTTON_DOWN:
                updateServiceSelected = !updateServiceSelected; // Toggle between 0 and 1
                drawServiceItemScreen(1);
                break;

            case BUTTON_SELECT:
            case BUTTON_SELECT_2:
                if (updateServiceSelected == 1)
                {
                    // Perform the update
                    // You should implement actual update logic here:
                    // - Add 5000 to oil reading
                    // - Update change mileage
                    // - Reset remaining

                    Serial.println("Gear oil service updated.");
                    turnBuzzerOn(true);

                    // Optionally reset selection after update
                    updateServiceSelected = 0;
                    drawServiceItemScreen(1);
                }
                break;

            case BUTTON_BACK:
                currentMenu = 5;
                serviceSubSelected = 0;
                drawServiceMainSubmenu(0);
                break;
            }
        }

        if (currentMenu == 52)
        {
            switch (irCode)
            {
            case BUTTON_UP:
            case BUTTON_DOWN:
                updateServiceSelected = !updateServiceSelected; // Toggle between 0 and 1
                drawServiceItemScreen(2);
                break;

            case BUTTON_SELECT:
            case BUTTON_SELECT_2:
                if (updateServiceSelected == 1)
                {
                    // Perform the update
                    // You should implement actual update logic here:
                    // - Add 5000 to oil reading
                    // - Update change mileage
                    // - Reset remaining

                    Serial.println("Coolant service updated.");
                    turnBuzzerOn(true);

                    // Optionally reset selection after update
                    updateServiceSelected = 0;
                    drawServiceItemScreen(2);
                }
                break;

            case BUTTON_BACK:
                currentMenu = 5;
                serviceSubSelected = 0;
                drawServiceMainSubmenu(0);
                break;
            }
        }

        if (currentMenu == 53)
        {
            switch (irCode)
            {
            case BUTTON_UP:
            case BUTTON_DOWN:
                updateServiceSelected = !updateServiceSelected; // Toggle between 0 and 1
                drawServiceItemScreen(3);
                break;

            case BUTTON_SELECT:
            case BUTTON_SELECT_2:
                if (updateServiceSelected == 1)
                {
                    // Perform the update
                    // You should implement actual update logic here:
                    // - Add 5000 to oil reading
                    // - Update change mileage
                    // - Reset remaining

                    Serial.println("Timing service updated.");
                    turnBuzzerOn(true);

                    // Optionally reset selection after update
                    updateServiceSelected = 0;
                    drawServiceItemScreen(3);
                }
                break;

            case BUTTON_BACK:
                currentMenu = 5;
                serviceSubSelected = 0;
                drawServiceMainSubmenu(0);
                break;
            }
        }
        if (currentMenu == 54)
        {
            switch (irCode)
            {
            case BUTTON_UP:
            case BUTTON_DOWN:
                updateServiceSelected = !updateServiceSelected; // Toggle between 0 and 1
                drawServiceItemScreen(4);
                break;

            case BUTTON_SELECT:
            case BUTTON_SELECT_2:
                if (updateServiceSelected == 1)
                {
                    // Perform the update
                    // You should implement actual update logic here:
                    // - Add 5000 to oil reading
                    // - Update change mileage
                    // - Reset remaining

                    Serial.println("battery service updated.");
                    turnBuzzerOn(true);

                    // Optionally reset selection after update
                    updateServiceSelected = 0;
                    drawServiceItemScreen(4);
                }
                break;

            case BUTTON_BACK:
                currentMenu = 5;
                serviceSubSelected = 0;
                drawServiceMainSubmenu(0);
                break;
            }
        }
    */
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
    {
        switch (irCode)
        {
        case BUTTON_BACK:
            currentMenu = 0;
            selectedItem = 0;
            drawMenu(selectedItem);
            break;
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
        // Action for "Engine" menu item
        currentMenu = 4;
        turnBuzzerOn(true);
        Serial.println("Engine selected");
        break;
    case 4:
        currentMenu = 5; // Enter Service submenu
        serviceSubSelected = 0;
        turnBuzzerOn(true);
        Serial.println("Service Menu selected");
        break;
    case 5:
        // Action for "HUD" menu item
        currentMenu = 6;
        turnBuzzerOn(true);
        Serial.println("HUD selected");
        break;
    case 6:
        // Action for "Key Fob" menu item
        currentMenu = 7;
        turnBuzzerOn(true);
        Serial.println("Key Fob selected");
        break;
    }
}

void decodeIR()
{
    if (IrReceiver.decode())
    {
        Serial.print("remote:");
        unsigned long irCode = IrReceiver.decodedIRData.decodedRawData;
        Serial.println(irCode, HEX);
        lastIRCode = irCode;
        handleMenuNavigation(irCode); // Handle regular menu navigation
        IrReceiver.resume();          // Enable receiving of the next value
    }
}
