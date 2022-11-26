#include <Arduino.h>

const int IN = 33; //
int Irvalue = 0; //
const int OUT = 19;

void setup() {
  pinMode(OUT, OUTPUT);
  digitalWrite(OUT, HIGH);
  Serial.begin(115200);
}

void loop() {
  Irvalue = analogRead(IN);
  Serial.print("IR Value: ");
  Serial.println(Irvalue); 
}