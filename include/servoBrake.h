#ifndef servoBrake
#define servoBrake

#include <Arduino.h>

#define STOPPOS 20
#define OFFPOS 90
#define SERVODELAY 150      //amazon specs said 150 miliseconds for 60 degrees at 5V and 130 at 6.8V

void servoSetup();

void writeServo(int value);

void writeMicroseconds(int value);

#endif