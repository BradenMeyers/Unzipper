#ifndef REMOTE
#define REMOTE

#include <Arduino.h>

extern int stall;
extern int half;

void remoteSetup();

void remoteLoop();

#endif