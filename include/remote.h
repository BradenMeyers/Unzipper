#ifndef REMOTE
#define REMOTE

#include <Arduino.h>

extern int stall;
extern int half;
extern int beep;

extern bool racecarMode;
extern int racecarSpeed;

void remoteSetup();

void remoteLoop();

#endif