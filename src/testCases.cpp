#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <timer.h>
#include <motor.h>
#include <testCases.h>
#include <serverESP.h>
#include <accelerometer.h>
#include <main.h>
Blogger testLogger;
Timer accelerometerTest;
Timer dataOverload;

void accelerometerLoop(){
    if(accelerometerTest.getTime() < 20000){
        testLogger.log("Check Stable Every Second: ", true);
        checkStable(1000);
        delay(2000);
    }
    else if(accelerometerTest.getTime() < 22000){
        testLogger.log("Going to reset Stable", true);
        setStable();
        delay(500);
    }
    else
        accelerometerTest.start();    
}

//ODOMETER TESTS
int hallEffectPin1 = 33;  //14 and 33 on MAR17
int hallEffectPin2 = 14;


void odometerSetup(){
    pinMode(hallEffectPin1, INPUT_PULLUP);
    pinMode(hallEffectPin2, INPUT_PULLUP);
}

void countRotationsHallEffecttest1(){
  static int magneticValue = digitalRead(hallEffectPin1);
  static int rotationstest = 0;
  if(magneticValue != digitalRead(hallEffectPin1)){
    rotationstest++;
    testLogger.log("Odometer 1 rotations: ", true);
    testLogger.log(rotationstest);
    magneticValue = !magneticValue;
  }
}

void countRotationsHallEffecttest2(){
  static int magneticValue = digitalRead(hallEffectPin2);
  static int rotationstest = 0;
  if(magneticValue != digitalRead(hallEffectPin2)){
    rotationstest++;
    testLogger.log("Odometer 2 rotations: ", true);
    testLogger.log(rotationstest);
    magneticValue = !magneticValue;
  }
}

void odometerLoop(){
  countRotationsHallEffecttest1();
  countRotationsHallEffecttest2();
  //Serial.println(digitalRead(hallEffectPin1));
  //Serial.println(digitalRead(hallEffectPin2));
  //delay(1000);
}

//SERVO TESTS

// void servoLoop(){
//     static int pos = 90;
//     if(pos <= 180){
//         writeServo(pos);
//         delay(SERVODELAY/2);
//         testLogger.log("Moving servo forward", true);
//         pos +=30;
//     }
//     else{
//         pos = 0;
//         testLogger.log("Moving servo backward", true);
//         writeServo(pos);
//         delay(SERVODELAY*3);
//     }
// }

//MOTOR TESTS
int motorChannelTest = 1;
String directionStrTest = "UP";
byte motorDirectionTest = 12;
byte motorTest = 13;

void turnOnMotorTest(int speed){
  if(directionStrTest == "DOWN")
    digitalWrite(motorDirectionTest, HIGH);
  else
    digitalWrite(motorDirectionTest, LOW);
    writeMicroseconds(speed);
  /* if(batteryVoltage()< 16.50  && speed != 0){ 
    speed = speed + 10;
    mainlog.log("motor speed increase due to low battery voltage", true);
  } */
  if(speed == OFFPOS){
    //mainlog.log("MOTOR IS OFF------------------", true);
  }
}

void motorLoop(){
    delay(1000);
    directionStrTest = "UP";
    testLogger.log("Spin motor Forward", true);
    for (int i = OFFPOS; i < OFFPOS+200; i++)
    {
        turnOnMotorTest(i);
        delay(10);
    }
    delay(1000);
    delay(1000);
    turnOnMotor(OFFPOS);
}

typedef enum{
    ACCELEROMETER,
    ODOMETER, 
    LED,
    MOTOR
} sm_test;
sm_test testState = LED;


void web_select(int testSel){
    if(testSel != testState){
        if(testSel == MOTOR){
            testState = MOTOR;
            testLogger.log("Motor test will spin the motor forward then backward", true);
        }
        else if(testSel == ACCELEROMETER){
            testLogger.log("Accelerometer test will check the stability every 0.5 seconds", true);
            testState = ACCELEROMETER;
        }
        else if(testSel == ODOMETER){
            testState = ODOMETER; 
            testLogger.log("odometers will count the rotations",true);
        }
        testLogger.log("Selected test: ", true);
        testLogger.log(testState);  
    } 
}

void testSelect(){
    testLogger.checkLogLength();
    while(state == TEST){
        if(testState != MOTOR){
            turnOnMotor(OFFPOS);
        }
        switch (testState)
        {
        case MOTOR:
            motorLoop();
            break;
        case ACCELEROMETER:
            accelerometerLoop();
            break;
        case ODOMETER:
            static bool initodometer = false;
            if(initodometer){
                odometerSetup;
                initodometer = true;
                testLogger.log("Odometer Setup", true);
            }
            odometerLoop();
            break;
        default:
            if(dataOverload.getTime() > 10000){
                testLogger.log("select a test", true);
                dataOverload.start();
            }
            break;
        }
    }
}