#ifndef ODOM
#define ODOM

#include <Arduino.h>

bool isStalled();

unsigned long startCount();
unsigned long stopCount();

void setupOdometer();

void loopOdometer();

#endif