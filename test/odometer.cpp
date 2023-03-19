#include <Arduino.h>

int hallEffectPin1 = 33;  //14 and 33 on MAR17
int hallEffectPin2 = 14;
int rotations = 0;

void setup(){
    Serial.begin(115200);
    pinMode(hallEffectPin1, INPUT_PULLUP);
    pinMode(hallEffectPin2, INPUT_PULLUP);

}

void countRotationsHallEffect(){
  static int magneticValue = digitalRead(hallEffectPin1);
  if(magneticValue != digitalRead(hallEffectPin1)){
    rotations ++;
    magneticValue = !magneticValue;
    Serial.println(rotations);
  }
}



void loop(){
  //countRotationsHallEffect();
  //Serial.println(digitalRead(hallEffectPin));
  Serial.println(digitalRead(hallEffectPin1));
  Serial.println(digitalRead(hallEffectPin2));
  delay(1000);

}