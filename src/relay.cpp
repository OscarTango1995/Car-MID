#include <Arduino.h>
#include "relay.h"
#include "elm.h"
#include "buzzer.h"
#include "gps.h"

extern bool carUnlocked;
const int unLockRelayPin = 14;
const int lockRelayPin = 12;
extern int lockStatus;
static bool ecuFailed = false;

void initRelays()
{
    pinMode(lockRelayPin, OUTPUT);
    digitalWrite(lockRelayPin, LOW); // Turn relay off initially
    pinMode(unLockRelayPin, OUTPUT);
    digitalWrite(unLockRelayPin, LOW); // Turn relay off initially
}

void lockCar()
{
    float speed = 0;
    speed = myELM327.kph();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
        ecuFailed = true;
    }

    // Check the speed and lock the car if necessary
    if (speed >= 20)
    {
        Serial.println("Locking Car");
        digitalWrite(lockRelayPin, HIGH); // Turn relay on
        delay(250);
        digitalWrite(lockRelayPin, LOW); // Turn relay off
        carUnlocked = false;
        lockStatus = 1;
    }
}

void unLockCar()
{
    if (carUnlocked)
        return;

    Serial.print("Un-Locking Car");
    digitalWrite(unLockRelayPin, HIGH); // Turn relay on
    delay(250);
    digitalWrite(unLockRelayPin, LOW); // Turn relay off
    lockStatus = 0;
    carUnlocked = true;
}
