#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "logger.h"

#define SD_CS 5 // Chip Select pin for SD Card
File logFile;

void initLogging()
{
    // Initialize SD card
    if (!SD.begin(SD_CS))
    {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized.");

    logFile = SD.open("/log.txt", FILE_WRITE);
    if (logFile)
    {
        logFile.close();
    }
    else
    {
        Serial.println("Error opening log file!");
    }

    logFile = SD.open("/error.txt", FILE_WRITE);
    if (logFile)
    {
        logFile.close();
    }
    else
    {
        Serial.println("Error opening error file!");
    }

    logFile = SD.open("/avg.txt", FILE_WRITE);
    if (logFile)
    {
        logFile.close();
    }
    else
    {
        Serial.println("Error opening avg file!");
    }
}

void logError(const String &errorMessage)
{
    logFile = SD.open("/error.txt", FILE_APPEND);
    if (logFile)
    {
        logFile.print("ERROR: ");
        logFile.println(errorMessage);
        logFile.close();
        Serial.println("Error logged: " + errorMessage);
    }
    else
    {
        Serial.println("Error opening error file!");
    }
}

void logInfo(const String &infoMessage)
{
    logFile = SD.open("/log.txt", FILE_APPEND);
    if (logFile)
    {
        logFile.print("INFO: ");
        logFile.println(infoMessage);
        logFile.close();
        Serial.println("Info logged: " + infoMessage);
    }
    else
    {
        Serial.println("Error opening log file!");
    }
}

void logAvg(const String &avgMessage)
{
    logFile = SD.open("/avg.txt", FILE_APPEND);
    if (logFile)
    {
        logFile.print("Avg: ");
        logFile.println(avgMessage);
        logFile.close();
        Serial.println("Avg logged: " + avgMessage);
    }
    else
    {
        Serial.println("Error opening avg file!");
    }
}