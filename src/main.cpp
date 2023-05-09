#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <remote.h>
#include <servoBrake.h>
#include <serverESP.h>
#include <accelerometer.h>
Logger mainlog;
#include <timer.h>
Timer recoveryTimer;
Timer atTheTopTimer;
Timer readyTimeOutTimer;
Timer stallTimer;
Timer handleBarTimer;
Timer odometerTimer;
#define DELAYODOMETER 5

byte buzzer = 27;
byte motorDirection = 12;  //updated on mar 17
byte motor = 26;  //updated on mar 17
byte handleBarSensor = 17;  //was 19
byte odometerHallEffect = 33;
#define ODOMETERSTALLRATE 160

#define motorChannel 1
#define resolution 8
#define freq 5000
#define RECOVERY_TO_READY_AFTER_BEEN_GRABBED_TIME 1700



void ledStateMachine(){
  static byte readyLight = 25;
  static byte zipLight = 32;
  static byte recoveryLight = 23;
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    pinMode(readyLight, OUTPUT);
    pinMode(zipLight, OUTPUT);
    pinMode(recoveryLight, OUTPUT);
  }
  switch(state){
    case READY:
      digitalWrite(readyLight, HIGH);
      digitalWrite(zipLight, LOW);
      digitalWrite(recoveryLight, LOW);
      break;
    case ZIPPING:
      digitalWrite(readyLight, LOW);
      digitalWrite(zipLight, HIGH);
      digitalWrite(recoveryLight, LOW);
      break; 
    case RECOVERY:
      digitalWrite(readyLight, LOW);
      digitalWrite(zipLight, LOW);
      digitalWrite(recoveryLight, HIGH);
      break;
  }
}

//bool atTheTop = false;
//bool someonOnStateChanged = false;


unsigned long rotations = 0;
unsigned long beforeRotations;
unsigned long countToTopLimit;
unsigned long countToTopLimitOffset = 20;

bool shouldCheckAtTheTop = true;

// void countRotations(){
//   static bool crossedThreshold = false;
//   for(int times=0; times<=4; times++){
//   odometerSensorValue = analogRead(odometerSensor);
//   if(odometerSensorValue > highThreshold and not crossedThreshold){
//     //mainlog.logln(rotations);
//     rotations++;
//     crossedThreshold = true;
//     Serial.println(rotations);
//   }
//   else if(odometerSensorValue < lowThreshold){crossedThreshold = false;}
//   }
// }

void countRotationsHallEffect(int pin){
  int magneticValue = digitalRead(pin);
  static int lastValue = digitalRead(pin);
  if(odometerTimer.getTime() > DELAYODOMETER){
    if(magneticValue != lastValue){
      rotations ++;
      lastValue = magneticValue;
      odometerTimer.start();
      // Serial.println(rotations);
    }
  }
}

void startCount(){
  beforeRotations = rotations;
  mainlog.log("Before Rotations: ");
  mainlog.log(beforeRotations, true);
}

void stopCount(){
  if(rotations - beforeRotations < 10){   //This is if someone pulls it at the top and stuff dont actully go down
    countToTopLimit = rotations + 500;
    return;
  }
  countToTopLimit = (rotations + (rotations - beforeRotations)) - countToTopLimitOffset;
  mainlog.log("Count limit: \n");
  mainlog.log(countToTopLimit);
  mainlog.log("Rotatons: \n");
  mainlog.log(rotations);
}

bool isAtTheTop(){
  
  if(shouldCheckAtTheTop){
    if(rotations > countToTopLimit){return true;}
  }
  return false;
}

bool isStalled(){
  static unsigned long stallRotationCount;
  countRotationsHallEffect(odometerHallEffect);
  // If there have been no rotations in less than 160 milliseconds will return true, otherwise false
  if(stall){  
    if(not stallTimer.started){
      stallTimer.start();
      stallTimer.started = true;
      stallRotationCount = rotations;
      return false;
    }
    else if(stallTimer.getTime() > ODOMETERSTALLRATE){
      stallTimer.started = false;
      // Serial.print("rotations per check:");
      // Serial.println(rotations - stallRotationCount);
      if(rotations - stallRotationCount < 1){
        return true;
      }
    }
    return false;
  }
  else{
    // mainlog.log("Stall turned off", true);
    return false;
  }
}

bool someoneOn(int timeLimit){
  serverLoop();
  remoteLoop();
  static byte handleBarSensorValue = 0;
  if(digitalRead(handleBarSensor) != handleBarSensorValue){
    if(handleBarTimer.getTime() > timeLimit){
      handleBarSensorValue = !handleBarSensorValue;
    }
  }
  else{handleBarTimer.start();}

  if(handleBarSensorValue){return true;}  //adjust handlebar to be opposite with the pullup
  return false;
}

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

void readyAtTheTop(){
  bool servoPositionLock = false;
  mainlog.checkLogLength();
  readyTimeOutTimer.start();
  while(!someoneOn(1000)){
    writeServo(STOPPOS);
    if(/* readyTimeOutTimer.getTime() > 120000 or */ wifiSkipToRecovery){
      state = RECOVERY;
      wifiSkipToRecovery = false;
      mainlog.log("Going up the zip line because ready timed out.", true);
      return;
    }
    while(racecarMode){
      turnOnMotor(racecarSpeed);
      remoteLoop();
      serverLoop();
    }
    turnOnMotor(0);
    if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
  }
  writeServo(OFFPOS);
  delay(SERVODELAY);
  state = ZIPPING;
}

void movingDown(){
  startCount();
  while(someoneOn(1000)){
    countRotationsHallEffect(odometerHallEffect);
    if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
  }
  stopCount();
  mainlog.log("Total Rotations that were zipped: ");
  mainlog.log(rotations - beforeRotations, true);
  state = RECOVERY;
  writeServo(STOPPOS);
  delay(SERVODELAY);
}

void stopTheMotor(){
  turnOnMotor(0);
  digitalWrite(buzzer, LOW);
  /* if(!wifiStopMotor){
    if(atTheTopTimer.getTime() > 2000){countToTopLimitOffset += 2;}
    else if(atTheTopTimer.getTime() < 1000){countToTopLimitOffset -= 2;}
  } */
  wifiStopMotor = false;
  recoveryTimer.start();
  while(recoveryTimer.getTime() < RECOVERY_TO_READY_AFTER_BEEN_GRABBED_TIME){}
  state = READY;
}

void moveToTop(){
  const char* someoneOnLog = "Stopping motor because someone was on";
  const char* wifiStopMotorLog = "Stopping motor because wifi";
  const char* stalledLog = "Stopping motor because it is stalled";
  static int motorAccelerationTimeLimit = 25;
  digitalWrite(buzzer, HIGH);
  //atTheTop = false;
  recoveryTimer.start();
  while(!checkStable(afterZipDelay*1000)){  //!checkStable(afterZipDelay*1000)  //recoveryTimer.getTime() < afterZipDelay*1000
    if(wifiStopMotor){
      wifiStopMotor = false;
      stopTheMotor();
      mainlog.log(wifiStopMotorLog, true);
      return;
    }
    if(someoneOn(500)){
      stopTheMotor();
      mainlog.log(someoneOnLog, true);
      return;
    }
  }
  writeServo(OFFPOS);
  delay(SERVODELAY);
  mainlog.log("Begin speeding motor up", true);
  digitalWrite(buzzer, LOW);
  for(int motorSpeed=50; motorSpeed<=motorsMaxSpeed; motorSpeed++){
    turnOnMotor(motorSpeed);
    recoveryTimer.start();
    while(recoveryTimer.getTime() < motorAccelerationTimeLimit){
      countRotationsHallEffect(odometerHallEffect);
      if(someoneOn(50)){
        stopTheMotor();
        mainlog.log(someoneOnLog, true);
        return;
      }
      if(wifiStopMotor){
        stopTheMotor();
        mainlog.log(wifiStopMotorLog, true);
        return;
      }
      if((motorSpeed > (motorsMaxSpeed - 30)) and isStalled()){
        mainlog.log(stalledLog, true);
        stopTheMotor();
        return;
      }
      if(!movingStable()){
        stopTheMotor();
        mainlog.log("System is unstable", true);
        return;
      }
    }
  }
  mainlog.log("Motor has reached max speed", true);
  while(true){
    //Serial.println(rotations);
    countRotationsHallEffect(odometerHallEffect);
    if(half){
      turnOnMotor(motorsMaxSpeed/2);
      mainlog.log("half speed activated from remote", true);
    }
    if(someoneOn(50)){
      mainlog.log(someoneOnLog, true);
      break;
    }
    if(wifiStopMotor){
      mainlog.log(wifiStopMotorLog, true);
      break;
    }
    if(isStalled()){
      mainlog.log(stalledLog, true);
      stopTheMotor();
      break;
    }
    if(!movingStable()){
      stopTheMotor();
      mainlog.log("System is unstable", true);
      break;
    }
    // if(isAtTheTop() and not atTheTop){
    //   turnOnMotor(motorsMaxSpeed - 70);
    //   mainlog.log("Turning motor down because were at the top", true);
    //   atTheTop = true;
    //   atTheTopTimer.start();
    // }
  }
  stopTheMotor();
}

void updateVariableStrings(){
  if(state == READY){stateStr = "Ready";}
  else if(state == ZIPPING){stateStr = "Zipping";}
  else if(state == RECOVERY){stateStr = "Recovery";}
  ledStateMachine();
}

void setup(){
  serverSetup();
  Serial.begin(112500);
  analogReadResolution(12);
  pinMode(odometerHallEffect, INPUT);
  pinMode(handleBarSensor, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(motorDirection, OUTPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);
  ledcSetup(motorChannel, freq, resolution);
  ledcAttachPin(motor, motorChannel);
  ledcWrite(motorChannel, 0);
  remoteSetup();
  setupAccel();
  servoSetup();
}

void loop(){
  updateVariableStrings();
  mainlog.log("Current state : ");
  mainlog.log(stateStr, true);
  if(state == READY){readyAtTheTop();}
  else if(state == ZIPPING){movingDown();}
  else if(state == RECOVERY){moveToTop();}
}
