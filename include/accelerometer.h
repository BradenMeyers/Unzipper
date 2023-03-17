#ifndef ACCEL
#define ACCEL

#include <Arduino.h>

float uprightError;

float getTemperature();

void resetGyro();

void setStable();

bool checkStable(int timeStable);

void setupAccel();

#endif