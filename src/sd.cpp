#include <SPI.h>
#include <SD.h>
#include "gps.h"
#include "average.h"

#define SD_CS 33   // Chip select pin (CS)
#define SD_MOSI 23 // Master Out Slave In (MOSI)
#define SD_MISO 19 // Master In Slave Out (MISO)
#define SD_SCK 18  // Serial Clock (SCK)
extern float totalFuelFlow;
extern double totalDistance;
extern float average;

void initAverageFile()
{
  // Check if the CSV file already exists
  if (!SD.exists("/average.csv"))
  {
    // Create the CSV file
    File csvFile = SD.open("/average.csv", FILE_WRITE);
    if (csvFile)
    {
      csvFile.println("Average,Distance,Fuel Level,Range,Time"); // Write headers
      csvFile.close();
      Serial.println("CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to open CSV file for writing.");
    }
  }
  else
  {
    Serial.println("CSV file already exists. No action taken.");
  }
}

void initSDCard()
{
  // Initialize SPI with custom pins
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Start the SD card module with custom settings
  if (!SD.begin(SD_CS, SPI, 4000000)) // You can adjust the frequency as needed
  {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  initAverageFile();
}

bool writeAvgToCSV(Average avg)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open("/average.csv", FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(avg.average, 2);
    file.print(",");
    file.print(avg.distance);
    file.print(",");
    file.print(avg.fuel, 2);
    file.print(",");
    file.print(avg.dte, 2);
    file.print(",");
    file.println(timestamp);
    file.close();
    Serial.println("Data logged to CSV.");
    return true;
  }
  else
  {
    Serial.println("Failed to open file for writing.");
    return false;
  }
}

Average readLastAverageData()
{
  initSDCard();
  delay(750);
  File file = SD.open("/average.csv", FILE_READ);

  Average avg;
  avg.average = 0;
  avg.distance = 0;
  avg.fuel = 0.0;
  avg.dte = 0.0;

  if (file.size() == 0)
  {
    Serial.println("File is empty.");
    return avg;
  }
  if (!file)
  {
    Serial.println("Failed to open file for reading.");
    return avg;
  }

  String lastLine = "";
  while (file.available())
  {
    String line = file.readStringUntil('\n'); // Read each line
    if (line.length() > 0)
    {
      lastLine = line; // Store the last non-empty line
    }
  }

  file.close();

  if (lastLine.length() == 0)
  {
    Serial.println("No valid data found.");
    return avg; // Return empty avg if no valid data
  }

  // Now parse the last line into an Average struct
  int index = 0;
  String value;

  // Split by comma and extract fields
  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.average = value.toFloat(); // First value is average
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.distance = value.toDouble(); // Second value is distance
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.fuel = value.toFloat(); // Third value is fuel level
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.dte = value.toFloat(); // Fourth value is dte

  return avg;
}

void deleteAverageFile()
{
  if (SD.exists("/average.csv"))
  {
    Serial.println("File exists, deleting now...");

    if (SD.remove("/average.csv"))
    {
      Serial.println("File deleted successfully");
      initSDCard();
    }
    else
    {
      Serial.println("Failed to delete file");
    }
  }
  else
  {
    Serial.println("File does not exist");
  }
}