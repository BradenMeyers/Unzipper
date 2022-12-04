const int escPin = 9;
const int buttonPin = 2;
const int ledPin = 13;
int escPwmvalue = 0;
int buttonValue = 0;


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode (escPin, OUTPUT);
pinMode (buttonPin, INPUT_PULLUP);
pinMode (ledPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

buttonValue = digitalRead(buttonPin);
Serial.println(buttonValue);
if (buttonValue == HIGH){
  digitalWrite(escPin, LOW);
  digitalWrite(13,LOW);
}
else {
  
  digitalWrite(13,HIGH);
  analogWrite(escPin,escPwmvalue);
}
}
