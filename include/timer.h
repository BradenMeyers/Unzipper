#include <Arduino.h>

class Timer{
public:
  unsigned long currentTime;
  bool started = false;
  void start(){
    currentTime = millis();
  }
  unsigned long getTime(){
    return millis() - currentTime;
  }
  
};