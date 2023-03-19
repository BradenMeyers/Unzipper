#ifndef ACCEL
#define ACCEL

#include <Arduino.h>

extern float uprightError;

float getTemperature();

void resetGyro();

void setStable();

bool checkStable(int timeStable);

void setupAccel();

#endif