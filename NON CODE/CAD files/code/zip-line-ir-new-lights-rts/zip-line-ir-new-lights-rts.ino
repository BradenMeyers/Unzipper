//FINAL ITERATION with ir sensor

#include <IRremote.h>
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

#define READY 1
#define RECOVERY 3
const int motor = 9;
const int readylight = 13;
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
    while (handlebarbuttonvalue == 1){
      checkbuttons();
    }
    digitalWrite(readylight, LOW);
    digitalWrite(recoverylight, HIGH);
    state = RECOVERY;
    delay(1000);   //helps the button not go wack
  }

//--------------------------------------------------------------------RECOVERY--------------------------------------------------------------------------------------------------------
  else if (state == RECOVERY){
    for (int i = 60; i <= 255; i++){     //this speeds up the motor slowly-------------- if want to start at a higher voltage then set i = larger num
      analogWrite(motor, i);
      delay(15);
      checkbuttons();
      checkir();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){
        break;
      }
    }
    Serial.println("FULL SPEED");
    while (stopbuttonvalue == 1 and handlebarbuttonvalue == 0){         
      checkbuttons();
      checkir();
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
