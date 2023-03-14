#ifndef SERVERESP
#define SERVERESP

#include <Arduino.h>
#include <string>

#define READY 1
#define ZIPPING 2
#define RECOVERY 3

extern byte state;
extern String stateStr;
extern int serverState;

extern byte odometerSensor;
extern int odometerSensorValue;

extern int motorsMaxSpeed;
extern String motorMaxSpeedStr;
extern int afterZipDelay;
extern String afterZipDelayStr;  //cause actual variable is in miliseconds but wifi is in seconds
extern String directionStr;
extern bool wifiStopMotor;
extern bool wifiSkipToRecovery;

extern int highThreshold;
extern String highThresholdStr;
extern int lowThreshold;
extern String lowThresholdStr;

void serverSetup();

void serverLoop();

class Logger{
public:
  String(logStr) = "";
  String newLine = "<br>";
  void log(const char* message, bool ln=false){
    if(ln){
      logNewLine();
    }
    logStr = message + logStr;
    Serial.print(message);
  }
  void log(String string, bool ln=false){
    if(ln){
      logNewLine();
    }
    logStr = string + logStr;
    Serial.print(string);
  }
  void log(int i, bool ln=false){
    if(ln){
      logNewLine();
    }
    logStr = String(i) + logStr;
    Serial.print(i);
  }
  void log(unsigned long l, bool ln=false){
    Serial.print(l);
    if(ln){
      logNewLine();
    }
    logStr = String(l) + logStr;
  }
  void logNewLine(){
    Serial.println();
    logStr = newLine + logStr;
    logStr = millis() * 0.001 + logStr;
    logStr = " - " + logStr;
  }
  void checkLogLength(){
    if(logStr.length() > 1100){
      logStr = logStr.substring(100);
    }
  }
  void clearLog(){
    logStr = "";
  }
};

float batteryVoltage();

#endif