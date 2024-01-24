
#include <Arduino.h>
#include <motor.h>
#include <main.h>
#include <serverESP.h>


bool attached = false;

/*
void attachServo(int pin){
    ledcSetup(MOTORCHANNEL, FREQUENCY, RESOLUTION); // channel #, 50 Hz, timer width
    ledcAttachPin(pin, MOTORCHANNEL);   // GPIO pin assigned to channel
    attached = true;
}

void detachServo()
{
    ledcDetachPin(MOTORPIN);
    attached = false;
}

int usToTicks(int usec)
{
    return (int)((float)usec / ((float)REFRESH_USEC / (float)DEFAULT_TIMER_WIDTH_TICKS));   
}

void writeMicroseconds(int value){
    if (attached)   // ensure channel is valid
    {
        if (value < MINMICRO)          // ensure pulse width is valid
            value = MINMICRO;
        else if (value > MAXMICRO)
            value = MAXMICRO;

        value = usToTicks(value);  // convert to ticks
        // do the actual write
        ledcWrite(MOTORCHANNEL, value);
    }
}



int ticksToUs(int ticks)
{
    return (int)((float)ticks * ((float)REFRESH_USEC / (float)DEFAULT_TIMER_WIDTH_TICKS)); 
}

*/

void turnOnMotor(int speed){
  if(directionStr == "DOWN")
    digitalWrite(MOTORDIR, HIGH);
  else
    digitalWrite(MOTORDIR, LOW);
    ledcWrite(motorChannel, speed);
  /* if(batteryVoltage()< 16.50  && speed != 0){ 
    speed = speed + 10;
    mainlog.log("motor speed increase due to low battery voltage", true);
  } */
  if(speed == 0){
    //mainlog.log("MOTOR IS OFF------------------", true);
  }
}

void halfSpeedMotor(){
  int halfSpeed = int(motorsMaxSpeed*0.5);
  turnOnMotor(halfSpeed);
}

void motorSetup(){
    pinMode(MOTORDIR, OUTPUT);
    pinMode(MOTORPIN, OUTPUT);
    digitalWrite(MOTORDIR, LOW);
    digitalWrite(MOTORPIN, LOW);
    ledcSetup(motorChannel,freq, resolution);
    ledcAttachPin(MOTORPIN, motorChannel);
    turnOnMotor(OFFPOS);
    // Serial.println("Servo setup");
    // delay(100);
}



