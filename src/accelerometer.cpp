#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <timer.h>
#include <accelerometer.h>

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

Timer gyro;
Timer accel;
Timer tempTimer;
Timer stableTimer;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

//Upright Position
float uprightX = -9.8;
float uprightY = 0;
float uprightZ = 0; 

float uprightError = 1.5;

//Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

unsigned long gyroDelay = 10;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 100;

// Init MPU6050
void initMPU(){
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

void getGyroReadings(){
  mpu.getEvent(&a, &g, &temp);

  float gyroX_temp = g.gyro.x;
  if(abs(gyroX_temp) > gyroXerror)  {
    gyroX += gyroX_temp/50.00;
  }
  
  float gyroY_temp = g.gyro.y;
  if(abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp/70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if(abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp/90.00;
  }
}

void getAccReadings() {
  mpu.getEvent(&a, &g, &temp);
  // Get current acceleration values
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
}

float getTemperature(){
  mpu.getEvent(&a, &g, &temp);
  return temp.temperature;
}

void resetGyro(){
    gyroX=0;
    gyroY=0;
    gyroZ=0;
}

void read_temp(bool print=false){
    if (tempTimer.getTime() > temperatureDelay) {
    // Send Events to the Web Server with the Sensor Readings
        getTemperature();
        tempTimer.start();
        if(print){
            Serial.print("Temperature: ");
            Serial.println(temperature);
        }
    }
}

void read_gyro(bool print=false){
    if (gyro.getTime() > gyroDelay) {
    // Send Events to the Web Server with the Sensor Readings
        getGyroReadings();
        gyro.start();
        if(print){
            Serial.print("Gyro x: ");
            Serial.println(gyroX);
            Serial.print("Gyro y: ");
            Serial.println(gyroY);
            Serial.print("Gyro Z: ");
            Serial.println(gyroZ);
        }
    }
}

void read_acc(bool print=false){
    if (accel.getTime() > accelerometerDelay) {
    // Send Events to the Web Server with the Sensor Readings
        getAccReadings();
        accel.start();
        if(print){
            Serial.print("Accel x: ");
            Serial.println(accX);
            Serial.print("Accel y: ");
            Serial.println(accY);
            Serial.print("Accel Z: ");
            Serial.println(accZ);
        }
    }
}

void setStable(){
    float count = 25;
    uprightX = 0;
    uprightY = 0;
    uprightZ = 0;
    for (int i=0; i< count; i++){
        getAccReadings();
        uprightX += accX;
        uprightY += accY;
        uprightZ += accZ;
    }
    uprightX = uprightX/count;
    uprightY = uprightY/count;
    uprightZ = uprightZ/count;

}

bool checkInstantStable(){
    static bool stableX = false;
    static bool stableY = false;
    static bool stableZ = false;
    getAccReadings();
    if(abs(accX - uprightX) < uprightError)
        stableX = true;
    else
        stableX = false;
    if(abs(accY - uprightY) < uprightError)
        stableY = true;
    else
        stableY = false;
    if(abs(accZ - uprightZ) < uprightError)
        stableZ = true;
    else
        stableZ = false;

    return stableX and stableY and stableZ;
}

bool checkStable(int timeStable){
    static bool init = false;
    if (!init){
        stableTimer.start();
        init = true;
    }
    if(checkInstantStable()){
        if(stableTimer.getTime() > timeStable){
            init = false;
            Serial.println("the zipline is stable");
            return true;
        }
        else
            return false;
    }
    else{
        stableTimer.start();
        Serial.println("the zipline is not stable");
        return false;
    }
}

void setupAccel(){
    initMPU();
}

void looop(){
    // Serial.println(checkStable());
    // delay(400);
}