#include <Arduino.h>
#include <remote.h>
#include <timer.h>
#include <esp_now.h>
#include <WiFi.h>
#include <serverESP.h>

Timer sendData;
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
} test_struct;

typedef struct send_message {
  int dir;    //1 down  and 0 Up 
  int max;
  int zipDelay;
} send_message;

send_message outgoing;
test_struct incoming;

uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0x37, 0xE2, 0xA0};

String success;
esp_now_peer_info_t peerInfo;
static bool sentFirstMessage = false;
int stall = 1;
int half = 0; //0 is not pressed 1 is pressed.

void updateOutgoing(){
  if(directionStr == "DOWN")
    outgoing.dir = 1;
  else
    outgoing.dir = 0;
  outgoing.max = motorsMaxSpeed;
  outgoing.zipDelay = afterZipDelay;
}

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incoming, incomingData, sizeof(incoming));
  Serial.print("Bytes received: ");
  Serial.println(len);
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
  Serial.println();
  
  motorsMaxSpeed = incoming.max;
  motorMaxSpeedStr = String(motorsMaxSpeed);
  afterZipDelay = incoming.zipDelay;
  afterZipDelayStr = String(afterZipDelay);
  wifiStopMotor = incoming.stop;
  wifiSkipToRecovery = incoming.go;
  stall = incoming.stall;
  if(incoming.dir == 1)
    directionStr = "DOWN";
  else
    directionStr = "UP";
  updateOutgoing();
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
    sentFirstMessage = true;
  }
  else{
    success = "Delivery Fail :(";
  }
}

void compareMessages(){
  if(directionStr == "DOWN" && outgoing.dir == 0 or directionStr == "UP" && outgoing.dir == 1){
    sentFirstMessage = false;
    Serial.print("triggered on direction");
  }
  if(motorsMaxSpeed != outgoing.max){
    Serial.print("triggered on max speed change");
    sentFirstMessage = false;
  }
  if(afterZipDelay != outgoing.zipDelay){
    Serial.print("triggered onzip delay change");
    sentFirstMessage = false;
  }
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
  compareMessages();
  if(sendData.getTime() > 500 && !sentFirstMessage){
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
}
