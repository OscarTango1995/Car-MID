#include <Arduino.h>
#include "fuel_level.h"
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;   // Create instance
#define VOLTAGE_PIN 32  // Use ADC pin (GPIO32 in this case)
#define REF_VOLTAGE 3.3 // Reference voltage (3.3V for ESP32)
#define ADC_MAX 4095    // Maximum ADC value for 12-bit resolution

const float tankCapacity = 35.0; // Total tank capacity in liters

void initFuelLevelSensor()
{
    ads.begin();           // Initializes ADS1115 with default I2C address (0x48)
    ads.setGain(GAIN_TWO); // ±4.096V for ~0.125mV resolution
    pinMode(VOLTAGE_PIN, INPUT);
}

float readVoltage()
{
    float voltageSum = 0.0;
    ads.setGain(GAIN_TWO); // or GAIN_TWO as discussed

    for (int i = 0; i < 10; i++)
    {
        int16_t raw = ads.readADC_SingleEnded(0);

        // Convert to voltage: ADS1115 (gain = 2) => 2.048V full scale
        float voltage = (raw * 2.048) / 32767.0;
        voltageSum += voltage;

        delay(10); // small delay between samples
    }

    float averageVoltage = voltageSum / 10.0;
    return averageVoltage;
}



float getFuel()
{
    float vin = readVoltage();  // Will now always return valid value
    float fuelPercentage = getFuelPercentage(vin);
    float currentFuelLiters = (fuelPercentage / 100.0) * tankCapacity;
    return currentFuelLiters;
}

float getFuelPercentage(float voltage)
{
    // Define voltage and percentage pairs
    //                          E,1/4,1/2,3/4,F
    float voltageLevels[] = {1.30, 1.20, 0.90, 0.42, 0.12};
    float fuelPercentages[] = {0, 25, 50, 75, 100};

    for (int i = 0; i < 4; i++)
    {
        // Find the range where the voltage falls
        if (voltage <= voltageLevels[i] && voltage >= voltageLevels[i + 1])
        {
            // Interpolate between the bounds
            return fuelPercentages[i] + ((voltage - voltageLevels[i]) / (voltageLevels[i + 1] - voltageLevels[i])) * (fuelPercentages[i + 1] - fuelPercentages[i]);
        }
    }

    // Return 0 if below empty or 100 if above full
    return (voltage > voltageLevels[0]) ? 0 : 100;
}
