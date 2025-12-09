#ifndef BUZZER_H
#define BUZZER_H

// Define the pin for the buzzer
extern const int buzzerPin;

// Function declarations
void initBuzzer();
void turnBuzzerOn(bool turn);
void beep(int time);
void turnBuzzerOff();
#endif
