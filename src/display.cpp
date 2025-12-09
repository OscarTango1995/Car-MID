#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_I2CDevice.h>
#include "temperature.h"
#include "altitude.h"
#include "espnow.h"
#include "sd.h"

extern StatusData lastStatus;
extern int engServiceSelected;
extern int gearServiceSelected;
extern int coolantServiceSelected;
extern int batteryServiceSelected;
extern int timingServiceSelected;
extern int updateServiceSelected;
extern ServiceEntry services[5]; // Actual definition

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
        "4. Engine",
        "5. Maintenance",
        "6. HUD",
        "7. Key Fob"};

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

void drawServiceSubmenuItems(int selected)
{
    oled2.setI2CAddress(0x3D << 1);
    oled2.setFont(u8g2_font_7x14_tr);

    int contentYPosition = 25;
    int lineHeight = 12;

    const char *subItems[] = {
        "1. ENG",
        "2. GEAR",
        "3. COOLANT",
        "4. TIMING",
        "5. BATTERY",
    };

    int totalItems = sizeof(subItems) / sizeof(subItems[0]);
    int maxVisible = 4;

    // Clear only the content area
    oled2.setDrawColor(0);
    oled2.drawBox(1, 15, 126, 48); // content area only
    oled2.setDrawColor(1);

    // Calculate scrolling start index
    int startIdx = 0;
    if (selected >= maxVisible)
        startIdx = selected - maxVisible + 1;

    for (int i = 0; i < maxVisible && (startIdx + i) < totalItems; i++)
    {
        int idx = startIdx + i;
        oled2.setCursor(10, contentYPosition + i * lineHeight);
        oled2.print((idx == selected) ? ">" : " ");
        oled2.print(subItems[idx]);
    }

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
void drawGPSScreen(bool update, int sats, int speed, int fix)
{
    oled2.setI2CAddress(0x3D << 1);
    char satStr[10], spdStr[10], fixStr[10];
    sprintf(satStr, "%d ", sats);
    sprintf(spdStr, "%d KM/H", speed);
    sprintf(fixStr, "%dD", fix);

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
        oled2.print("FIX : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight);
        oled2.print(satStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight);
        oled2.print(spdStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight);
        oled2.print(fixStr);
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
        oled2.print(fixStr);
    }

    oled2.sendBuffer();
}

// Function to display altitude data on the OLED
void drawAltitudeScreen(Altitude altitude,int temp, float pitch, bool update)
{
    oled2.setI2CAddress(0x3D << 1);
    char altStr[10], hpaStr[10], iatStr[10], pitchStr[10];
    sprintf(altStr, "%d M", (int)altitude.altitude);
    sprintf(hpaStr, "%d hPa", (int)altitude.pressure);
    sprintf(iatStr, "%d  C", temp);
    sprintf(pitchStr, "%0.1f  °\t", (float)pitch);

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
        oled2.print("PTCH: ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight);
        oled2.print(altStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight);
        oled2.print(hpaStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight);
        oled2.print(iatStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight);
        oled2.print(pitchStr);
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
        oled2.print(pitchStr);
    }
    oled2.sendBuffer();
}

// Function to display avg data on the OLED
void drawAvgScreen(float avg, float dis, float fuel, float dte)
{
    Serial.println();
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
    oled.print("FREM: ");
    oled.setCursor(7, contentYPosition + 3 * lineHeight);
    oled.print("DTE : ");

    char avgStr[10], disStr[20], fuelStr[10], dteStr[10];
    snprintf(avgStr, sizeof(avgStr), "%0.2f KPL", avg);
    snprintf(disStr, sizeof(disStr), "%0.2f KM", dis);
    snprintf(fuelStr, sizeof(fuelStr), "%0.2f L", fuel);
    snprintf(dteStr, sizeof(dteStr), "%0.2f KM", dte);

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

void drawServiceItemScreen(int serviceIndex)
{
    readServiceData();

    oled2.setI2CAddress(0x3D << 1);
    drawInitialDisplay(oled2, services[serviceIndex].type.c_str());

    oled2.setFont(u8g2_font_7x14_tr);
    int y = 25;
    int h = 12;

    if (serviceIndex == 4)
    {
        oled2.setCursor(10, y);
        oled2.print("DATE:");
        oled2.print(services[serviceIndex].date); // Date from CSV
        oled2.setCursor(10, y + h);
        oled2.print(" ");
        oled2.setCursor(10, y + h * 2);
        oled2.print(" ");
    }
    else
    {
        // ENGINE OIL MILEAGE
        oled2.setCursor(10, y);
        oled2.print("AT  :");
        oled2.print(services[serviceIndex].mileage);
        oled2.print(" KM");

        // SERVICE DUE MILEAGE
        oled2.setCursor(10, y + h);
        oled2.print("DUE :");
        oled2.print(services[serviceIndex].mileage + getKmAdder(serviceIndex)); // Assuming service due after +5000 km
        oled2.print(" KM");

        // SERVICE DATE
        oled2.setCursor(10, y + h * 2);
        oled2.print("DATE:");
        oled2.print(services[serviceIndex].date); // Date from CSV
    }
    // UPDATE HIGHLIGHT
    oled2.setFont(u8g2_font_6x10_tr);
    oled2.setCursor(10, y + h * 3);
    if (updateServiceSelected == 1)
    {
        oled2.print("-> UPDATE?");
    }
    else
    {
        oled2.print("             "); // Clear previous text
    }

    oled2.sendBuffer();
}

void drawServiceMainSubmenu(int selected)
{
     oled2.setI2CAddress(0x3D << 1);
    drawInitialDisplay(oled2, "MAINTENANCE"); // Draw once
    drawServiceSubmenuItems(selected);        // Draw items
}

void drawHUDScreen(int brightnessLevel)
{
    oled2.setI2CAddress(0x3D << 1);

    // Set font for HUD title
    oled2.setFont(u8g2_font_7x14_tr);
    drawInitialDisplay(oled2, "HUD");

    // Define brightness labels and bar positions
    const char *levels[] = {"L", "M", "H"};
    int barX = 13, barY = 28, barWidth = 100, barHeight = 15; // Moved right by 3 pixels
    int stepWidth = barWidth / 3;

    // Set font for the rest of the UI
    oled2.setFont(u8g2_font_5x8_tr);

    // Draw small font title above bar
    oled2.setCursor(barX + 25, barY - 2);
    oled2.print("BRIGHTNESS");

    // Draw brightness bar background
    oled2.drawFrame(barX, barY + 3, barWidth, barHeight);

    // Map brightness levels (5 -> L, 10 -> M, 15 -> H)
    int brightnessIndex = (brightnessLevel - 5) / 5;
    int barOffset = (brightnessLevel == 5) ? 2 : (brightnessLevel == 15 ? 1 : 0);
    // int barWidthAdjust = (brightnessLevel == 15) ? 2 : 0; // Reduce width by 1 pixel when at H
    oled2.drawBox(barX + (brightnessIndex * stepWidth) + barOffset, barY + 5, stepWidth - 2, barHeight - 4);

    // Draw labels below steps with improved spacing
    oled2.setCursor(barX, barY + barHeight + 11);                                   // Left aligned
    oled2.print(levels[0]);                                                         // L
    oled2.setCursor(barX + stepWidth + (stepWidth / 2) - 3, barY + barHeight + 11); // Center aligned
    oled2.print(levels[1]);                                                         // M
    oled2.setCursor(barX + (2 * stepWidth) + stepWidth - 3, barY + barHeight + 11); // Adjusted right aligned by moving 2 pixels right
    oled2.print(levels[2]);                                                         // H

    oled2.sendBuffer();
}

void drawKeyFobScreen(StatusData lastStatus)
{
    char tempStr[10];
    sprintf(tempStr, "%d C", (int)lastStatus.immobilizerTemperature);
    oled2.setI2CAddress(0x3D << 1);

    drawInitialDisplay(oled2, "KEY FOB");
    oled2.setFont(u8g2_font_7x14_tr);
    int contentYPosition = 25;
    int lineHeight = 12;

    oled2.setCursor(6, contentYPosition + 0 * lineHeight);
    oled2.print("CONN: ");
    oled2.setCursor(6, contentYPosition + 1 * lineHeight);
    oled2.print("BATT: ");
    oled2.setCursor(6, contentYPosition + 2 * lineHeight);
    oled2.print("RSSI: ");
    oled2.setCursor(6, contentYPosition + 3 * lineHeight);
    oled2.print("TEMP: ");

    oled2.setCursor(50, contentYPosition + 0 * lineHeight);
    oled2.print(lastStatus.isConnected ? "CON" : "DSC");
    oled2.setCursor(50, contentYPosition + 1 * lineHeight);
    oled2.print(lastStatus.batteryLevel);
    oled2.setCursor(50, contentYPosition + 2 * lineHeight);
    oled2.print(lastStatus.rssi);
    oled2.setCursor(50, contentYPosition + 3 * lineHeight);
    oled2.print(tempStr);

    oled2.sendBuffer();
}

void drawWarningScreen(int batt, const char *title, bool coolant)
{
    oled2.setI2CAddress(0x3D << 1);
    oled2.clearDisplay();

    // Draw the outer frame
    oled2.drawFrame(0, 0, 128, 64);

    // Title bar
    oled2.setDrawColor(0);
    oled2.drawBox(0, 0, 128, 13);
    oled2.setDrawColor(1);
    oled2.drawFrame(0, 0, 128, 13);

    // Title text
    oled2.setFont(u8g2_font_6x12_tr);
    oled2.setCursor((128 - oled2.getStrWidth("WARNING")) / 2, 10);
    oled2.print("WARNING");

    // Centered warning messages
    oled2.setFont(u8g2_font_10x20_tr);                  // Bold readable font
    int textHeight = 20;                                // height of font_10x20_tr
    int yStart = (64 - 13 - (textHeight * 2)) / 2 + 13; // Adjusted for title bar height
    const char *line1 = title;
    if (batt > -1)
    {
        char battStr[6]; // enough for "100%" + null terminator
        if (coolant)
        {
            sprintf(battStr, "%dC", batt); // batt is your integer
        }
        else
        {
            sprintf(battStr, "%d%%", batt); // batt is your integer
        }

        const char *line2 = battStr;
        // Line 2
        oled2.setCursor((128 - oled2.getStrWidth(line2)) / 2, yStart + textHeight * 2);
        oled2.print(line2);
    }

    // Line 1
    oled2.setCursor((128 - oled2.getStrWidth(line1)) / 2, yStart + textHeight);
    oled2.print(line1);

    oled2.sendBuffer();
}

// Function to display altitude data on the OLED
void drawEngineScreen(int rpm, int coolantTemp, int speed, float fuel, float voltage, bool update)
{

    oled2.setI2CAddress(0x3D << 1);
    char rpmStr[10], engStr[10], speedStr[16], fuelStr[10];
    sprintf(rpmStr, "%d ", rpm);
    sprintf(engStr, "%d C", coolantTemp);
    sprintf(speedStr, "%0.2f ", voltage);
    // sprintf(speedStr, "%d KM/H ", speed);
    sprintf(fuelStr, "%0.2f L", fuel);

    if (!update)
    {
        drawInitialDisplay(oled2, "ENGINE");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;
        int lineHeight = 12;

        oled2.setCursor(6, contentYPosition + 0 * lineHeight);
        oled2.print("RPM : ");
        oled2.setCursor(6, contentYPosition + 1 * lineHeight);
        oled2.print("VOL: ");
        // oled2.print("SPD: ");
        oled2.setCursor(6, contentYPosition + 2 * lineHeight);
        oled2.print("ENG : ");
        oled2.setCursor(6, contentYPosition + 3 * lineHeight);
        oled2.print("FUEL : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight);
        oled2.print(rpmStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight);
        oled2.print(speedStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight);
        oled2.print(engStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight);
        oled2.print(fuelStr);
    }
    else
    {
        oled2.setDrawColor(0);
        oled2.drawBox(45, 15, 70, 47);

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25);
        oled2.print(rpmStr);
        oled2.setCursor(50, 37);
        oled2.print(speedStr);
        oled2.setCursor(50, 49);
        oled2.print(engStr);
        oled2.setCursor(50, 61);
        oled2.print(fuelStr);
    }
    oled2.sendBuffer();
}
