#include <Arduino.h>


#define motorChannel 0
#define resolution 8
#define freq 5000

String directionStr = "UP";

byte motorDirection = 12;
byte motor = 26;

void turnOnMotor(int speed){
  if(directionStr == "DOWN")
    digitalWrite(motorDirection, HIGH);
  else
    digitalWrite(motorDirection, LOW);
  ledcWrite(motorChannel, speed);
  /* if(batteryVoltage()< 16.50  && speed != 0){ 
    speed = speed + 10;
    mainlog.log("motor speed increase due to low battery voltage", true);
  } */
  if(speed == 0){
    //mainlog.log("MOTOR IS OFF------------------", true);
  }
}


void setup(){
    Serial.begin(115200);
    pinMode(motorDirection, OUTPUT);
    pinMode(motor, OUTPUT);
    digitalWrite(motor, LOW);
    ledcSetup(motorChannel, freq, resolution);
    ledcAttachPin(motor, motorChannel);
    ledcWrite(motorChannel, 0);
}

void loop(){
    delay(1000);
    for (int i = 0; i < 200; i++)
    {
        turnOnMotor(i);
        delay(10);
    }
    delay(1000);
    directionStr = "DOWN";
    for (int i = 0; i < 200; i++)
    {
        turnOnMotor(i);
        delay(10);
    }
    delay(1000);
    directionStr = "UP";
    
}