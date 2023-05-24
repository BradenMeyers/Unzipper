#ifndef ODOM
#define ODOM

#include <Arduino.h>

extern byte odometerHallEffect;
extern byte odometerHallBackup;

bool isStalled();

unsigned long startCount();
unsigned long stopCount();

void setupOdometer();

void loopOdometer();

void disableOdometer(byte HEpin);

#endif