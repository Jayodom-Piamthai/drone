//joystick setup
#define VRX_PIN 25  // Arduino pin connected to VRX pin
#define VRY_PIN 26  // Arduino pin connected to VRY pin
#define VRX2_PIN 32
#define VRY2_PIN 13
int xValue = 0;  // To store value of the X axis
int yValue = 0;  // To store value of the Y axis
int xValue2 = 0;
int yValue2 = 0;
int deadZone = 60; // set according to joystick drift
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
#define payloadPin 2
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
  //x goes 0-4095 from left to right offset 25 up(~485)
  //y goes 0-4095 from top to bottom (bot-top with mapping)offset 20 down (~537)
  //middle 212
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
  xValue = map(xValue, 0, 4095, 0, 1023);
  yValue = map(yValue, 0, 4095, 1023, 0);
  xValue2 = analogRead(VRX2_PIN);
  yValue2 = analogRead(VRY2_PIN);
  xValue2 = map(xValue2, 0, 4095, 0, 1023);
  yValue2 = map(yValue2, 0, 4095, 1023, 0);
  if ((512 + deadZone > xValue) && (xValue > 512) || (512 - deadZone < xValue) && (xValue < 512)) {
    xValue = 512;
  }
  if ((512 + deadZone > xValue2) && (xValue2 > 512) || (512 - deadZone < xValue2) && (xValue2 < 512)) {
    xValue2 = 512;
  }
  // if ((512 + deadZone > yValue) && (yValue > 512) || (512 - deadZone < yValue) && (yValue < 512)) {
  //   yValue = 512;
  // }//Dissabled due to yValue controlling thrust force
  if ((512 + deadZone > yValue2) && (yValue2 > 512) || (512 - deadZone < yValue2) && (yValue2 < 512)) {
    yValue2 = 512;
  }
  if (digitalRead(payloadPin) == true) {
    payload = 1;
  } else {
    payload = 0;
  }
  // print data to Serial Monitor on Arduino IDE
  Serial.print("x = ");
  Serial.print(xValue);
  Serial.print(", y = ");
  Serial.print(yValue);
  Serial.print(", x2 = ");
  Serial.print(xValue2);
  Serial.print(", y2 = ");
  Serial.println(yValue2);
  delay(1);

  if (millis() < interval && !kickback) {
    sendData();
    Serial.println(count);
    Serial.println("sending");
  } else if (kickback) {
    if (millis() < interval) {
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
        mpuz = rawInput.substring(rawInput.indexOf(">") + 1, -1).toInt();
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
    } else {
      kickback = 0;
      interval = millis() + 5000;
      Serial.println("kickback failed,proceed");
    }
  } else {
    kickback = 1;
    kickround = 0;
    Serial.println("kicking");
    while (kickround < 5) {
      sendData();
      kickround += 1;
    }
    interval = millis() + 5000;
  }
}

//LoRa send and recieves
void sendData() {
  // Serial.print(count);
  // Serial.print(kickback);
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
  LoRa.println(kickback);

  LoRa.endPacket();
}
