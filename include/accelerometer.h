#ifndef ACCEL
#define ACCEL

#include <Arduino.h>

extern float uprightError;

float getTemperature();

bool movingStable();

void resetGyro();

void setStable();

bool checkStable(int timeStable);

void setupAccel();

#endif