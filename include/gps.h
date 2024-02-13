#ifndef GPS
#define GPS

#include <Arduino.h>


extern float gpsError;

int satelliteLock();

void resetGPShome();

double generateGPSLock();

void setupGPS();

void loopGPS();

double getDistanceToEnd();


class GPSDataRecorder {
private:
    double startingLat;
    double startingLong;
    
    String storageID;
    String maxSpeedKey;
    String distanceKey;
    String timeKey;

    void storeData();
    
public:
    double lastMaxSpeed;
    double maxSpeed;
    double ATMaxSpeed; //All time max speed get from spiffs
    double lastDistance;
    double distance;
    double ATDistance;  //all time distance 
    unsigned long lastTime;
    unsigned long time;
    unsigned long ATTime;   //all combined time 

    double lastAvg;
    double avg;
    double ATAvg;

    GPSDataRecorder(const String& identifier);

    void recordSpeed();  //should be looping this function as the process runs, 
    void startRecord();  //call on start
    void endRecord();    //call on end
    void resetStats(bool last=true, bool today=true, bool allTime=true);    // call when you want to reset stats

};

extern GPSDataRecorder recoveryData;
extern GPSDataRecorder zippingData;


#endif