#include <SPI.h>
#include <SD.h>
#include "sd.h"
#include "gps.h"
#include "average.h"
#include "fuel_level.h"
#include "elm.h"

#define SD_CS 33   // Chip select pin (CS)
#define SD_MOSI 23 // Master Out Slave In (MOSI)
#define SD_MISO 19 // Master In Slave Out (MISO)
#define SD_SCK 18  // Serial Clock (SCK)
#define MAX_SERVICE_ENTRIES 5

int serviceCount = 0;
ServiceEntry services[5]; // Actual definition

int lineCount = 0;
const char *averageFileName = "/average.csv";
const char *serviceFileName = "/service.csv";
const char *tripTotalFileName = "/trip_totals.csv";
const char *gpsFileName = "/gps.csv";
const char *lineCountFileName = "/line_count.txt";
const char *currentFuelFileName = "/current_fuel.txt";
const char *hudFileName = "/hud.txt";

void initAverageFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(averageFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(averageFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.println("Average,Distance Traveled,Fuel Used,Fuel Remaining,Range,Time");
      csvFile.close();
      Serial.println("Average CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to open Average CSV file for writing.");
    }
  }
  else
  {
    Serial.println("Average CSV file already exists. No action taken.");
  }
}

void initServiceFile()
{
  /* if (SD.exists(serviceFileName))
   {
     Serial.println("Average File exists, deleting now...");

     if (SD.remove(serviceFileName))
     {
       Serial.println("Average File deleted successfully");
       initServiceFile();
     }
     else
     {
       Serial.println("Failed to delete average file");
     }

   }*/
  // Check if the CSV file already exists
  if (!SD.exists(serviceFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(serviceFileName, FILE_WRITE);
    if (csvFile)
    {
      // Write header entries
      csvFile.println("ENGINE OIL,178000,10-07-2025");
      csvFile.println("GEAR OIL,173000,10-11-2023");
      csvFile.println("COOLANT,182000,01-06-2025");
      csvFile.println("TIMING,173000,15-10-2023");
      csvFile.println("BATTERY,09-04-2023");
      csvFile.close();
    }
    else
    {
      Serial.println("Failed to open Service CSV file for writing.");
    }
  }
  else
  {
    Serial.println("Service CSV file already exists. No action taken.");
  }
}

void initTripTotalFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(tripTotalFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(tripTotalFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.println("Total Distance Traveled,Total Fuel Used");
      csvFile.close();
      Serial.println("Trip Totals CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to open Trip Totals CSV file for writing.");
    }
  }
  else
  {
    Serial.println("Trip Totals CSV file already exists. No action taken.");
  }
}


void initGPSFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(gpsFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(gpsFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.print("Latitude");
      csvFile.print(",");
      csvFile.print("Longitude");
      csvFile.print(",");
      csvFile.println("Distance");
      csvFile.close();
      Serial.println("GPS CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to create GPS CSV file for writing.");
    }
  }
  else
  {
    Serial.println("GPS CSV file already exists. No action taken.");
  }
}

void initLineCountFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(lineCountFileName))
  {
    // Create the CSV file
    File lineCountFile = SD.open(lineCountFileName, FILE_WRITE);
    if (lineCountFile)
    {
      lineCountFile.println(lineCount);
      lineCountFile.close();
      Serial.println("line count file created");
    }
    else
    {
      Serial.println("Failed to open line count file for writing.");
    }
  }
  else
  {
    Serial.println("line count file already exists. No action taken.");
  }
}

void initCurrentFuelFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(currentFuelFileName))
  {
    // Create the CSV file
    File currentFuelFile = SD.open(currentFuelFileName, FILE_WRITE);
    if (currentFuelFile)
    {
      currentFuelFile.println(getFuel());
      currentFuelFile.close();
      Serial.println("current fuel file created");
    }
    else
    {
      Serial.println("Failed to open current fuel file for writing.");
    }
  }
  else
  {
    Serial.println("current fuel file already exists. No action taken.");
  }
}

void initHUDFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(hudFileName))
  {
    // Create the CSV file
    File hudFile = SD.open(hudFileName, FILE_WRITE);
    if (hudFile)
    {
      int initialBrightness = 5;
      hudFile.println(initialBrightness);
      hudFile.close();
      Serial.println("HUD file created");
    }
    else
    {
      Serial.println("Failed to open HUD file for writing.");
    }
  }
  else
  {
    Serial.println("HUD file already exists. No action taken.");
  }
}

void initSDCard(bool firstTime)
{
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI, 400000))
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
  if (firstTime)
  {
    initAverageFile();
    initLineCountFile();
    initGPSFile();
    initHUDFile();
    initServiceFile();
    initCurrentFuelFile();
    initTripTotalFile();
  }
}

bool writeAvgToCSV(Average avg)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open(averageFileName, FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(avg.average, 5);
    file.print(",");
    file.print(avg.distanceTraveled);
    file.print(",");
    file.print(avg.fuelUsed, 5);
    file.print(",");
    file.print(avg.fuelRemaining, 5);
    file.print(",");
    file.print(avg.dte, 5);
    file.print(",");
    file.println(timestamp);
    file.close();
    Serial.println("Average Data logged to CSV.");
    return true;
  }
  else
  {
    Serial.println("Failed to open average file for writing.");
    return false;
  }
}

void updateTripTotalToCSV(float fuelUsed)
{
  float distanceTraveled = updateGPSAndCalculateDistance();
  Total previousTotal = readTripTotalData();
  File file = SD.open(tripTotalFileName, FILE_WRITE);
  if (file)
  {
    // Always write header first
    file.println("Total Distance Traveled,Total Fuel Used");

    // Then write the latest totals
    file.printf("%.5f,%.5f\n",
                previousTotal.totalDistanceTraveled + distanceTraveled,
                previousTotal.totalFuelUsed + fuelUsed);
    file.close();
    Serial.println("Trip Total Data logged to CSV.");
    writeLineCount();
  }
  else
  {
    Serial.println("Failed to open Trip Total file for updation.");
  }
}

void writeGPSToCSV(double latitude, double longitude, double distance)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open(gpsFileName, FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(latitude, 6);
    file.print(",");
    file.print(longitude), 6;
    file.print(",");
    file.println(distance, 6);
    file.close();
    Serial.println("GPS Data logged to CSV.");
  }
}

void writeLineCount()
{
  lineCount = readLineCount();
  lineCount++;
  File lineCountFile = SD.open(lineCountFileName, FILE_WRITE);
  if (lineCountFile)
  {
    lineCountFile.println(lineCount);
    lineCountFile.close();
  }
}

void writeCurrentFuel(float currentFuel)
{
  File currentFuelFile = SD.open(currentFuelFileName, FILE_WRITE);
  if (currentFuelFile)
  {
    currentFuelFile.println(currentFuel);
    currentFuelFile.close();
  }
}

void writeHUDBrightness(int brightnessLevel)
{
  File hudFile = SD.open(hudFileName, FILE_WRITE);
  if (hudFile)
  {
    hudFile.println(brightnessLevel);
    hudFile.close();
  }
}

void updateServiceEntry(int index)
{
  // Load the current file into memory
  File file = SD.open(serviceFileName, FILE_READ);
  if (!file)
  {
    Serial.println("Failed to open service.csv for reading.");
    return;
  }

  String lines[5];
  int lineCount = 0;

  while (file.available() && lineCount < 5)
  {
    lines[lineCount++] = file.readStringUntil('\n');
  }
  file.close();

  // Prepare new line content
  String updatedLine;
  if (services[index].mileage > 0)
  {
    updatedLine = services[index].type + "," + String(services[index].mileage) + "," + services[index].date;
  }
  else
  {
    updatedLine = services[index].type + "," + services[index].date;
  }

  // Replace specific line
  if (index < lineCount)
  {
    lines[index] = updatedLine;
  }
  else
  {
    Serial.println("Invalid index for update.");
    return;
  }

  // Rewrite file with updated lines
  file = SD.open(serviceFileName, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open service.csv for writing.");
    return;
  }
  file.close(); // Truncate

  file = SD.open(serviceFileName, FILE_WRITE);
  for (int i = 0; i < lineCount; i++)
  {
    file.println(lines[i]);
  }
  file.close();

  Serial.print("Service entry ");
  Serial.print(index);
  Serial.println(" updated.");
}

int readLineCount()
{
  initSDCard(false);
  delay(300);
  File lineCountFile = SD.open(lineCountFileName, FILE_READ);
  if (lineCountFile)
  {
    lineCount = lineCountFile.parseInt();
    lineCountFile.close();
    return lineCount;
  }
  else
  {
    Serial.println("failed to open line count file");
    return 0;
  }
}

float readCurrentFuel()
{
  initSDCard(false);
  delay(300);
  File currentFuelFile = SD.open(currentFuelFileName, FILE_READ);
  if (currentFuelFile)
  {
    float currentFuel = currentFuelFile.parseFloat();
    currentFuelFile.close();
    return currentFuel;
  }
  else
  {
    Serial.println("failed to open current fuel file");
    return 0;
  }
}

int readHUDBrightness()
{
  initSDCard(false);
  delay(300);
  File hudFile = SD.open(hudFileName, FILE_READ);
  if (hudFile)
  {
    int brightness = hudFile.parseInt();
    Serial.printf("HUD:%d", brightness);
    hudFile.close();
    return brightness;
  }
  else
  {
    Serial.println("failed to open HUD file");
    return 5;
  }
}

void readServiceData()
{
  File file = SD.open("/service.csv");
  if (!file)
  {
    Serial.println("Failed to open service.csv");
    return;
  }

  int index = 0;
  while (file.available() && index < 5)
  {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() == 0)
      continue;

    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);

    ServiceEntry entry;
    entry.type = line.substring(0, firstComma);

    if (secondComma == -1)
    {
      entry.mileage = 0;
      entry.date = line.substring(firstComma + 1);
    }
    else
    {
      entry.mileage = line.substring(firstComma + 1, secondComma).toInt();
      entry.date = line.substring(secondComma + 1);
    }

    services[index++] = entry;
  }

  file.close();
}

GPS readLastCoordinates()
{
  initSDCard(false);
  delay(300);
  GPS gps;
  gps.latitude = 0.0;
  gps.longitude = 0.0;
  gps.distance = 0.0;

  // Open the gps.csv file for reading
  File gpsFile = SD.open(gpsFileName, FILE_READ);
  String lastLine = "";
  while (gpsFile.available())
  {
    String line = gpsFile.readStringUntil('\n'); // Read each line
    if (line.length() > 0)
    {
      lastLine = line; // Store the last non-empty line
    }
  }

  gpsFile.close();

  if (lastLine.length() == 0)
  {
    Serial.println("No valid data found.");
    return gps; // Return empty avg if no valid data
  }
  // Now parse the last line into an Average struct
  int index = 0;
  String value;

  // Split by comma and extract fields
  value = lastLine.substring(0, lastLine.indexOf(','));
  gps.latitude = value.toDouble(); // First value is average
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  gps.longitude = value.toDouble(); // Second value is distance travelled
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  gps.distance = value.toDouble(); // Third value is fuel level
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);
  return gps;
}

Average readLastAverageData()
{
  initSDCard(false);
  delay(300);
  File file = SD.open(averageFileName, FILE_READ);

  Average avg = {0, 0, 0.0, 0.0, 0.0};

  if (!file)
  {
    Serial.println("Failed to open file for reading.");
    return avg;
  }

  if (file.size() == 0)
  {
    Serial.println("File is empty.");
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
  avg.distanceTraveled = value.toDouble(); // Second value is distance travelled
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.fuelUsed = value.toFloat(); // Third value is fuel level
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.fuelRemaining = value.toFloat(); // fourth value is fuel Remaining
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.dte = value.toFloat(); // Fourth value is dte

  return avg;
}

Total readTripTotalData()
{
  initSDCard(false);
  delay(300);
  File file = SD.open(tripTotalFileName, FILE_READ);

  Total tripTotal = {0.0, 0.0};

  if (!file)
  {
    Serial.println("Failed to open file for reading.");
    return tripTotal;
  }

  if (file.size() == 0)
  {
    Serial.println("File is empty.");
    return tripTotal;
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
    return tripTotal; // Return empty avg if no valid data
  }
  // Skip header line if it’s the last one
  if (lastLine.startsWith("Total"))
  {
    Serial.println("Last line is header, ignoring.");
    return tripTotal;
  }

  // Now parse the last line into an Average struct
  String value;

  // Split by comma and extract fields
  value = lastLine.substring(0, lastLine.indexOf(','));
  tripTotal.totalDistanceTraveled = value.toFloat();

  // fuel = everything after the comma
  value = lastLine.substring(lastLine.indexOf(',') + 1);
  tripTotal.totalFuelUsed = value.toFloat();

  return tripTotal;
}

void resetAverageFile()
{
  if (SD.exists(averageFileName))
  {
    Serial.println("Average File exists, deleting now...");

    if (SD.remove(averageFileName))
    {
      Serial.println("Average File deleted successfully");
      initAverageFile();
    }
    else
    {
      Serial.println("Failed to delete average file");
    }
  }
}

void resetGPSFile()
{
  if (SD.exists(gpsFileName))
  {
    Serial.println("GPS File exists, deleting now...");

    if (SD.remove(gpsFileName))
    {
      Serial.println("GPS File deleted successfully");
      initGPSFile();
      updateGPSAndCalculateDistance();
    }
    else
    {
      Serial.println("Failed to delete gps file");
    }
  }
}

void resetTripTotalFile()
{
  if (SD.exists(tripTotalFileName))
  {
    Serial.println("Trip File exists, deleting now...");

    if (SD.remove(tripTotalFileName))
    {
      Serial.println("Trip File deleted successfully");
      initTripTotalFile();
    }
    else
    {
      Serial.println("Failed to delete Trip Total file");
    }
  }
}

void resetLineCount()
{
  Serial.println("Resetting line count");
  File lineCountFile = SD.open(lineCountFileName, FILE_WRITE);
  if (lineCountFile)
  {
    lineCountFile.println(0);
    lineCountFile.close();
  }
}

void resetCurrentFuel()
{
  if (SD.exists(currentFuelFileName))
  {
    Serial.println("current fuel File exists, deleting now...");

    if (SD.remove(currentFuelFileName))
    {
      Serial.println("current fuel File deleted successfully");
      initCurrentFuelFile();
    }
    else
    {
      Serial.println("Failed to delete current fuel file");
    }
  }
}

void deleteAllFiles()
{
  resetAverageFile();
  resetLineCount();
  resetGPSFile();
  resetCurrentFuel();
  resetTripTotalFile();
  initSDCard(true);
}

int getKmAdder(int index)
{
  switch (index)
  {
  case 0:
    return 5000; // ENG
  case 1:
    return 40000; // GEAR
  case 2:
    return 40000; // COOLANT
  case 3:
    return 80000; // TIMING
  case 4:
    return 0; // BATTERY (date only)
  default:
    return 0;
  }
}
