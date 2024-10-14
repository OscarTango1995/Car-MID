#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "temperature.h"
#include "buzzer.h"
#include "elm.h"

DHT dht(15, DHT21); // GPIO pin,Sensor Type
OneWire oneWire(4); // OneWire bus on GPIO pin 4
DallasTemperature sensors(&oneWire);

float insideTemperatureCalibration = 0.0;
float insideHumidityCalibration = -16.0;
float outsideTemperatureCalibration = 1.0;

// Function to initialize the Temperaure sensors
void initTempSensors()
{
    dht.begin();     // Initialize the DHT sensor
    sensors.begin(); // Initialize the DS18B20 sensor
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
    InsideTempHumidity result;
    result.temperature = dht.readTemperature();
    result.humidity = dht.readHumidity();
    return result;
}

Temperatures getTemperatures()
{
    InsideTempHumidity insideTemp = readInsideTemperature();
    delay(750);

    float outsideTemp = readOutsideTemperature();
    delay(750);

    Temperatures temp;
    temp.humidity = insideTemp.humidity + insideHumidityCalibration;
    temp.iat = insideTemp.temperature + insideTemperatureCalibration;
    temp.oat = outsideTemp + outsideTemperatureCalibration;
    return temp;
}