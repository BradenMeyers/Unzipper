#ifndef ACCEL
#define ACCEL

#include <Arduino.h>

float getTemperature();

void resetGyro();

void setStable();

bool checkStable(int timeStable);

void setupAccel();

#endif