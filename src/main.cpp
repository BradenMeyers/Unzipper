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
#define MAX_BUTTON_RETURN 3000
#define MIN_BUTTON_TIME 250
#define MIN_CALIBRATE 6000
#define ROTATIONS_BEFORE_ZIP 12


static int motorAccelerationTimeLimit = 20;  //this determines how fast the motor increses its speed by one PWM value
const char* someoneOnLog = "Stopping motor because someone was on";
const char* wifiStopMotorLog = "Stopping motor because wifi";
const char* buttonPushedLog = "Stopping motor because button was pushed";
const char* stalledLog = "Stopping motor because it is stalled";

void ledStateMachine(){
  static byte readyLight = 25;
  static byte zipLight = 23;
  static byte recoveryLight = 32;
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


unsigned long returnPushed(int maxTime){
  //Returns Time pushed
  static bool started = false;
  if(digitalRead(returnToHomeButton) == 0){
    started = true;
  }
  if(started){
    unsigned long currentTime = returnButtonTimer.getTime();
    if(digitalRead(returnToHomeButton) == 1){
      started = false;
      return currentTime;
    }
    else if(currentTime > maxTime){
      return currentTime;
    }
    else
      return 0;
  }
  else{
    returnButtonTimer.start();
    return 0;
  }
}

void setBeep(int time){
  digitalWrite(buzzer, HIGH);
  delay(time);
  digitalWrite(buzzer, LOW);
}

void beeper_check(){
  if(beep == 1)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);
}

//If time the button has been pushed is within min and max value for RETURN to home function returns true
//if the time is greater than the calibration time it will call home on GPS and set stable
bool checkReturnButton(){
  unsigned long buttonTime = returnPushed(MIN_CALIBRATE);
  if(buttonTime > MIN_BUTTON_TIME){
    if(buttonTime < MAX_BUTTON_RETURN){
      return true;
    }
    else if(buttonTime >= MIN_CALIBRATE){
      //Start calibration sequence
      
      //Turn on all the lights and turn on beep the buzzer
      digitalWrite(32, HIGH);
      digitalWrite(23, HIGH);
      logger.log("Reseting GPS Home and Calibrating", true);
      setBeep(200);
      resetGPShome();
      setStable();
      setBeep(400);
      digitalWrite(32, LOW);
      digitalWrite(23, LOW);
      return false;
    }
  }
  return false;
}

//If the return button is pressed more than time limit return true otherwise false
bool checkStopReturn(int timeLimit){
  if(digitalRead(returnToHomeButton) == 0){
    if(returnButtonTimer.getTime() > timeLimit){
      return true;
    }
  }
  else
    returnButtonTimer.start();

  return false;
}

void stopTheMotor(){      //TODO: Look at this function and move it to motor.cpp change
  recoveryData.endRecord();
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
    if(checkReturnButton()){
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
  bool started = false;
  while(someoneOn(1000)){
    loopOdometer();
    beeper_check();
    
    //Record speed and start counting distance and time when moving
    zippingData.recordSpeed();
    if((getCurrentCount() > ROTATIONS_BEFORE_ZIP) && !started){
      zippingData.startRecord();
      started = true;
    }
    
    if(state == TEST){
      return;
    }
  }
  stopCount();
  if(started)
    zippingData.endRecord();
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
    logger.log(wifiStopMotorLog, true);
  }
  else if(someoneOn(200)){
    logger.log(someoneOnLog, true);
  }
  else if(checkStopReturn(300)){
    logger.log(buttonPushedLog, true);
  }
  else{
    return false;
  }

  stopTheMotor();
  return true;
}

//return true if buttons are pushed, or wifi stop, or stall, return false, Runs accelerometer and GPS
bool escapeRecovery(){   //Escape recovery when motor has turned on
  loopOdometer();  //does this need to be somewhere else? 
  recoveryData.recordSpeed();
  if(someoneOn(50)){
    logger.log(someoneOnLog, true);
  }
  else if(wifiStopMotor){
    logger.log(wifiStopMotorLog, true);
  }
  else if((stallTimer.getTime() > 1000)  and isStalled()){
    logger.log(stalledLog, true);
  }
  // else if(!movingStable()){
  //   logger.log("System is unstable while moving", true);
  // }
  else
    return false;
  
  return true;
}

void moveToTop(){     //TODO: THis is where the work needs to be done order and saftey
  digitalWrite(buzzer, HIGH);     //
  // atTheTop = false;
  recoveryTimer.start();
  // while(!checkStable(500)){ 
  //   if(recoveryTimer.getTime() > 10000) //TODO: make sure this works
  //     break;
  //   if(checkStopReturn(300)){
  //     stopTheMotor();
  //     logger.log(buttonPushedLog, true);
  //     return;
  //   }
  //   backgroundProcesses();
  // }
  recoveryTimer.start();
  returnButtonTimer.start();
  while(recoveryTimer.getTime() < (afterZipDelay*1000)){  
    if(initialEscapeRecovery())
      return;
  }
  logger.log("Begin speeding motor up", true);
  digitalWrite(buzzer, LOW);  //TODO: Have the beeper pulse when motor is on
  stallTimer.start();
  recoveryData.startRecord();
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
      halfSpeedMotor();   //uncomment  for GPS half speed
      // logger.log("GPS Triggered",);
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
  // logger.log("here 1");
  setupAccel();
  // logger.log("here 2");
  motorSetup();
  // logger.log("here 3");
  setupOdometer();
  // logger.log("here 4");
  setupGPS();
  // logger.log("here 5");
  // Serial.println("i made it here");
}

void loop(){
  updateVariableStrings();
  logger.log("Current state: ", true);
  logger.log(stateStr);
  if(state == READY){readyAtTheTop();}
  else if(state == ZIPPING){movingDown();}
  else if(state == RECOVERY){moveToTop();}
  else if(state == TEST){testSelect();}
}
