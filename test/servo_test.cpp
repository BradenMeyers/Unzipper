#include <Arduino.h>
#include <servoBrake.h>

void setup(){
    servoSetup();
}

int pos = 90;

void loop(){
    if(pos <= 180){
        writeServo(pos);
        delay(SERVODELAY/2);
        pos +=30;
    }
    else{
        pos = 0;
        writeServo(pos);
        delay(SERVODELAY*3);
    }
}