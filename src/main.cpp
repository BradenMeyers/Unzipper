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
#include <timer.h>
#include <testCases.h>
#include <main.h>
#include <odometer.h>
#include <gps.h>
Timer recoveryTimer;
Timer atTheTopTimer;
Timer readyTimeOutTimer;
Timer handleBarTimer;

byte buzzer = 27;
byte motorDirection = 12;  //updated on mar 17
byte motor = 26;  //updated on mar 17
byte handleBarSensor = 4;  //was 17 but switched cuz Serial on JULY 4th

#define motorChannel 1
#define resolution 8
#define freq 5000
#define RECOVERY_TO_READY_AFTER_BEEN_GRABBED_TIME 1700

static int motorAccelerationTimeLimit = 25;  //this determines how fast the motor increses its speed by one PWM value
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
  logger.checkLogLength();
  readyTimeOutTimer.start();
  writeServo(OFFPOS);
  while(!someoneOn(1000)){
    backgroundProcesses();
    //writeServo(OFFPOS);
    //writeServo(stopPosServo);      //Uncomment these lines out for the servo to be on during ready mode
    if(/* readyTimeOutTimer.getTime() > 120000 or */ wifiSkipToRecovery){
      state = RECOVERY;
      wifiSkipToRecovery = false;
      logger.log("Wireless command to Recovery", true);
      return;
    }
    while(racecarMode){
      writeServo(OFFPOS); //take break off 
      turnOnMotor(racecarSpeed);
      backgroundProcesses();
    }
    //writeServo(stopPosServo);  //uncomment this line when you turn servo on in ready mode
    turnOnMotor(0);
    if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
    if(state == TEST){
      return;
    }
  }
  //writeServo(OFFPOS);    //Uncomment these lines out for the servo to be on during ready mode
  delay(SERVODELAY);
  state = ZIPPING;
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
  state = RECOVERY;
  writeServo(stopPosServo);
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
  if(someoneOn(50)){
      logger.log(someoneOnLog, true);
      return true;
    }
    if(wifiStopMotor){
      logger.log(wifiStopMotorLog, true);
      return true;
    }
    if((motorSpeedparam > (motorsMaxSpeed - 30)) and isStalled()){
      logger.log(stalledLog, true);
      return true;
    }
    if(!movingStable()){
      logger.log("System is unstable while moving", true);
      return true;
    }
  return false;
}

void moveToTop(){
  digitalWrite(buzzer, HIGH);
  //atTheTop = false;
  recoveryTimer.start();
  while(!checkStable(afterZipDelay*1000)){  //!checkStable(afterZipDelay*1000)  //recoveryTimer.getTime() < afterZipDelay*1000
    if(initialEscapeRecovery())
      return;
  }
  writeServo(OFFPOS); 
  delay(SERVODELAY*1.5);  
  logger.log("Begin speeding motor up", true);
  digitalWrite(buzzer, LOW);
  for(int motorSpeed=50; motorSpeed<=motorsMaxSpeed; motorSpeed++){
    turnOnMotor(motorSpeed);
    recoveryTimer.start();
    while(recoveryTimer.getTime() < motorAccelerationTimeLimit){
      if(escapeRecovery(motorSpeed)){
        stopTheMotor();
        return;
      }
    }
  }
  logger.log("Motor has reached max speed", true);
  while(true){
    //Serial.println(rotations);
    if(half){
      turnOnMotor(motorsMaxSpeed/2);
      logger.log("half speed activated from remote", true);
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
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);
  ledcSetup(motorChannel, freq, resolution);
  ledcAttachPin(motor, motorChannel);
  ledcWrite(motorChannel, 0);
  remoteSetup();
  //setupAccel();
  servoSetup();
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
