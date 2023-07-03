#include <gps.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>


static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setupGPS(){
  Serial.begin(115200);
  ss.begin(GPSBaud);
}

void loopGPS(){
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.println(gps.location.lng(), 6);
      // Number of satellites in use (u32)
      Serial.print("Number of satellites in use = "); 
      Serial.println(gps.satellites.value());

    }
  }
}

