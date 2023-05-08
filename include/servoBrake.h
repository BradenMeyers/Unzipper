#ifndef servoBrake.h
#define ESP32_Servo_h

#include <Arduino.h>

#define STOPPOS 20
#define OFFPOS 90

void servoSetup();

void writeServo(int value);

void writeMicroseconds(int value);


#endif