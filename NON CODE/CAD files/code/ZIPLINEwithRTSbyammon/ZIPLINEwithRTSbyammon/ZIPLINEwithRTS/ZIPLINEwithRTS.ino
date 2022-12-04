//yayy
//this script could possibly be for the return to start function so the motor only turns on when that button is pressed. I dont know if this will work but I tried.
#define READY 1
#define ZIPPING 2              //don't know what this is for but seems important
#define RECOVERY 3
int motor = 9;
int handlebarbutton = 3;
int stopbutton = 10;        //changed the stop button to pin 10 instead of 11    
int stopbuttonvalue; 
int handlebarbuttonvalue;
int state = READY;
int readylight = 13;
int ziplight = 12;        // ziplight to 11
int recoverylight = 11;    //recoveryled to 12
unsigned long currenttime;
unsigned long aftertime;
unsigned long clocktime;
long finezip = 3000;

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

void setup(){
  pinMode(motor, OUTPUT);
  pinMode(handlebarbutton, INPUT_PULLUP);
  pinMode(stopbutton, INPUT_PULLUP);
  pinMode(readylight, OUTPUT);
  pinMode(ziplight, OUTPUT);    //set the leds to output
  pinMode(recoverylight, OUTPUT); 
  Serial.begin(9600);
}


void loop(){
//------------------------------------------------------------------READY------------------------------------------------------------------------------------------------------------
  Serial.print("state = "), Serial.println(state);
  Serial.print("finezip is: "), Serial.println(finezip);
  if (state == READY){
    checkbuttons();
    digitalWrite(motor, LOW);
    digitalWrite(readylight, HIGH);
    while (handlebarbuttonvalue == 0){    // changed HBBV from 1 to 0
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    starttime();
    state = ZIPPING;
    digitalWrite(readylight, LOW);
    digitalWrite(ziplight, HIGH);           //turn on the zipping light
    delay(100);   //helps the button not go wack
  }



//-------------------------------------------------------------------ZIPPING-----------------------------------------------------------------------------------------------------------------------
  else if (state == ZIPPING){
    while (handlebarbuttonvalue == 1){        //changed HBBV from 0 to 1
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    gettime();
    Serial.print("time spent zipping: "), Serial.println(clocktime);

    while (handlebarbuttonvalue == 0){
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    while(handlebarbuttonvalue == 1){
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    delay(1000);
    state = RECOVERY;
  }
  
//--------------------------------------------------------------------RECOVERY--------------------------------------------------------------------------------------------------------
  else if (state == RECOVERY){         //Theres 3 stages to recovery, speed up the motor, go at full speed for a time (depending on how long they zipped), and finally go to half power untill its stopped.
    for (int i = 55; i <= 255; i++){     //this speeds up the motor slowly-------------- if want to start at a higher voltage then set i = larger num
      analogWrite(motor, i);
      delay(15);
      checkbuttons();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){     //changed HBBV from 0 to 1
        break;
      }
    }
    
    starttime();
    clocktime = currenttime + clocktime + finezip;             //finezip will determine how long it goes at full speed and then switch to half speed
    currenttime = clocktime - currenttime;
    Serial.print("recovery time in normal: "), Serial.println(currenttime);
    Serial.print("recovery time in millis(): "), Serial.println(clocktime);
    Serial.println("full speed");
    
    while (stopbuttonvalue == 1 and handlebarbuttonvalue == 0 and currenttime < clocktime){     //changed HBBV from 1 to 0    
      checkbuttons();
      starttime();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){     //changed HBBV from 0 to 1
        finezip = finezip - 500;
      }
    }
    
    analogWrite(motor, 155);      //put motor at half speed-----------------------------
    Serial.println("half speed");
    starttime();
    
    while(stopbuttonvalue == 1 and handlebarbuttonvalue == 0){    //changed HBBV from 1 to 0
      checkbuttons();
      if (stopbuttonvalue == 0 or handlebarbuttonvalue == 1){     //changed HBBV from 0 to 1
        gettime();
        Serial.print("half speed time: "), Serial.println(clocktime);
        if (clocktime > 4000){     //this is stating that if half speed has been going for to long then cut some time so it gets to half speed quicker
          finezip = finezip + 500;
        }
      }
    }
    
    digitalWrite(motor, LOW);             //stop motor
    Serial.println("motor is off");
    digitalWrite(recoverylight, LOW);      //turn the recoverylight off
    while (handlebarbuttonvalue == 1){
      handlebarbuttonvalue = digitalRead(handlebarbutton);
    }
    delay(300);
    state = READY;    
 }
}
