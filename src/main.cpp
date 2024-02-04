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
Timer returnButtonTimer;
Timer stallTimer;

byte buzzer = 27;
byte handleBarSensor = 33;  //TODO: New hall effect swtich i think i did that
byte returnToHomeButton = 18;
bool autonomousReturn = true;

#define RECOVERY_TO_READY_AFTER_BEEN_GRABBED_TIME 1700

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
  //remoteLoop();
  loopGPS();
  // Serial.println("processing");
}

void changeAutonomy(bool newSetting){
  autonomousReturn = newSetting;
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

bool returnPushed(int timeLimit){
  //Returns a true if the button has been pushed for more than timeLimit
  //Returns a false otherwise
  static byte returnbuttonValue = 1;
  if(digitalRead(returnToHomeButton) == 0){
    if(returnButtonTimer.getTime() > timeLimit){
      returnbuttonValue = 0;
    }
  }
  else{returnButtonTimer.start();}

  if(returnbuttonValue){return false;}  //adjust handlebar to be opposite with the pullup
  returnbuttonValue = 1;
  return true;
}

void beeper_check(){
  if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
}

void stopTheMotor(){      //TODO: Look at this function and move it to motor.cpp change
  turnOnMotor(OFFPOS);
  digitalWrite(buzzer, LOW);
  wifiStopMotor = false;
  recoveryTimer.start();
  while(recoveryTimer.getTime() < RECOVERY_TO_READY_AFTER_BEEN_GRABBED_TIME){}
  state = READY;
}

void readyAtTheTop(){
  logger.checkLogLength();
  readyTimeOutTimer.start();
  while(!someoneOn(1000)){
    backgroundProcesses();
    if(returnPushed(500)){
      state = RECOVERY;
      logger.log("Recovery button pushed", true);
      return;
    }
    if(wifiSkipToRecovery){
      state = RECOVERY;
      wifiSkipToRecovery = false;
      logger.log("Wireless command to Recovery", true);
      return;
    }
    while(racecarMode){
      turnOnMotor(racecarSpeed);      //TODO: Move this into remote file 
      backgroundProcesses();
    }
    beeper_check();
    if(state == TEST){
      return;
    }
  }
  state = ZIPPING;
}

void movingDown(){
  startCount();
  while(someoneOn(1000)){
    loopOdometer();
    beeper_check();
    if(state == TEST){
      return;
    }
  }
  stopCount();
  if(autonomousReturn){
    state = RECOVERY;
  }
  else{
    state = READY;
  }
}

bool initialEscapeRecovery(){     //Escape recovery before motor turns on
  if(wifiStopMotor){
      wifiStopMotor = false;
      stopTheMotor();
      logger.log(wifiStopMotorLog, true);
      return true;
    }
  if(someoneOn(200)){
      stopTheMotor();
      logger.log(someoneOnLog, true);
      return true;
  }
  return false;
}

bool escapeRecovery(){   //Escape recovery when motor has turned on
  loopOdometer();  //does this need to be somewhere else? 
    if(someoneOn(50)){
      logger.log(someoneOnLog, true);
      return true;
    }
    if(wifiStopMotor){
      logger.log(wifiStopMotorLog, true);
      return true;
    }
    if((stallTimer.getTime() > 1000)  and isStalled()){
      logger.log(stalledLog, true);
      return true;
    }
    if(!movingStable()){
      logger.log("System is unstable while moving", true);
      return true;
    }
  return false;
}

void moveToTop(){     //TODO: THis is where the work needs to be done order and saftey
  digitalWrite(buzzer, HIGH);     //
  // atTheTop = false;
  recoveryTimer.start();
  while(!checkStable(500)){ 
    if(recoveryTimer.getTime() > 10000) //TODO: make sure this works
      break;
    backgroundProcesses();
  }
  recoveryTimer.start();
  while(recoveryTimer.getTime() < (afterZipDelay*1000)){  
    if(initialEscapeRecovery())
      return;
  }
  logger.log("Begin speeding motor up", true);
  digitalWrite(buzzer, LOW);  //TODO: Have the beeper pulse when motor is on
  stallTimer.start();
  for(int motorSpeed=MOTORINITSPEED; motorSpeed<=motorsMaxSpeed; motorSpeed++){  //TODO: Move this to Motor.cpp
    turnOnMotor(motorSpeed);
    recoveryTimer.start();
    while(recoveryTimer.getTime() < motorAccelerationTimeLimit){
      if(escapeRecovery()){
        stopTheMotor();
        return;
      }
    }
  }
  turnOnMotor(motorsMaxSpeed);
  logger.log("Motor has reached max speed", true);
  while(true){
    //Serial.println(rotations);
    double distanceToEnd = getDistanceToEnd();
    if(half){
      halfSpeedMotor();
      logger.log("half speed activated from remote", true);
    }
    else if(distanceToEnd < 15){
      //halfSpeedMotor();   //CHange back  for GPS half
      logger.log("GPS Triggered");
    }
    if(escapeRecovery()){
      break;
    }  
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
  Serial.begin(112500);
  serverSetup();
  analogReadResolution(12);
  pinMode(returnToHomeButton, INPUT_PULLUP);
  pinMode(handleBarSensor, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  //remoteSetup();   //TODO: Eventually fix the remote
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
