
#include <Arduino.h>
#include <servoBrake.h>

#define MINMICRO 500
#define MAXMICRO 2500
#define NUETRALMICRO 1500       //PWM specs found on datasheet from part and on Amazon

#define SERVOPIN 19          //GPIO pin 4
#define SERVOCHANNEL 2
#define FREQUENCY 50    //AMAZON Specs for servo said frequency could be between 50-330hz
#define RESOLUTION 16   //found this on esp32 servo library as DEFAULT_TIMER_WIDTH

#define REFRESH_USEC 20000 //conversion for microseconds to ticks
#define DEFAULT_TIMER_WIDTH_TICKS 65536

bool attached = false;

void attachServo(int pin){
    ledcSetup(SERVOCHANNEL, FREQUENCY, RESOLUTION); // channel #, 50 Hz, timer width
    ledcAttachPin(SERVOPIN, SERVOCHANNEL);   // GPIO pin assigned to channel
    attached = true;
}

void detachServo()
{
    ledcDetachPin(SERVOPIN);
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
        ledcWrite(SERVOCHANNEL, value);
    }
}



int ticksToUs(int ticks)
{
    return (int)((float)ticks * ((float)REFRESH_USEC / (float)DEFAULT_TIMER_WIDTH_TICKS)); 
}

void servoSetup(){
    writeServo(OFFPOS);
    attachServo(SERVOPIN);
    writeServo(OFFPOS);
    Serial.println("Servo setup");
    delay(100);
}
