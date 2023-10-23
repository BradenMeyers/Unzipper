#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <remote.h>
#include <motor.h>
#include <serverESP.h>
#include <accelerometer.h>
#include <timer.h>
#include <testCases.h>
#include <main.h>
#include <odometer.h>
#include <gps.h>
Timer recoveryTimer;
Timer atTheTopTimer;
Timer readyTimeOutTimer;
Timer handleBarTimer;
Timer stallTimer;

byte buzzer = 27;
byte motorDirection = 12;  //updated on mar 17
byte motor = 13;  //july5th updated becasue pin 26 on reset it was pulling high. 
byte handleBarSensor = 4;  //was 17 but switched cuz Serial on JULY 4th

#define motorChannel 1
#define resolution 8
#define freq 5000
#define RECOVERY_TO_READY_AFTER_BEEN_GRABBED_TIME 1700
#define brakePos 1200

static int motorAccelerationTimeLimit = 20;  //this determines how fast the motor increses its speed by one PWM value
const char* someoneOnLog = "Stopping motor because someone was on";
const char* wifiStopMotorLog = "Stopping motor because wifi";
const char* stalledLog = "Stopping motor because it is stalled";

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
    case TEST:
      digitalWrite(readyLight, LOW);
      digitalWrite(zipLight, HIGH);
      digitalWrite(recoveryLight, HIGH);
  }
}

void backgroundProcesses(){
  serverLoop();
  remoteLoop();
  loopGPS();
  // Serial.println("processing");
}

bool someoneOn(int timeLimit){
  backgroundProcesses();
  static byte handleBarSensorValue = 1;
  if(digitalRead(handleBarSensor) != handleBarSensorValue){
    if(handleBarTimer.getTime() > timeLimit){
      handleBarSensorValue = !handleBarSensorValue;
    }
  }
  else{handleBarTimer.start();}

  if(handleBarSensorValue){return false;}  //adjust handlebar to be opposite with the pullup
  return true;
}

void turnOnMotor(int speed){
  if(directionStr == "DOWN")
    digitalWrite(motorDirection, HIGH);
  else
    digitalWrite(motorDirection, LOW);
    writeMicroseconds(speed);
  /* if(batteryVoltage()< 16.50  && speed != 0){ 
    speed = speed + 10;
    mainlog.log("motor speed increase due to low battery voltage", true);
  } */
  if(speed == 0){
    //mainlog.log("MOTOR IS OFF------------------", true);
  }
}

void halfSpeedMotor(){
  int halfSpeed = int((motorsMaxSpeed-1500)*0.5);
  halfSpeed +=1500;
  turnOnMotor(halfSpeed);
}

void stopTheMotor(){
  turnOnMotor(OFFPOS);
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

void turnOnBrake(){
  turnOnMotor(brakePos);
}

void takeOffBrake(){
  turnOnMotor(OFFPOS);
}

void readyAtTheTop(){
  logger.checkLogLength();
  readyTimeOutTimer.start();
  turnOnBrake();
  while(!someoneOn(1000)){
    backgroundProcesses();
    if(/* readyTimeOutTimer.getTime() > 120000 or */ wifiSkipToRecovery){
      state = RECOVERY;
      wifiSkipToRecovery = false;
      logger.log("Wireless command to Recovery", true);
      return;
    }
    while(racecarMode){
      turnOnMotor(racecarSpeed);      //check here
      backgroundProcesses();
    }
    if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
    if(state == TEST){
      return;
    }
  }
  state = ZIPPING;
  takeOffBrake();
}

void movingDown(){
  startCount();
  while(someoneOn(1000)){
    loopOdometer();
    if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
    if(state == TEST){
      return;
    }
  }
  stopCount();
  turnOnBrake();
  state = RECOVERY;
}

bool initialEscapeRecovery(){
  if(wifiStopMotor){
      wifiStopMotor = false;
      stopTheMotor();
      logger.log(wifiStopMotorLog, true);
      return true;
    }
  else if(someoneOn(500)){
      stopTheMotor();
      logger.log(someoneOnLog, true);
      return true;
  }
  else
    return false;
}

bool escapeRecovery(int motorSpeedparam){
  loopOdometer();  //does this need to be somewhere else? 
  // if(someoneOn(50)){
  //     logger.log(someoneOnLog, true);
  //     return true;
  //   }
    if(wifiStopMotor){
      logger.log(wifiStopMotorLog, true);
      return true;
    }
    if((stallTimer.getTime() > 1000)  and isStalled()){
      logger.log(stalledLog, true);
      return true;
    }
    // if(!movingStable()){
    //   logger.log("System is unstable while moving", true);
    //   return true;
    // }
  return false;
}

void moveToTop(){
  digitalWrite(buzzer, HIGH);
  //atTheTop = false;
  recoveryTimer.start();
  // while(!checkStable(afterZipDelay*1000)){  //!checkStable(afterZipDelay*1000)  //recoveryTimer.getTime() < afterZipDelay*1000
  //   if(initialEscapeRecovery())
  //     return;
  // }
  delay(afterZipDelay*1000);
  logger.log("Begin speeding motor up", true);
  digitalWrite(buzzer, LOW);
  stallTimer.start();
  for(int motorSpeed=1600; motorSpeed<=motorsMaxSpeed; motorSpeed++){
    turnOnMotor(motorSpeed);
    recoveryTimer.start();
    while(recoveryTimer.getTime() < motorAccelerationTimeLimit){
      if(escapeRecovery(motorSpeed)){
        stopTheMotor();
        return;
      }
    }
  }
  turnOnMotor(motorsMaxSpeed);
  recoveryTimer.start();
  logger.log("Motor has reached max speed", true);
  while(true){
    //Serial.println(rotations);
    double distanceToEnd = getDistanceToEnd();
    if(half){
      halfSpeedMotor();
      logger.log("half speed activated from remote", true);
    }
    else if(distanceToEnd < 15){
      halfSpeedMotor();
      logger.log("GPS Triggered");
    }
    if(escapeRecovery(motorsMaxSpeed)){
      break;
    }
    // if(isAtTheTop() and not atTheTop){
    //   turnOnMotor(motorsMaxSpeed - 70);
    //   logger.log("Turning motor down because were at the top", true);
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
  else if(state == RECOVERY){stateStr = "Test";}
  ledStateMachine();
}

void setup(){
  serverSetup();
  Serial.begin(112500);
  analogReadResolution(12);
  pinMode(handleBarSensor, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(motorDirection, OUTPUT);
  // pinMode(motor, OUTPUT);
  // digitalWrite(motor, LOW);
  // ledcSetup(motorChannel, freq, resolution);
  // ledcAttachPin(motor, motorChannel);
  // ledcWrite(motorChannel, OFFPOS);
  remoteSetup();
  setupAccel();
  motorSetup();
  setupOdometer();
  setupGPS();
  // Serial.println("i made it here");
}

void loop(){
  updateVariableStrings();
  logger.log("Current state : ", true);
  logger.log(stateStr);
  if(state == READY){readyAtTheTop();}
  else if(state == ZIPPING){movingDown();}
  else if(state == RECOVERY){moveToTop();}
  else if(state == TEST){testSelect();}
}
