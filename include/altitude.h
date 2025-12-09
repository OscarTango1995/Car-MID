#ifndef ALTITUDE_H
#define ALTITUDE_H

struct Altitude {
    float pressure;
    float altitude;
};
// Function declarations for initializing altiude sensor
void initAltitude();
Altitude getAltitude();
bool waitForAltMillis(unsigned long duration, unsigned long &previousMillis);


#endif // ALTITUDE_H
