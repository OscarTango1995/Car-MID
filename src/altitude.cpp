#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "altitude.h"

Adafruit_BMP280 bmp;

#define SEALEVELPRESSURE_HPA (1013.0)

float temperatureCalibration = 0.0;

// Function to initialize the Temperaure sensors
void initAltitude()
{
  // Initialize the BMP280 sensor on the second bus
  if (!bmp.begin(0x76))
  { // Use Wire1 for BMP280
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1)
      ;
  }

  // Configure BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_SLEEP,       // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,      // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,     // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,       // Filtering
                  Adafruit_BMP280::STANDBY_MS_4000); // Standby time
}

Altitude getAltitude()
{
  bmp.reset();
  initAltitude();
  delay(750);

  bmp.takeForcedMeasurement();
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  Altitude result;
  result.altitude = altitude;
  result.pressure = pressure;
  result.temperature = temperature;
  return result;
}
