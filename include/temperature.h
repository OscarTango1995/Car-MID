#ifndef TEMPERATURE_H
#define TEMPERATURE_H

struct InsideTempHumidity {
    float temperature;
    float humidity;
};

struct Temperatures {
    float oat;
    float iat;
    float humidity;
};

void initTempSensors(); 
InsideTempHumidity readInsideTemperature();
float readOutsideTemperature();
float readEngineTemperature();
Temperatures getTemperatures();
void readTemp();
bool waitForTempMillis(unsigned long duration, unsigned long &previousMillis);

#endif // TEMPERATURE_H
