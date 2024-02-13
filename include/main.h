#ifndef MAIN
#define MAIN

#include <Arduino.h>

// #define MOTORPIN 13        //should be 13 on new circuit board dec 28th, july 5th updated becasue pin 26 on reset it was pulling high
// #define MOTORDIR 12            //verified dec 28th

#define PWM1 12
#define PWM2 4
#define ENA 13

#define motorChannel 1
#define resolution 8
#define freq 5000

void turnOnMotor(int speed);

void changeAutonomy(bool newSetting);

void setBeep(int time);


#endif