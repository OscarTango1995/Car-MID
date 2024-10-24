#include <DHT.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_I2CDevice.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "temperature.h"
#include "buzzer.h"
#include "elm.h"

Adafruit_AHTX0 insideTempSensor;
OneWire oneWire(4); // OneWire bus on GPIO pin 4
DallasTemperature sensors(&oneWire);

float insideTemperatureCalibration = 0.0;
float insideHumidityCalibration = 0.0;
float outsideTemperatureCalibration = 1.0;
unsigned long oatTempMillis = 0;
unsigned long iatTempMillis = 0;

bool waitForTempMillis(unsigned long duration, unsigned long &previousMillis)
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= duration)
    {
        previousMillis = currentMillis; // Reset the timer
        return true;                    // Delay period is over
    }
    return false; // Still waiting
}

// Function to initialize the Temperaure sensors
void initTempSensors()
{
    sensors.begin(); // Initialize the DS18B20 sensor
    Wire.begin();
    delay(150);
    // Initialize AHT21
    insideTempSensor.begin();
}

float readOutsideTemperature()
{
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

float readEngineTemperature()
{
    return readCoolantTemp();
}

InsideTempHumidity readInsideTemperature()
{
    insideTempSensor.begin();
    delay(150);
    sensors_event_t humidityEvent, temperatureEvent;
    insideTempSensor.getEvent(&humidityEvent, &temperatureEvent);

    InsideTempHumidity result;
    result.temperature = temperatureEvent.temperature;
    result.humidity = humidityEvent.relative_humidity;
    return result;
}

void readTemp()
{
    insideTempSensor.begin();
    delay(150);
    sensors_event_t humidityEvent, temperatureEvent;
    insideTempSensor.getEvent(&humidityEvent, &temperatureEvent);
    Serial.print("AHT Temp:");
    Serial.println(temperatureEvent.temperature + insideTemperatureCalibration);
    Serial.print("AHT Humidity:");
    Serial.println(humidityEvent.relative_humidity + insideHumidityCalibration);
    Serial.println();
}

Temperatures getTemperatures()
{
    static InsideTempHumidity lastInsideTemp = {0.0, 0.0}; // Store last valid readings
    static float lastOutsideTemp = 0.0;

    if (waitForTempMillis(750, iatTempMillis))
    {
        lastInsideTemp = readInsideTemperature();
        iatTempMillis = millis();
    }
    if (waitForTempMillis(750, oatTempMillis))
    {
        lastOutsideTemp = readOutsideTemperature();
        oatTempMillis = millis();
    }

    Temperatures temp;
    temp.humidity = lastInsideTemp.humidity + insideHumidityCalibration;
    temp.iat = lastInsideTemp.temperature + insideTemperatureCalibration;
    temp.oat = lastOutsideTemp + outsideTemperatureCalibration;
    return temp;
}