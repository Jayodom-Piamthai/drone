//joystick setup
#define VRX_PIN 32  // Arduino pin connected to VRX pin
#define VRY_PIN 33  // Arduino pin connected to VRY pin
#define VRX2_PIN 34
#define VRY2_PIN 35
int xValue = 0;  // To store value of the X axis
int yValue = 0;  // To store value of the Y axis
int xValue2 = 0;
int yValue2 = 0;
//LoRa setup
#include <SPI.h>
#include <LoRa.h>
//this pin for esp32 wroom
#define ss 5
#define rst 14
#define dio0 2
// LoRa.setPins(ss, rst, dio0);
int count = 0;
int interval = 0;
int kickback;
int kickround;

//drone data
int payload;
int humid;
int temp;
int mpux;
int mpuy;
int mpuz;

void setup() {
  Serial.begin(38400);
  Serial.println("drone begin");
  millis();
  //LoRa
  LoRa.setPins(ss, rst, dio0);
  while (!Serial)
    ;
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Receiver Started");
  interval = millis() + 10000;
}

void loop() {

  // put your main code here, to run repeatedly:
  //joystick input
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
  xValue2 = analogRead(VRX2_PIN);
  yValue2 = analogRead(VRY2_PIN);
  // print data to Serial Monitor on Arduino IDE
  // Serial.print("x = ");
  // Serial.print(xValue);
  // Serial.print(", y = ");
  // Serial.print(yValue);
  // Serial.print(", x2 = ");
  // Serial.print(xValue2);
  // Serial.print(", y2 = ");
  // Serial.println(yValue2);

  if (millis() < interval && !kickback) {
    sendData();
  } else if (kickback) {

    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      Serial.println("package recieved");
      count += 1;
      kickback = 0;
      interval = millis() + 5000;
      String rawInput;
      while (LoRa.available()) {
        rawInput += ((char)LoRa.read());  //recieve message as ascii char and put into string
      }
      Serial.println(rawInput);
      temp = rawInput.substring(0, rawInput.indexOf(",")).toInt();
      humid = rawInput.substring(rawInput.indexOf(",") + 1, rawInput.indexOf("<")).toInt();
      mpux = rawInput.substring(rawInput.indexOf("<") + 1, rawInput.indexOf(".")).toInt();
      mpuy = rawInput.substring(rawInput.indexOf(".") + 1, rawInput.indexOf(">")).toInt();
      mpuz = rawInput.substring(rawInput.indexOf("/") + 1, -1).toInt();
      Serial.print("temp = ");
      Serial.print(temp);
      Serial.print(", humid = ");
      Serial.print(humid);
      Serial.print(", mpux = ");
      Serial.print(mpux);
      Serial.print(", mpuy = ");
      Serial.print(mpuy);
      Serial.print(", mpuz = ");
      Serial.print(mpuz);
    }
  } else {
    kickback = 1;
    kickround = 0;
    Serial.println("kicking");
    while (kickround < 5) {
      sendData();
      kickround += 1;
    }
  }
  // if(kickback == 1){
  //   LoRa.receive();
  // }
  // else if((millis() > interval)){
  //   kickback = 1;
  //   sendData();
  //   LoRa.receive();
  // }
  // else{
  // LoRa.receive();
  // sendData();
  // }
  Serial.print("temp = ");
  Serial.print(temp);
  Serial.print(", humid = ");
  Serial.print(humid);
  Serial.print(", mpux = ");
  Serial.print(mpux);
  Serial.print(", mpuy = ");
  Serial.print(mpuy);
  Serial.print(", mpuz = ");
  Serial.println(mpuz);
}

//LoRa send and recieves
void sendData() {
  Serial.print(count);
  Serial.print(kickback);
  Serial.println("package sended");
  LoRa.beginPacket();
  LoRa.print(xValue);
  LoRa.print(",");
  LoRa.print(yValue);
  LoRa.print("<");
  LoRa.print(xValue2);
  LoRa.print(".");
  LoRa.print(yValue2);
  LoRa.print(">");
  LoRa.print(payload);
  LoRa.print("/");
  LoRa.print(kickback);

  LoRa.endPacket();
}
