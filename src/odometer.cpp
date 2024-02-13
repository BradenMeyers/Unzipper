#include <odometer.h>
#include <timer.h>
#include <Arduino.h>
#include <remote.h>
#include <serverESP.h>


#define DELAYODOMETER 5
#define ODOMETERSTALLRATE 500

class HESensor
{
public:
    int pin;
    unsigned long rotations = 0;
    unsigned long beforeRotations;
    unsigned long stallRotationCount;
    Timer stallTimer;
    Timer odometerTimer;

    HESensor(int pin) : pin(pin) {}

    void countRotationsHallEffect(){
        int magneticValue = digitalRead(pin);
        static int lastValue = digitalRead(pin);
        if(odometerTimer.getTime() > DELAYODOMETER){
            if(magneticValue != lastValue){
                rotations++;
                lastValue = magneticValue;
                odometerTimer.start();
                // Serial.println(rotations);
            }
        }
    }

    void logRotations(){
        logger.log("Total Rotations that were zipped: ", true);
        logger.log(rotations - beforeRotations);
    }

    bool isStalledIndiv(){
        // If there have been no rotations in less than 160 milliseconds will return true, otherwise false
        if(stall){  
            if(not stallTimer.started){
                stallTimer.start();
                stallTimer.started = true;
                stallRotationCount = rotations;
                return false;
            }
            else if(stallTimer.getTime() > ODOMETERSTALLRATE){
                stallTimer.started = false;
                //Serial.print("rotations per check:");
                //Serial.println(rotations - stallRotationCount);
                if(rotations - stallRotationCount < 1){
                    return true;
                }
            }
            return false;
        }
        else{
            //logger.log("Stall turned off", true);
            Serial.println("stall off");
            return false;
        }
    }

    unsigned long startIndivCount(){
        beforeRotations = rotations;
        return beforeRotations;
    }

    unsigned long stopIndivCount(){
        if(rotations - beforeRotations < 10){   //This is if someone pulls it at the top and stuff dont actully go down
            //countToTopLimit = rotations + 500;
            return 0;
        }
        //countToTopLimit = (rotations + (rotations - beforeRotations)) - countToTopLimitOffset;
        return rotations - beforeRotations;
    }

    void setupHE(){
        pinMode(pin, INPUT_PULLUP);         //CHECK this
    }
};


#ifdef BACKUP_ENABLE
bool overideBackup = false;
#endif
#ifndef BACKUP_ENABLE
bool overideBackup = true;
#endif

byte odometerHallEffect = 14; 
byte odometerHallBackup = 33;
bool overideMain = false;

// unsigned long countToTopLimit;
// unsigned long countToTopLimitOffset = 20;

//bool atTheTop = false;
//bool someonOnStateChanged = false;
bool shouldCheckAtTheTop = true;

HESensor mainHE(odometerHallEffect);
HESensor backupHE(odometerHallBackup);

unsigned long getCurrentCount(){
    return mainHE.beforeRotations - mainHE.rotations;
}

unsigned long startCount(){
    return mainHE.startIndivCount() + backupHE.startIndivCount();
}

unsigned long stopCount(){          //look here and take out backup if we wont be using.
    unsigned long mainCount = mainHE.stopIndivCount();
    unsigned long backupCount = backupHE.stopIndivCount();
    if((mainCount == 0) and (backupCount == 0)){
        logger.log("both sensors did not read significant rotation", true);
        return 0;
    }
    if(((mainCount - backupCount) > 200) or ((backupCount - mainCount) > 200)){
        logger.log("Significant discrepancy in odometer sensor", true);
        if(mainCount > backupCount){
            logger.log("Main sensor reads zipping rotations: ", true);
            logger.log(mainCount);
            return mainCount;
        }
        else{
            logger.log("Backup sensor reads zipping rotations: ", true);
            logger.log(backupCount);
            return backupCount;
        }
    }
    unsigned long averageRotation = (mainCount + backupCount)/2;
    // logger.log("Count limit: \n", true);
    // logger.log(countToTopLimit);
    logger.log("Rotatons: \n", true);
    logger.log(averageRotation);
    return averageRotation;
}

 bool isStalled(){
    bool mainStall = (mainHE.isStalledIndiv() and !overideMain);
    if(mainStall)
        logger.log("triggered stall on main", true);
    bool backupStall = (backupHE.isStalledIndiv() and !overideBackup);
    if(backupStall)
        logger.log("triggered stall on backup", true);
    return (mainStall or backupStall);              //CHECK HERE if odometer is not working. It might need an and
 }

void setupOdometer(){
    mainHE.setupHE();
    //backupHE.setupHE();
}

void loopOdometer(){
    mainHE.countRotationsHallEffect();

    //backupHE.countRotationsHallEffect();
    //Serial.println("i am here");
}

void disableOdometer(byte HEpin){
    if (HEpin == odometerHallBackup){
        overideBackup = true;
    }
    else if(HEpin == odometerHallEffect){
        overideMain = true;
    }
}