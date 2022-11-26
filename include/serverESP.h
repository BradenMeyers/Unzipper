#ifndef SERVERESP
#define SERVERESP

#include <Arduino.h>

#define READY 1
#define ZIPPING 2
#define RECOVERY 3

extern byte state;

extern byte odometerSensor;
extern int odometerSensorValue;
extern byte motorsMaxSpeed;
extern unsigned long afterZipDelay;

extern int highThreshold;
extern String highThresholdStr;
extern int lowThreshold;
extern String lowThresholdStr;

extern String stateStr;
extern String directionStr;
extern String motorMaxSpeedStr;
extern String afterZipDelayStr;  //cause actual variable is in miliseconds but wifi is in seconds

extern bool wifiStopMotor;
extern bool wifiSkipToRecovery;

void serverSetup();

void serverLoop();

class Logger{
public:
  String(logStr) = "";
  String newLine = "<br>";
  void log(const char* message, bool ln=false){
    logStr += message;
    Serial.print(message);
    if(ln){
      logNewLine();
    }
  }
  void log(String string, bool ln=false){
    logStr += string;
    Serial.print(string);
    if(ln){
      logNewLine();
    }
  }
  void log(int i, bool ln=false){
    logStr += String(i);
    Serial.print(i);
    if(ln){
      logNewLine();
    }
  }
  void log(unsigned long l, bool ln=false){
    logStr += String(l);
    Serial.print(l);
    if(ln){
      logNewLine();
    }
  }
  void logNewLine(){
    logStr += " - ";
    logStr += millis() * 0.001;
    logStr += newLine;
    Serial.println();
  }
  void checkLogLength(){
    if(logStr.length() > 1100){
      logStr = logStr.substring(100);
    }
  }
};

float batteryVoltage();

#endif