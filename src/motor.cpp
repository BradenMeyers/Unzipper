
#include <Arduino.h>
#include <motor.h>

#define MINMICRO 1000
#define MAXMICRO 2000
#define NUETRALMICRO 1500       //PWM specs found on datasheet from part and on Amazon

#define MOTORPIN 13          //GPIO pin 4
#define MOTORCHANNEL 2
#define FREQUENCY 50    //AMAZON Specs for servo said frequency could be between 50-330hz
#define RESOLUTION 16   //found this on esp32 servo library as DEFAULT_TIMER_WIDTH

#define REFRESH_USEC 20000 //conversion for microseconds to ticks
#define DEFAULT_TIMER_WIDTH_TICKS 65536

bool attached = false;

void attachServo(int pin){
    ledcSetup(MOTORCHANNEL, FREQUENCY, RESOLUTION); // channel #, 50 Hz, timer width
    ledcAttachPin(MOTORPIN, MOTORCHANNEL);   // GPIO pin assigned to channel
    attached = true;
}

void detachServo()
{
    ledcDetachPin(MOTORPIN);
    attached = false;
}

void writeServo(int value)
{
    // treat values less than MIN_PULSE_WIDTH (500) as angles in degrees (valid values in microseconds are handled as microseconds)
    if (value < MINMICRO)
    {
        if (value < 0)
            value = 0;
        else if (value > 180)
            value = 180;

        value = map(value, 0, 180, MINMICRO, MAXMICRO);   //map an angle to microseconds
    }
    writeMicroseconds(value);
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

void motorSetup(){
    writeMicroseconds(OFFPOS);
    attachServo(MOTORPIN);
    writeMicroseconds(OFFPOS);
    // Serial.println("Servo setup");
    // delay(100);
}

