#ifndef servoBrake
#define servoBrake

#include <Arduino.h>

extern int stopPosServo;
#define OFFPOS 122
#define SERVODELAY 200      //amazon specs said 150 miliseconds for 60 degrees at 5V and 130 at 6.8V

void servoSetup();

void writeServo(int value);

void writeMicroseconds(int value);

#endif