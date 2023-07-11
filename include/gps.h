#ifndef GPS
#define GPS

#include <Arduino.h>

extern float gpsError;

int satelliteLock();

void resetGPShome();

double generateGPSLock();

void setupGPS();

void loopGPS();

double getDistanceToEnd();

#endif