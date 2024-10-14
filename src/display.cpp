#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_I2CDevice.h>
#include"temperature.h"
#include"altitude.h"

// U8G2 constructor for the OLED (replace 0x3C with the correct address if needed)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C oled(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_SW_I2C oled2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

void drawInitialDisplay(U8G2_SH1106_128X64_NONAME_F_SW_I2C &oled, const char* title) {
    oled2.clearDisplay();
    // Draw the outer frame
    oled2.drawFrame(0, 0, 128, 64); // Draw the frame around the entire display

    // Draw title bar
    oled2.setDrawColor(0);  // Set draw color to black for the title bar
    oled2.drawBox(0, 0, 128, 13);  // Draw filled box for the title bar
    oled2.setDrawColor(1);  // Set draw color to white for border
    oled2.drawFrame(0, 0, 128, 13);  // Draw border around title bar

    oled2.setFont(u8g2_font_6x12_tr);
    oled2.setDrawColor(1);  // Set draw color to white for text

    // Center the title
    oled2.setCursor((128 - oled2.getStrWidth(title)) / 2, 10);
    oled2.print(title);
}

void drawInitialAvgDisplay(U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C &oled, const char* content) {
    oled.clearDisplay();
    
    // Draw the outer frame around the entire display
    oled.setDrawColor(1);  // Set draw color to white
    oled.drawFrame(0, 0, 128, 32);  // Draw a border around the display
    
    oled.setFont(u8g2_font_9x15_tr); // Set font for content
    oled.setDrawColor(1);  // Set draw color to white for text

    // Start drawing the content
    int contentYPosition = 15;  // Start position for content
    const int leftPadding = 6; // Left padding for text
    const int lineHeight = 13; // Line height for spacing

    // Tokenize content by newline
    char* token = strtok((char*)content, "\n");

    while (token != NULL) {
        oled.setCursor(leftPadding, contentYPosition);  // Set cursor for each line
        oled.print(token);  // Print each line
        contentYPosition += lineHeight;  // Move down for the next line
        token = strtok(NULL, "\n");  // Get the next line
    }

    oled.sendBuffer();
}


// Function to draw a menu with submenus
void drawMenu(int selectedItem) {
    // Define menu items
    const char* menuItems[] = {
        "1. GPS",
        "2. Temps",
        "3. Altitude",
        "4. Average",
        "5. Service"
    };

    int itemCount = sizeof(menuItems) / sizeof(menuItems[0]);

    // Draw the title bar and its frame
    oled2.setDrawColor(0);  // Set to black
    oled2.drawBox(0, 0, 128, 13);  // Clear the title area (to refresh it)
    oled2.setDrawColor(1);  // Set back to white for the border
    oled2.drawFrame(0, 0, 128, 13);  // Draw the title bar frame

    // Draw the title text
    oled2.setFont(u8g2_font_6x12_tr);
    oled2.setCursor((128 - oled2.getStrWidth("Main Menu")) / 2, 10);
    oled2.print("Main Menu");

    // Draw the content frame (around the menu items area)
    oled2.setDrawColor(1);  // Ensure we're drawing in white
    oled2.drawFrame(0, 12, 128, 52);  // Frame around the content area

    // Clear the content area where the menu items are drawn
    oled2.setDrawColor(0);  // Set to black for clearing
    oled2.drawBox(1, 15, 126, 48);  // Clear the inside content area (leave the frame)

    oled2.setDrawColor(1);  // Set back to white for drawing text

    // Draw the menu items
    oled2.setFont(u8g2_font_7x14_tr);  // Set the font for menu items
    int contentYPosition = 26;  // Start just below the title bar
    int lineHeight = 12;  // Height of each menu item
    int availableHeight = 64 - contentYPosition - 2;  // Total height available for content
    int maxItemsInView = availableHeight / lineHeight;  // Max number of items that fit in the view

    // Calculate the starting index for the visible menu items
    int startIndex = selectedItem - (maxItemsInView / 2);
    if (startIndex < 0) {
        startIndex = 0;
    } else if (startIndex + maxItemsInView > itemCount) {
        startIndex = itemCount - maxItemsInView;
    }

    const int leftPadding = 6;  // Left padding for menu items

    // Loop through the visible menu items and print them
    for (int i = 0; i < maxItemsInView; i++) {
        int currentIndex = startIndex + i;
        if (currentIndex >= itemCount) break;  // Avoid going out of bounds

        // Set cursor position
        oled2.setCursor(leftPadding, contentYPosition + 2 + (i * lineHeight));

        // Print the `>` symbol for the selected item
        if (currentIndex == selectedItem) {
            oled2.print(">");  // Print highlight sign before the selected item
        } else {
            oled2.print(" ");  // Space for alignment
        }

        // Print the menu item text
        oled2.print(menuItems[currentIndex]);
    }

    // Send the updated buffer to the display
    oled2.sendBuffer();
}


void initDisplays() {
    Wire.begin();
    delay(100); 

    oled.setI2CAddress(0x3C << 1);  
    if (!oled.begin()) {
    Serial.println(F("Failed to initialize display"));
    while (1);  
<<<<<<< HEAD
=======
    }
>>>>>>> 01f5be2 (EML 327 Support Added)
    delay(750);
    }

    oled2.setI2CAddress(0x3D << 1);  
    if (!oled2.begin()) {
    Serial.println(F("Failed to initialize display"));
    while (1);  
  }
      delay(750);

}

// Function to display temperature data on the OLED
<<<<<<< HEAD
void drawTemperaturesScreen(Temperatures temp, bool update) {
    char oatStr[10], iatStr[10], rhStr[10], engStr[10];
    sprintf(oatStr, "%d °C", (int)temp.oat);
    sprintf(iatStr, "%d °C", (int)temp.iat);
    sprintf(rhStr, "%d %%", (int)temp.humidity);  
    sprintf(engStr, "%d °C", (int)temp.engine);

    if(!update){
        drawInitialDisplay(oled2,"TEMPERATURE");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;  
        int lineHeight = 12;  

        oled2.setCursor(6, contentYPosition +0* lineHeight); oled2.print("OAT : ");
        oled2.setCursor(6, contentYPosition +1* lineHeight); oled2.print("IAT : ");
        oled2.setCursor(6, contentYPosition +2* lineHeight); oled2.print("RH  : ");
        oled2.setCursor(6, contentYPosition +3* lineHeight); oled2.print("ENG : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight); oled2.print(oatStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight); oled2.print(iatStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight); oled2.print(rhStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight); oled2.print(engStr); 
        oled2.sendBuffer();
    }else{
        oled2.setDrawColor(0); 
        oled2.drawBox(45, 15, 70, 47); 
        
        oled2.setDrawColor(1);
        oled2.setCursor(50, 25); oled2.print(oatStr);
        oled2.setCursor(50, 37); oled2.print(iatStr);
        oled2.setCursor(50, 49); oled2.print(rhStr);
        oled2.setCursor(50, 61); oled2.print(engStr); 
    }
        oled2.sendBuffer();
=======
void drawTemperaturesScreen(Temperatures temp, int coolantTemp) {
    char content[60]; 
    snprintf(content, sizeof(content), "OAT : %d C\nIAT : %d C\nRH  : %d %% \nENG : %d C", 
             (int)temp.oat, (int)temp.iat, (int)temp.humidity,coolantTemp);
    
    drawDisplay(oled2, "TEMPERATURES", content);    
>>>>>>> 01f5be2 (EML 327 Support Added)
}


// Function to display GPS data on the OLED
void drawGPSScreen(bool update) {
    char satStr[10], spdStr[10], dteStr[10];
    sprintf(satStr, "%d ", 4);
    sprintf(spdStr, "%d KM/H", 100);
    sprintf(dteStr, "%d KM", 500);  

    if(!update){
        drawInitialDisplay(oled2,"GPS");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;  
        int lineHeight = 12;  

        oled2.setCursor(6, contentYPosition +0* lineHeight); oled2.print("SAT : ");
        oled2.setCursor(6, contentYPosition +1* lineHeight); oled2.print("SPD : ");
        oled2.setCursor(6, contentYPosition +2* lineHeight); oled2.print("DTE : ");

        oled2.setCursor(50, contentYPosition + 0 * lineHeight); oled2.print(satStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight); oled2.print(spdStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight); oled2.print(dteStr);
        oled2.sendBuffer();
    }else{
        oled2.setDrawColor(0); 
        oled2.drawBox(45, 15, 70, 47); 

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25); oled2.print(satStr);
        oled2.setCursor(50, 37); oled2.print(spdStr);
        oled2.setCursor(50, 49); oled2.print(dteStr);
    }
        oled2.sendBuffer();
}


// Function to display altitude data on the OLED
void drawAltitudeScreen(Altitude altitude,Temperatures temp,bool update) {
    char altStr[10], hpaStr[10], iatStr[10],oatStr[10];
    sprintf(altStr, "%d M", (int)altitude.altitude);
    sprintf(hpaStr, "%d hPa", (int)altitude.pressure);
    sprintf(iatStr, "%d  C", (int)altitude.temperature);
    sprintf(oatStr, "%d  C", (int)temp.oat);  


    if(!update){
        drawInitialDisplay(oled2,"ALTITUDE");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;  
        int lineHeight = 12;  

        oled2.setCursor(6, contentYPosition +0* lineHeight); oled2.print("ALT : ");
        oled2.setCursor(6, contentYPosition +1* lineHeight); oled2.print("PRES: ");
        oled2.setCursor(6, contentYPosition +2* lineHeight); oled2.print("IAT : ");
        oled2.setCursor(6, contentYPosition +3* lineHeight); oled2.print("OAT : ");


        oled2.setCursor(50, contentYPosition + 0 * lineHeight); oled2.print(altStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight); oled2.print(hpaStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight); oled2.print(iatStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight); oled2.print(oatStr);

    }else{
        oled2.setDrawColor(0); 
        oled2.drawBox(45, 15, 70, 47); 

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25); oled2.print(altStr);
        oled2.setCursor(50, 37); oled2.print(hpaStr);
        oled2.setCursor(50, 49); oled2.print(iatStr);
        oled2.setCursor(50, 61); oled2.print(oatStr);

    }
        oled2.sendBuffer();
}

// Function to display avg data on the OLED
void drawAvgScreen(bool primary,bool update) {
    if (primary){
        char avgStr[10], fflStr[16], fuelStr[10],dteStr[10];
        sprintf(avgStr, "%0.1f KM/L", (float)16.35);
        sprintf(fflStr, "%0.2f", (float)12.3);
        sprintf(fuelStr, "%d %%", 30);
        sprintf(dteStr, "%d KM", 300);  

    if(!update){
        drawInitialDisplay(oled2,"AVERAGE");
        oled2.setFont(u8g2_font_7x14_tr);
        int contentYPosition = 25;  
        int lineHeight = 12;  

        oled2.setCursor(6, contentYPosition +0* lineHeight); oled2.print("AVG : ");
        oled2.setCursor(6, contentYPosition +1* lineHeight); oled2.print("FF/L : ");
        oled2.setCursor(6, contentYPosition +2* lineHeight); oled2.print("FUEL: ");
        oled2.setCursor(6, contentYPosition +3* lineHeight); oled2.print("DTE : ");


        oled2.setCursor(50, contentYPosition + 0 * lineHeight); oled2.print(avgStr);
        oled2.setCursor(50, contentYPosition + 1 * lineHeight); oled2.print(fflStr);
        oled2.setCursor(50, contentYPosition + 2 * lineHeight); oled2.print(fuelStr);
        oled2.setCursor(50, contentYPosition + 3 * lineHeight); oled2.print(dteStr);

    }else{
        oled2.setDrawColor(0); 
        oled2.drawBox(45, 15, 70, 47); 

        oled2.setDrawColor(1);
        oled2.setCursor(50, 25); oled2.print(avgStr);
        oled2.setCursor(50, 37); oled2.print(fflStr);
        oled2.setCursor(50, 49); oled2.print(fuelStr);
        oled2.setCursor(50, 61); oled2.print(dteStr);
    }
        oled2.sendBuffer();
    }else{
    char content[60]; 
    snprintf(content, sizeof(content), "AVG : %d KM/L \nDTE : %dKM", 
             20, 500);
    
    drawInitialAvgDisplay(oled, content); 
    }
    
}

void drawServiceScreen() {
    int eoilValue = 178000;
    int goilValue = 173580;

    int eoilcValue = eoilValue + 5000; // Add 5000 to the original value
    int goilcValue = goilValue + 40000; // Add 30000 to the original value
 

    drawInitialDisplay(oled2,"SERVICE");
    oled2.setFont(u8g2_font_7x14_tr);
    int contentYPosition = 25;  
    int lineHeight = 12;  

    oled2.setCursor(6, contentYPosition +0* lineHeight); oled2.print("E-OIL: ");
    oled2.setCursor(6, contentYPosition +1* lineHeight); oled2.print("G-OIL: ");
    oled2.setCursor(6, contentYPosition +2* lineHeight); oled2.print("E-CHG: ");
    oled2.setCursor(6, contentYPosition +3* lineHeight); oled2.print("G-CHG: ");


    oled2.setCursor(50, contentYPosition + 0 * lineHeight); oled2.print(eoilValue);
    oled2.setCursor(50, contentYPosition + 1 * lineHeight); oled2.print(goilValue);
    oled2.setCursor(50, contentYPosition + 2 * lineHeight); oled2.print(eoilcValue);
    oled2.setCursor(50, contentYPosition + 3 * lineHeight); oled2.print(goilcValue);

    oled2.sendBuffer();
}
