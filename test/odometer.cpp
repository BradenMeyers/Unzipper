#include <Arduino.h>

int hallEffectPin = 14;
int rotations = 0;

void setup(){
    Serial.begin(115200);
}

void countRotationsHallEffect(){
  static int magneticValue = digitalRead(hallEffectPin);
  if(magneticValue != digitalRead(hallEffectPin)){
    rotations ++;
    magneticValue = !magneticValue;
    Serial.println(rotations);
  }
}

void loop(){
  countRotationsHallEffect();
}