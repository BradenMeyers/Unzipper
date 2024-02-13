#include <Arduino.h>
#include <serverESP.h>
#include <WiFi.h>
#include <ElegantOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <accelerometer.h>
#include <testCases.h>
#include <odometer.h>
#include <gps.h>
#include <motor.h>
#include <main.h>

Blogger logger;

typedef enum{
    ACCELEROMETER,
    ODOMETER, 
    LED,
    MOTOR,
    SERVO
} server_test;
server_test testServer = LED;

//#include <constants.h>
#include <Preferences.h>
Preferences preferences;
#include <timer.h>
Timer batterycheck;

const char* ssid = "Zip-Line-Controls";
const char* password = "123456789";

byte batteryMonitor = 34;
byte odometerSensor = 33;

byte state = READY;
int odometerSensorValue = 0;
int motorsMaxSpeed = 200;
float afterZipDelay = 7;

int highThreshold = 3000;
String highThresholdStr = String(highThreshold);
int lowThreshold = 300;
String lowThresholdStr = String(lowThreshold);

String stateStr = "Ready";
String directionStr = "UP";
String motorMaxSpeedStr = String(map(motorsMaxSpeed, MOTORINITSPEED, 255, 0, 100));
String afterZipDelayStr = String(afterZipDelay);  //cause actual variable is in miliseconds but wifi is in seconds
String uprightErrorStr = String(uprightError);
String gpsErrorStr = String(gpsError);
bool wifiStopMotor = false;
bool wifiSkipToRecovery = false;
bool autonomousMode = true; 

AsyncWebServer server(80);

float batteryVoltage(){
    float reading = analogRead(batteryMonitor);
    reading = ((reading)/(4095)) * 25.6;  //calibrated on May 24 2023
    static float voltage = 19.68;
    float alpha = 0.8;
    voltage = voltage*alpha + (1-alpha)*reading;
    return voltage;
}


//TODO: Really Cut down the time and figure out this processor thing. dont always send strings and
//try out some new things. 
String zipDataProcessor(const String& var){
    if(var == "lastMaxSpeedZip"){return String(zippingData.lastMaxSpeed,1);}
    else if(var == "lastDistanceZip"){return String(zippingData.lastDistance,1);}
    else if(var == "lastAvgZip"){return String(zippingData.lastAvg,1);}
    else if(var == "lastTimeZip"){return String(zippingData.lastTime);}
    else if(var == "maxSpeedZip"){return String(zippingData.maxSpeed,1);}
    else if(var == "distanceZip"){return String(zippingData.distance,1);}
    else if(var == "timeZip"){return String(zippingData.time);}
    else if(var == "avgZip"){return String(zippingData.avg,1);}
    else if(var == "ATMaxSpeedZip"){return String(zippingData.ATMaxSpeed,1);}
    else if(var == "ATdistanceZip"){return String(zippingData.ATDistance,1);}
    else if(var == "ATtimeZip"){return String(zippingData.ATTime);}
    else if(var == "ATavgZip"){return String(zippingData.ATAvg,1);}
    else if(var == "lastMaxSpeedRecov"){return String(recoveryData.lastMaxSpeed,1);}
    else if(var == "lastDistanceRecov"){return String(recoveryData.lastDistance,1);}
    else if(var == "lastAvgRecov"){return String(recoveryData.lastAvg,1);}
    else if(var == "lastTimeRecov"){return String(recoveryData.lastTime);}
    else if(var == "maxSpeedRecov"){return String(recoveryData.maxSpeed,1);}
    else if(var == "distanceRecov"){return String(recoveryData.distance,1);}
    else if(var == "timeRecov"){return String(recoveryData.time);}
    else if(var == "avgRecov"){return String(recoveryData.avg,1);}
    else if(var == "ATMaxSpeedRecov"){return String(recoveryData.ATMaxSpeed,1);}
    else if(var == "ATdistanceRecov"){return String(recoveryData.ATDistance,1);}
    else if(var == "ATtimeRecov"){return String(recoveryData.ATTime);}
    else if(var == "ATavgRecov"){return String(recoveryData.ATAvg,1);}
    return String();
}

String logPageProcessor(const String& var){
    if(var == "AUTOMODE"){return String(autonomousMode);}
    else if(var == "LOGSTRING"){return logger.logStr.c_str();}
    return String();
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //logln(var);
  if (var == "MAXSPEEDVALUE"){return String(map(motorsMaxSpeed, MOTORINITSPEED, 255, 0, 100));}
  else if(var == "STATE"){return stateStr;}
  else if(var == "DIRECTION"){return directionStr;}
  else if(var == "AFTERZIPDELAY"){return afterZipDelayStr;}
  else if(var == "LOWTHRESHOLD"){return lowThresholdStr;}
  else if(var == "HIGHTHRESHOLD"){return highThresholdStr;}
  else if(var == "UPRIGHTERROR"){return String(uprightError);}
  else if(var == "VOLTAGE") return String(batteryVoltage());
  else if(var == "TEMPERATURE") return String(getTemperature());
  else if(var == "TESTLOG"){return testLogger.logStr.c_str();}
  else if(var == "SATLOCK"){return String(satelliteLock());}
  else if(var == "GPSERROR"){return String(gpsError);}
  return String();
}

void storeValues(){
    preferences.putUInt("lower", lowThreshold);
    preferences.putUInt("upper", highThreshold);
    preferences.putUInt("max", motorsMaxSpeed);
    preferences.putFloat("zDelay", afterZipDelay);
    preferences.putFloat("uError", uprightError);
    preferences.putFloat("automode", autonomousMode);
    preferences.putFloat("gpsError", gpsError);

}

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void serverSetup(){
    preferences.begin("my-app", false);
    motorsMaxSpeed = preferences.getUInt("max", 200);     //max speed
    highThreshold = preferences.getUInt("upper", 2500); //Upper IR Threshold
    lowThreshold = preferences.getUInt("lower", 1000); //Lower IR threshold
    afterZipDelay = preferences.getFloat("zDelay", 10);  //Delay after the zipping state
    uprightError = preferences.getFloat("uError", 1.5);  //Delay after the zipping state
    gpsError = preferences.getFloat("gpsError", 3);  //Delay after the zipping state
    autonomousMode = preferences.getBool("automode", true);
    motorMaxSpeedStr = String(map(motorsMaxSpeed, MOTORINITSPEED, 255, 0, 100));
    afterZipDelayStr = String(afterZipDelay);
    lowThresholdStr = String(lowThreshold);
    highThresholdStr = String(highThreshold);
    uprightErrorStr = String(uprightError);
    gpsErrorStr = String(gpsError);
    
    //SETUP WIFI CONFIG
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    //WiFi.begin();
    
    ElegantOTA.begin(&server);    // Start ElegantOTA    server.begin();
    server.begin();
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    initSPIFFS();
    delay(3000);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/maxspeed", HTTP_GET, [] (AsyncWebServerRequest *request){
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) {
        motorMaxSpeedStr = request->getParam("value")->value();
        motorsMaxSpeed = map(motorMaxSpeedStr.toInt(), 0, 100, MOTORINITSPEED, 255);
        logger.log("Max speed in now: ", true);
        logger.log(motorsMaxSpeed);
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/afterzipdelay", HTTP_GET, [] (AsyncWebServerRequest *request){
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) {
        afterZipDelayStr = request->getParam("value")->value();
        afterZipDelay = afterZipDelayStr.toFloat();
        logger.log("After zip delay is now: ", true);
        logger.log(afterZipDelayStr);
        }
        request->send(200, "text/plain", "OK");
    });
    server.on("/uprightError", HTTP_GET, [] (AsyncWebServerRequest *request){
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) {
        uprightErrorStr = request->getParam("value")->value();
        uprightError = uprightErrorStr.toFloat();
        logger.log("Upright Error is now: ", true);
        logger.log(uprightErrorStr);
        }
        request->send(200, "text/plain", "OK");
    });
    server.on("/gpsError", HTTP_GET, [] (AsyncWebServerRequest *request){
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) {
        gpsErrorStr = request->getParam("value")->value();
        gpsError = gpsErrorStr.toFloat();
        logger.log("GPS Error is now: ", true);
        logger.log(gpsErrorStr);
        }
        request->send(200, "text/plain", "OK");
    });
    server.on("/stopclick", HTTP_GET, [](AsyncWebServerRequest *request){
        if(state == RECOVERY){wifiStopMotor = true;}
        Serial.println("stop Button clicked");
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/directionclick", HTTP_GET, [](AsyncWebServerRequest *request){
        if (directionStr == "UP")
          directionStr = "DOWN";
        else   
          directionStr = "UP";
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/goupclick", HTTP_GET, [](AsyncWebServerRequest *request){
        if(state == READY){wifiSkipToRecovery = true;}
        Serial.println("go Button clicked");
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/odometersettingsclick", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/odometer.html", String(), false, processor);
    });

    server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request){
        if(state == TEST){
            state = READY;
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    
    });

    server.on("/zipStats", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/zipStats.html", String(), false, zipDataProcessor);
    });

    server.on("/logclick", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/logger.html", String(), false, logPageProcessor);
    });

    server.on("/autoOFF", HTTP_GET, [](AsyncWebServerRequest *request){
        changeAutonomy(false);
        autonomousMode = false;
        logger.log("Autonomous Mode Off", true);
        request->send(SPIFFS, "/logger.html", String(), false, logPageProcessor);
    });
    server.on("/autoON", HTTP_GET, [](AsyncWebServerRequest *request){
        changeAutonomy(true);
        autonomousMode = true;
        logger.log("Autonomous Mode On", true);
        request->send(SPIFFS, "/logger.html", String(), false, logPageProcessor);
    });
    server.on("/clearlog", HTTP_GET, [](AsyncWebServerRequest *request){
        logger.clearLog();
        Serial.println("CLEARING LOG----");
        request->send(SPIFFS, "/logger.html", String(), false, logPageProcessor);
    });

    server.on("/disableMainHE", HTTP_GET, [](AsyncWebServerRequest *request){
        disableOdometer(odometerHallEffect);
        logger.log("Disabled main HE sensor", true);
        request->send(SPIFFS, "/odometer.html", String(), false, processor);
    });
    server.on("/disablebackupHE", HTTP_GET, [](AsyncWebServerRequest *request){
        disableOdometer(odometerHallBackup);
        logger.log("Disabled backup HE sensor", true);
        request->send(SPIFFS, "/odometer.html", String(), false, processor);
    });
    server.on("/resetStable", HTTP_GET, [](AsyncWebServerRequest *request){
        setStable();
        logger.log("Stable reset", true);
        request->send(SPIFFS, "/odometer.html", String(), false, processor);
    });
    server.on("/resetStats", HTTP_GET, [](AsyncWebServerRequest *request){
        zippingData.resetStats();
        recoveryData.resetStats();
        request->send(SPIFFS, "/odometer.html", String(), false, processor);
    });
    server.on("/resetGPS", HTTP_GET, [](AsyncWebServerRequest *request){
        resetGPShome();
        request->send(SPIFFS, "/odometer.html", String(), false, processor);
    });
    server.on("/testclick", HTTP_GET, [](AsyncWebServerRequest *request){
        state = TEST;
        request->send(SPIFFS, "/test.html", String(), false, processor);
    });
    server.on("/motortest", HTTP_GET, [](AsyncWebServerRequest *request){
        web_select(MOTOR);
        request->send(SPIFFS, "/test.html", String(), false, processor);
    });
    server.on("/acceltest", HTTP_GET, [](AsyncWebServerRequest *request){
        web_select(ACCELEROMETER);
        request->send(SPIFFS, "/test.html", String(), false, processor);
    });
    server.on("/odomtest", HTTP_GET, [](AsyncWebServerRequest *request){
        web_select(ODOMETER);
        request->send(SPIFFS, "/test.html", String(), false, processor);
    });
    server.on("/servotest", HTTP_GET, [](AsyncWebServerRequest *request){
        web_select(SERVO);
        request->send(SPIFFS, "/test.html", String(), false, processor);
    });
    pinMode(batteryMonitor, INPUT);
    batterycheck.start();
}

void serverLoop(){
    if(batterycheck.getTime() > 5000){
        float volts = batteryVoltage();
        batterycheck.start();
        storeValues();
    }
    logger.checkLogLength();
}