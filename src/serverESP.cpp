#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <serverESP.h>
#include <constants.h>

const char* ssid = "Zip-Line-Controls";
const char* password = "123456789";

byte state = READY;

byte odometerSensor = 33;
int odometerSensorValue = 0;
byte motorsMaxSpeed = 200;
unsigned long afterZipDelay = 7000;

int highThreshold = 3000;
String highThresholdStr = String(highThreshold);
int lowThreshold = 300;
String lowThresholdStr = String(lowThreshold);

String stateStr = "Ready";
String directionStr = "UP";
String motorMaxSpeedStr = String(motorsMaxSpeed);
String afterZipDelayStr = String(afterZipDelay * 0.001);  //cause actual variable is in miliseconds but wifi is in seconds

bool wifiStopMotor = false;
bool wifiSkipToRecovery = false;

Logger logger;

AsyncWebServer server(80);

byte batteryMonitor = 34;
float batteryVoltage(){
    float reading = analogRead(batteryMonitor);
    reading = ((reading)/(4095)) * 19.68;
    static float voltage = 19.68;
    float alpha = 0.8;
    voltage = voltage*alpha + (1-alpha)*reading;
    return voltage;
}



// Replaces placeholder with button section in your web page
String processor(const String& var){
  //logln(var);
  if (var == "MAXSPEEDVALUE"){return String(motorsMaxSpeed);}
  else if(var == "STATE"){return stateStr;}
  else if(var == "DIRECTION"){return directionStr;}
  else if(var == "AFTERZIPDELAY"){return afterZipDelayStr;}
  else if(var == "CURRENTIRVALUE"){return String(analogRead(odometerSensor));}
  else if(var == "LOWTHRESHOLD"){return lowThresholdStr;}
  else if(var == "HIGHTHRESHOLD"){return highThresholdStr;}
  else if(var == "VOLTAGE") return String(batteryVoltage());
  //else if(var == "LOGSTRING"){return logger.logStr;}
  return String();
}


void serverSetup(){
    WiFi.softAP(ssid, password);
    server.begin();
    delay(3000);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/maxspeed", HTTP_GET, [] (AsyncWebServerRequest *request){
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) {
        motorMaxSpeedStr = request->getParam("value")->value();
        motorsMaxSpeed = motorMaxSpeedStr.toInt();
        logger.log("Max speed in now: ");
        logger.log(motorMaxSpeedStr, true);
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/afterzipdelay", HTTP_GET, [] (AsyncWebServerRequest *request){
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam("value")) {
        afterZipDelayStr = request->getParam("value")->value();
        afterZipDelay = afterZipDelayStr.toInt() * 1000;
        logger.log("After zip delay is now: ");
        logger.log(afterZipDelayStr, true);
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/stopclick", HTTP_GET, [](AsyncWebServerRequest *request){
        if(state == RECOVERY){wifiStopMotor = true;}
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/directionclick", HTTP_GET, [](AsyncWebServerRequest *request){
        if (directionStr == "UP")
          directionStr = "DOWN";
        else   
          directionStr = "UP";
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/goupclick", HTTP_GET, [](AsyncWebServerRequest *request){
        if(state == READY){wifiSkipToRecovery = true;}
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/odometersettingsclick", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", odometer_index_html, processor);
    });

    server.on("/back", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/logclick", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", log_page_index_html, processor);
    });

    server.on("/clearlog", HTTP_GET, [](AsyncWebServerRequest *request){
        //logger.logStr = "";
        Serial.println("CLEARING LOG----");
        request->send_P(200, "text/html", log_page_index_html, processor);
    });

    server.on("/lowthreshold", HTTP_GET, [] (AsyncWebServerRequest *request){
        if (request->hasParam("value")) {
        lowThresholdStr = request->getParam("value")->value();
        lowThreshold = lowThresholdStr.toInt();
        logger.log("low threshold is now: ");
        logger.log(lowThresholdStr, true);
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/highthreshold", HTTP_GET, [] (AsyncWebServerRequest *request){
        if (request->hasParam("value")) {
        highThresholdStr = request->getParam("value")->value();
        highThreshold = highThresholdStr.toInt();
        logger.log("high threshold is now: ");
        logger.log(highThresholdStr, true);
        }
        request->send(200, "text/plain", "OK");
    });
    pinMode(batteryMonitor, INPUT);
}

void serverLoop(){

}