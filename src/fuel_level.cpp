#include <Arduino.h>
#define VOLTAGE_PIN 32             // Use ADC pin (GPIO34 in this case)
#define REF_VOLTAGE 3.3            // Reference voltage (3.3V for ESP32)
#define ADC_MAX 4095               // Maximum ADC value for 12-bit resolution
#define VOLTAGE_DIVIDER_RATIO 5.56 // Divider ratio (1/0.2)
const float tankCapacity = 45.0;   // Total tank capacity in liters
const uint8_t tankFull=11;
const uint8_t tankLow=120;


void initFuelLevelSensor()
{
    pinMode(VOLTAGE_PIN, INPUT);
}


float readVoltage()
{
    int adcValue = analogRead(VOLTAGE_PIN);             // Read ADC value
    float voltage = (adcValue * REF_VOLTAGE) / ADC_MAX; // Convert ADC value to voltage
    voltage *= VOLTAGE_DIVIDER_RATIO;                   // Compensate for the voltage divider
    return voltage;
}

float getFuelAvailable()
{
    float vin = readVoltage();

    // Convert voltage to resistance of the sender unit
    float senderResistance = (vin / (REF_VOLTAGE - vin)) * 30000;//30K Ohm known resistor
    Serial.print("Sender resistance: ");
    Serial.println(senderResistance);

    // Map resistance to fuel level (linear mapping)
    float fuelPercentage = (tankLow - senderResistance) / (tankLow - tankFull) * 100.0;
    fuelPercentage = constrain(fuelPercentage, 0, 100); // Keep percentage between 0 and 100
    float fuelLiters = (fuelPercentage / 100.0) * tankCapacity;

    Serial.print("Fuel Level: ");
    Serial.print(fuelLiters);
    Serial.println(" L");

    return fuelLiters;
}
