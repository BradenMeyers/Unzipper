#include <Arduino.h>

int ready = 25;
int zip = 32;
int recov = 23;


void setup(){
    pinMode(ready, OUTPUT);
    pinMode(zip, OUTPUT);
    pinMode(recov, OUTPUT);
}
void loop(){
  delay(1000);
  digitalWrite(ready, HIGH);
  Serial.println("I am here1");
  delay(1000);
  digitalWrite(ready, LOW);
  digitalWrite(zip, HIGH);
  Serial.println("I am here2");
  delay(1000);
  digitalWrite(zip, LOW);
  digitalWrite(recov, HIGH);
  Serial.println("I am here3");
  delay(1000);
  digitalWrite(recov, LOW);
}