#include <Arduino.h>
#include "buzzer.h"

// Define the buzzer pin
const int buzzerPin = 13; // Set your actual buzzer pin here

// Initialize the buzzer pin
void initBuzzer()
{
  pinMode(buzzerPin, OUTPUT);
}

// Turn the buzzer on for 1 second
void turnBuzzerOn(bool turn)
{
  digitalWrite(buzzerPin, HIGH); // Turn the buzzer on
  if (turn)
  {
    delay(150);                   // Buzzer on for 1 second
    digitalWrite(buzzerPin, LOW); // Turn the buzzer off
  }
}
