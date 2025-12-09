#ifndef FUEL_LEVEL_H
#define FUEL_LEVEL_H

// Function to initialize the voltage sensor
void initFuelLevelSensor();
float readVoltage();
float getFuelPercentage(float voltage);
float getFuel();
#endif // VOLTAGE_SENSOR_H
