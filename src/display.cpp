#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_I2CDevice.h>
#include "temperature.h"
#include "altitude.h"

// U8G2 constructor for the OLED (replace 0x3C with the correct address if needed)
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);  // average display
U8G2_SH1106_128X64_NONAME_F_SW_I2C oled2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE); // main display

void drawInitialDisplay(U8G2_SH1106_128X64_NONAME_F_SW_I2C &oled2, const char *title)
{
    oled2.setI2CAddress(0x3D << 1);
    oled2.clearDisplay();
    // Draw the outer frame
    oled2.drawFrame(0, 0, 128, 64); // Draw the frame around the entire display

    // Draw title bar
    oled2.setDrawColor(0);          // Set draw color to black for the title bar
    oled2.drawBox(0, 0, 128, 13);   // Draw filled box for the title bar
    oled2.setDrawColor(1);          // Set draw color to white for border
    oled2.drawFrame(0, 0, 128, 13); // Draw border around title bar

    oled2.setFont(u8g2_font_6x12_tr);
    oled2.setDrawColor(1); // Set draw color to white for text

    // Center the title
    oled2.setCursor((128 - oled2.getStrWidth(title)) / 2, 10);
    oled2.print(title);
}

// Function to draw a menu with submenus
void drawMenu(int selectedItem)
{
    oled2.setI2CAddress(0x3D << 1);
    // Define menu items
    const char *menuItems[] = {
        "1. GPS",
        "2. Temps",
        "3. Altitude",
        "4. Average",
        "5. Service"};

    int itemCount = sizeof(menuItems) / sizeof(menuItems[0]);

    // Draw the title bar and its frame
    oled2.setDrawColor(0);          // Set to black
    oled2.drawBox(0, 0, 128, 13);   // Clear the title area (to refresh it)
    oled2.setDrawColor(1);          // Set back to white for the border
    oled2.drawFrame(0, 0, 128, 13); // Draw the title bar frame

    // Draw the title text
    oled2.setFont(u8g2_font_6x12_tr);
    oled2.setCursor((128 - oled2.getStrWidth("Main Menu")) / 2, 10);
    oled2.print("Main Menu");

    // Draw the content frame (around the menu items area)
    oled2.setDrawColor(1);           // Ensure we're drawing in white
    oled2.drawFrame(0, 12, 128, 52); // Frame around the content area

    // Clear the content area where the menu items are drawn
    oled2.setDrawColor(0);         // Set to black for clearing
    oled2.drawBox(1, 15, 126, 48); // Clear the inside content area (leave the frame)

    oled2.setDrawColor(1); // Set back to white for drawing text

    // Draw the menu items
    oled2.setFont(u8g2_font_7x14_tr);                  // Set the font for menu items
    int contentYPosition = 26;                         // Start just below the title bar
    int lineHeight = 12;                               // Height of each menu item
    int availableHeight = 64 - contentYPosition - 2;   // Total height available for content
    int maxItemsInView = availableHeight / lineHeight; // Max number of items that fit in the view

    // Calculate the starting index for the visible menu items
    int startIndex = selectedItem - (maxItemsInView / 2);
    if (startIndex < 0)
    {
        startIndex = 0;
    }
    else if (startIndex + maxItemsInView > itemCount)
    {
        startIndex = itemCount - maxItemsInView;
    }

    const int leftPadding = 6; // Left padding for menu items

    // Loop through the visible menu items and print them
    for (int i = 0; i < maxItemsInView; i++)
    {
        int currentIndex = startIndex + i;
        if (currentIndex >= itemCount)
            break; // Avoid going out of bounds

        // Set cursor position
        oled2.setCursor(leftPadding, contentYPosition + 2 + (i * lineHeight));

        // Print the `>` symbol for the selected item
        if (currentIndex == selectedItem)
        {
            oled2.print(">"); // Print highlight sign before the selected item
        }
        else
        {
            oled2.print(" "); // Space for alignment
        }

        // Print the menu item text
        oled2.print(menuItems[currentIndex]);
    }

    // Send the updated buffer to the display
    oled2.sendBuffer();
}

void initDisplays()
{
    Wire.begin();
    delay(100);

    oled.setI2CAddress(0x3C << 1);
    if (!oled.begin())
    {
        Serial.println(F("Failed to initialize display"));
        while (1)
            ;
        delay(750);
    }

    oled2.setI2CAddress(0x3D << 1);
    if (!oled2.begin())
    {
        Serial.println(F("Failed to initialize display"));
        while (1)
            ;
        delay(750);
    }
}

// Function to display temperature data on the OLED
void drawTemperaturesScreen(Temperatures temp, bool update, int coolantTemp)
{
    oled2.setI2CAddress(0x3D << 1);
    char oatStr[10], iatStr[10], rhStr[10], engStr[10];
    sprintf(oatStr, "%d °C", (int)temp.oat);
    sprintf(iatStr, "%d °C", (int)temp.iat);
    sprintf(rhStr, "%d %%", (int)temp.humidity);
    sprintf(engStr, "%d °C", (int)coolantTemp);

    if (!update)
    {
        drawInitialDisplay(oled2, "TEMPERATURE");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;
        int lineHeight = 12;

        oled2.setCursor(6, contentYPosition + 0 * lineHeight);
        oled2.print("OAT : ");
        oled2.setCursor(6, contentYPosition + 1 * lineHeight);
        oled2.print("IAT : ");
        oled2.setCursor(6, contentYPosition + 2 * lineHeight);
        oled2.print("RH  : ");
        oled2.setCursor(6, contentYPosition + 3 * lineHeight);
        oled2.print("ENG : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight);
        oled2.print(oatStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight);
        oled2.print(iatStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight);
        oled2.print(rhStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight);
        oled2.print(engStr);
        oled2.sendBuffer();
    }
    else
    {
        oled2.setDrawColor(0);
        oled2.drawBox(45, 15, 70, 47);

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25);
        oled2.print(oatStr);
        oled2.setCursor(50, 37);
        oled2.print(iatStr);
        oled2.setCursor(50, 49);
        oled2.print(rhStr);
        oled2.setCursor(50, 61);
        oled2.print(engStr);
    }
    oled2.sendBuffer();
}

// Function to display GPS data on the OLED
void drawGPSScreen(bool update)
{
    oled2.setI2CAddress(0x3D << 1);
    char satStr[10], spdStr[10], dteStr[10];
    sprintf(satStr, "%d ", 4);
    sprintf(spdStr, "%d KM/H", 100);
    sprintf(dteStr, "%d KM", 500);

    if (!update)
    {
        drawInitialDisplay(oled2, "GPS");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;
        int lineHeight = 12;

        oled2.setCursor(6, contentYPosition + 0 * lineHeight);
        oled2.print("SAT : ");
        oled2.setCursor(6, contentYPosition + 1 * lineHeight);
        oled2.print("SPD : ");
        oled2.setCursor(6, contentYPosition + 2 * lineHeight);
        oled2.print("DTE : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight);
        oled2.print(satStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight);
        oled2.print(spdStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight);
        oled2.print(dteStr);
        oled2.sendBuffer();
    }
    else
    {
        oled2.setDrawColor(0);
        oled2.drawBox(45, 15, 70, 47);

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25);
        oled2.print(satStr);
        oled2.setCursor(50, 37);
        oled2.print(spdStr);
        oled2.setCursor(50, 49);
        oled2.print(dteStr);
    }
    oled2.sendBuffer();
}

// Function to display altitude data on the OLED
void drawAltitudeScreen(Altitude altitude, Temperatures temp, bool update)
{
    oled2.setI2CAddress(0x3D << 1);
    char altStr[10], hpaStr[10], iatStr[10], oatStr[10];
    sprintf(altStr, "%d M", (int)altitude.altitude);
    sprintf(hpaStr, "%d hPa", (int)altitude.pressure);
    sprintf(iatStr, "%d  C", (int)altitude.temperature);
    sprintf(oatStr, "%d  C", (int)temp.oat);

    if (!update)
    {
        drawInitialDisplay(oled2, "ALTITUDE");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;
        int lineHeight = 12;

        oled2.setCursor(6, contentYPosition + 0 * lineHeight);
        oled2.print("ALT : ");
        oled2.setCursor(6, contentYPosition + 1 * lineHeight);
        oled2.print("PRES: ");
        oled2.setCursor(6, contentYPosition + 2 * lineHeight);
        oled2.print("IAT : ");
        oled2.setCursor(6, contentYPosition + 3 * lineHeight);
        oled2.print("OAT : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight);
        oled2.print(altStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight);
        oled2.print(hpaStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight);
        oled2.print(iatStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight);
        oled2.print(oatStr);
    }
    else
    {
        oled2.setDrawColor(0);
        oled2.drawBox(45, 15, 70, 47);

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25);
        oled2.print(altStr);
        oled2.setCursor(50, 37);
        oled2.print(hpaStr);
        oled2.setCursor(50, 49);
        oled2.print(iatStr);
        oled2.setCursor(50, 61);
        oled2.print(oatStr);
    }
    oled2.sendBuffer();
}

// Function to display avg data on the OLED
void drawAvgScreen(bool update, float avg, float dis, int fuel, int dte)
{
    oled.setI2CAddress(0x3C << 1);
    oled.clearDisplay();
    // Draw the outer frame
    oled.drawFrame(0, 0, 128, 64); // Draw the frame around the entire display

    // Draw title bar
    oled.setDrawColor(0);          // Set draw color to black for the title bar
    oled.drawBox(0, 0, 128, 13);   // Draw filled box for the title bar
    oled.setDrawColor(1);          // Set draw color to white for border
    oled.drawFrame(0, 0, 128, 13); // Draw border around title bar

    oled.setFont(u8g2_font_6x12_tr);
    oled.setDrawColor(1); // Set draw color to white for text

    // Center the title
    oled.setCursor((128 - oled.getStrWidth("AVERAGE")) / 2, 10);
    oled.print("AVERAGE");
    oled.setFont(u8g2_font_7x14_tr);
    int contentYPosition = 25;
    int lineHeight = 12;

    oled.setCursor(7, contentYPosition + 0 * lineHeight);
    oled.print("AVG : ");
    oled.setCursor(7, contentYPosition + 1 * lineHeight);
    oled.print("DIS : ");
    oled.setCursor(7, contentYPosition + 2 * lineHeight);
    oled.print("FUEL: ");
    oled.setCursor(7, contentYPosition + 3 * lineHeight);
    oled.print("DTE : ");

    char avgStr[10], disStr[16], fuelStr[10], dteStr[10];
    sprintf(avgStr, "%0.1f KPL", avg);
    sprintf(disStr, "%0.2f KM", dis);
    sprintf(fuelStr, "%d %%", fuel);
    sprintf(dteStr, "%d KM", dte);

    oled.setCursor(51, contentYPosition + 0 * lineHeight);
    oled.print(avgStr);
    oled.setCursor(51, contentYPosition + 1 * lineHeight);
    oled.print(disStr);
    oled.setCursor(51, contentYPosition + 2 * lineHeight);
    oled.print(fuelStr);
    oled.setCursor(51, contentYPosition + 3 * lineHeight);
    oled.print(dteStr);

    oled.sendBuffer();
}

void drawServiceScreen()
{
    oled2.setI2CAddress(0x3D << 1);
    int eoilValue = 178000;
    int goilValue = 173580;

    int eoilcValue = eoilValue + 5000;  // Add 5000 to the original value
    int goilcValue = goilValue + 40000; // Add 30000 to the original value

    drawInitialDisplay(oled2, "SERVICE");
    oled2.setFont(u8g2_font_7x14_tr);
    int contentYPosition = 25;
    int lineHeight = 12;

    oled2.setCursor(6, contentYPosition + 0 * lineHeight);
    oled2.print("E-OIL: ");
    oled2.setCursor(6, contentYPosition + 1 * lineHeight);
    oled2.print("G-OIL: ");
    oled2.setCursor(6, contentYPosition + 2 * lineHeight);
    oled2.print("E-CHG: ");
    oled2.setCursor(6, contentYPosition + 3 * lineHeight);
    oled2.print("G-CHG: ");

    oled2.setCursor(50, contentYPosition + 0 * lineHeight);
    oled2.print(eoilValue);
    oled2.setCursor(50, contentYPosition + 1 * lineHeight);
    oled2.print(goilValue);
    oled2.setCursor(50, contentYPosition + 2 * lineHeight);
    oled2.print(eoilcValue);
    oled2.setCursor(50, contentYPosition + 3 * lineHeight);
    oled2.print(goilcValue);

    oled2.sendBuffer();
}