#ifndef motor
#define motor

#include <Arduino.h>

#define OFFPOS 1500
#define SERVODELAY 200      //amazon specs said 150 miliseconds for 60 degrees at 5V and 130 at 6.8V


void motorSetup(int pin);

void writeServo(int value);

void writeMicroseconds(int value);

#endif