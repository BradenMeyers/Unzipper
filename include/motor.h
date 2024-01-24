#ifndef electrom
#define electrom

#include <Arduino.h>

#define OFFPOS 0
#define MOTORINITSPEED 60  //TODO: Make sure this is right start and that motor max speed can't be less than this

void motorSetup();

void turnOnMotor(int speed);

void halfSpeedMotor();


#endif