#include <Arduino.h>
#include <remote.h>
#include <timer.h>
#include <esp_now.h>
#include <WiFi.h>
#include <serverESP.h>
#include <testCases.h>
#include <main.h>
Timer sendData;
Timer dataRecTimer;
Timer racecarTimer;
//Structure example to receive data
//Must match the sender structure
typedef struct test_struct {
  int dir;    //1 down  and 0 Up 
  int max;
  int zipDelay;
  int stall;
  int stop;
  int go;
  int half;
  int beep;
} test_struct;
test_struct incoming;

typedef struct send_message {
  int dir;    //1 down  and 0 Up 
  int max;
  int zipDelay;
} send_message;
send_message outgoing;

typedef struct single_int{
  int connect;
} single_int;
single_int connect_request;

typedef struct racecarStruct{
  int dir;
  int speed;
} racecarStruct;
racecarStruct racecarData;

uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0x37, 0xE2, 0xA0}; //7C:9E:BD:37:E2:A0

String success;
esp_now_peer_info_t peerInfo;
static bool sentFirstMessage = false;
static bool sendRequest = false;
int failedAttempts = 0;
int stall = 1;
int half = 0; //0 is not pressed 1 is pressed.
int beep = 0; // 0 turn off beeper, 1 turn on beeper
bool racecarMode = false;
int racecarSpeed = 1500;

void updateOutgoing(){
  if(directionStr == "DOWN"){
    outgoing.dir = 1;
  }
  else{
    outgoing.dir = 0;
  }
  outgoing.max = motorsMaxSpeed;
  outgoing.zipDelay = afterZipDelay;
}

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  dataRecTimer.start();
  Serial.print("Bytes received: ");
  Serial.println(len);
  if(len < 6){
    memcpy(&connect_request, incomingData, sizeof(connect_request));
    sentFirstMessage = false;
    sendRequest = true;
    failedAttempts = 0;
  }
  if(len > 6 && len < 20){
    Serial.println("i am here 1");
    memcpy(&racecarData, incomingData, sizeof(racecarData));
    racecarMode = true;
    racecarTimer.start();
    if(state == READY){
      Serial.println("I am here");
      racecarSpeed = racecarData.speed;
      Serial.println(racecarSpeed);
      if(racecarData.dir == 1)
        directionStr = "DOWN";
      else
        directionStr = "UP";
      Serial.println(directionStr);
    }
  }
  else{
    racecarMode = false;    //when the remote leaves racecar mode data will be sent that is not 8 bytes
    turnOnBrake();
  }
  if(len > 25 ){
    memcpy(&incoming, incomingData, sizeof(incoming));
    Serial.print("Direction: ");
    Serial.println(incoming.dir);
    Serial.print("Delay: ");
    Serial.println(incoming.zipDelay);
    Serial.print("Max Speed: ");
    Serial.println(incoming.max);
    Serial.print("Stall: ");
    Serial.println(incoming.stall);
    Serial.print("Stop: ");
    Serial.println(incoming.stop);
    Serial.print("go: ");
    Serial.println(incoming.go);
    Serial.print("Half: ");
    Serial.println(incoming.half);
    Serial.print("Beep: ");
    Serial.println(incoming.beep);
    Serial.println();
    motorsMaxSpeed = incoming.max;
    motorMaxSpeedStr = String(motorsMaxSpeed);
    afterZipDelay = incoming.zipDelay;
    afterZipDelayStr = String(afterZipDelay);
    wifiStopMotor = incoming.stop;
    wifiSkipToRecovery = incoming.go;
    half = incoming.half;
    beep = incoming.beep;
    stall = incoming.stall;
    if(incoming.dir == 1)
      directionStr = "DOWN";
    else
      directionStr = "UP";
    updateOutgoing();
  }
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
    sentFirstMessage = true;
    sendRequest = false;
    failedAttempts = 0;
  }
  else{
    success = "Delivery Fail :(";
    failedAttempts++;
  }
}

void compareMessages(){
  if(directionStr == "DOWN" && outgoing.dir == 0 or directionStr == "UP" && outgoing.dir == 1){
    sentFirstMessage = false;
    sendRequest = true;
    failedAttempts = 0;
    Serial.println("triggered on direction"); 
  }
  if(motorsMaxSpeed != outgoing.max){
    Serial.println("triggered on max speed change");
    sentFirstMessage = false;
    sendRequest = true;
    failedAttempts = 0;
  }
  if(afterZipDelay != outgoing.zipDelay){
    Serial.println("triggered onzip delay change");
    sentFirstMessage = false;
    sendRequest = true;    
    failedAttempts = 0;
  }                     // not sure if the outgoing change could happen in between check and the update. 
  updateOutgoing();  //Needs to update the outgoing packet so it wont trigger again.
}
 
void remoteSetup() {
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  sendData.start();
}
 
void remoteLoop() {
  if(dataRecTimer.getTime() > 500)    //dont check if the messages are different until data is recieved, it runs the compare too fast
    compareMessages();
  if((sendData.getTime() > 250) && (sendRequest) && (failedAttempts < 10)){
    updateOutgoing();
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoing, sizeof(outgoing));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    sendData.start();
  }

  if(racecarTimer.getTime() > 500){
    racecarMode = false;
  }
}
