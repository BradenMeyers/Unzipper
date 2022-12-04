//FINAL ITERATION with ir sensor

#include <IRremote.h>
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

#define READY 1
#define ZIPPING 2
#define RECOVERY 3
const int motor = 9;
const int readylight = 13;
const int ziplight = 12;
const int recoverylight = 11;
const int handlebarbutton = 3;
const int stopbutton = 8;
int stopbuttonvalue;
int handlebarbuttonvalue;
int state = READY;
unsigned long currenttime;
unsigned long aftertime;
unsigned long clocktime;
long finezip = 2000;

void starttime(){
  currenttime = millis();
}

void gettime(){
  aftertime = millis();
  clocktime = aftertime - currenttime;
}

void checkbuttons(){
  handlebarbuttonvalue = digitalRead(handlebarbutton);
  stopbuttonvalue = digitalRead(stopbutton);
}

void checkir(){
  if (irrecv.decode(&results)){
      if (results.value == 3269679876){
        stopbuttonvalue = 0;
        Serial.print("remote stop  ");
      }
      else if (results.value == 3421054024){
        currenttime = clocktime;
        Serial.print("remote half speed  ");
      }
      Serial.print("remote value: "), Serial.println(results.value);
      irrecv.resume();
 }
}

void setup(){
  pinMode(motor, OUTPUT);
  pinMode(handlebarbutton, INPUT_PULLUP);
  pinMode(stopbutton, INPUT_PULLUP);
  pinMode(readylight, OUTPUT);
  pinMode(ziplight, OUTPUT);
  pinMode(recoverylight, OUTPUT);
  irrecv.enableIRIn();
  Serial.begin(9600);
}

void loop(){
//------------------------------------------------------------------READY------------------------------------------------------------------------------------------------------------
  Serial.print("state = "), Serial.print(state);
  Serial.print(" finezip is: "), Serial.println(finezip);
  if (state == READY){
    checkbuttons();
    digitalWrite(motor, LOW);
    digitalWrite(readylight, HIGH);
    while (handlebarbuttonvalue == 0){
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    starttime();
    state = ZIPPING;
    digitalWrite(readylight, LOW);
    delay(100);   //helps the button not go wack
  }



//-------------------------------------------------------------------ZIPPING-----------------------------------------------------------------------------------------------------------------------
  else if (state == ZIPPING){
    digitalWrite(ziplight, HIGH);
    while (handlebarbuttonvalue == 1){
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    gettime();
    state = RECOVERY;
    Serial.print("time spent zipping: "), Serial.println(clocktime);
    digitalWrite(ziplight, LOW);
    digitalWrite(recoverylight, HIGH);
    aftertime = millis();
    aftertime = aftertime + 2000;
    while (handlebarbuttonvalue == 0 and currenttime < aftertime){
      starttime();
      checkbuttons();
   }
   if (handlebarbuttonvalue == 1){
    digitalWrite(recoverylight, LOW);
    while (handlebarbuttonvalue == 1){
      checkbuttons();
    }
    state = READY;
    delay(300);
    return;
   }
    //delay(2000);       //wait for things to settle down before we go up the zip line
  }



//--------------------------------------------------------------------RECOVERY--------------------------------------------------------------------------------------------------------
  else if (state == RECOVERY){         //Theres 3 stages to recovery, speed up the motor, go at full speed for a time (depending on how long they zipped), and finally go to half power untill its stopped.
    for (int i = 60; i <= 255; i++){     //this speeds up the motor slowly-------------- if want to start at a higher voltage then set i = larger num
      analogWrite(motor, i);
      delay(15);
      checkbuttons();
      checkir();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){
        break;
      }
    }
    
    starttime();
    clocktime = currenttime + clocktime + finezip;             //finezip will determine how long it goes at full speed and then switch to half speed
    currenttime = clocktime - currenttime;
    Serial.print("recovery time in normal: "), Serial.println(currenttime);
    Serial.print("recovery time in millis(): "), Serial.println(clocktime);
    Serial.println("FULL SPEED");
    
    while (stopbuttonvalue == 1 and handlebarbuttonvalue == 0 and currenttime < clocktime){         
      checkbuttons();
      starttime();
      checkir();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){
        finezip = finezip - 500;
      }
    }
    
    analogWrite(motor, 200);      //put motor at half speed-----------------------------
    Serial.println("HALF SPEED");
    starttime();
    
    while(stopbuttonvalue == 1 and handlebarbuttonvalue == 0){
      checkbuttons();
      checkir();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){
        gettime();
        Serial.print("half speed time: "), Serial.println(clocktime);
        if (clocktime > 1500){     //this is stating that if half speed has been going for to long then cut some time so it gets to half speed quicker
          finezip = finezip + 500;
        }
      }
    }
    
    digitalWrite(motor, LOW);             //stop motor
    digitalWrite(recoverylight, LOW);
    Serial.println("motor is off");
    delay(100);
    checkbuttons();
    while (handlebarbuttonvalue == 1){
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    delay(300);
    state = READY;
  }
 }
