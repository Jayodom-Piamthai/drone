//MPU
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#define OUTPUT_READABLE_YAWPITCHROLL //for teapot visualization
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int valx , valy , valz;
char rd;
int prevVal;
int ledR = 3 ;
int ledG = 4 ;
int ledY = 5 ;
int pin11 = 11 , pin10 = 10 ;
int val1 , val2 ;
int valgy1 = 0 , valgy2 = 0;
int margin = 30; // set margin for mpu to joy control

//dht22
int temp;
int humid;
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTTYPE    DHT22
#define DHTPIN 3
DHT_Unified dht(DHTPIN, DHTTYPE);

//drone motor
#include <Servo.h>
Servo ESC1;     // create servo object to control the ESC
Servo ESC2;
Servo ESC3;
Servo ESC4;
int potValue;  // value from the analog pin

//LoRa
#include <SPI.h>
#include <LoRa.h>
#define ss 10
#define rst 9
#define dio0 2
long interval = 0;
int kickback = 0;
int kickround = 0;

//drone control
#define payloadPin 2
int xValue ;
int yValue ;
int xValue2 ;
int yValue2 ;
bool recieved;
long lastPack;
int payload;
long floatTime = 120000;//emergency floatdown time


void setup()
{
  millis();
  Serial.begin(38400);

  //MPU
  pinMode(ledR, OUTPUT) ;
  pinMode(ledG, OUTPUT) ;
  pinMode(ledY, OUTPUT) ;
  Wire.begin();
  Serial.println("Initialize MPU");
  mpu.initialize();
  Serial.println(mpu.testConnection() ? "Connected" : "Connection failed");

  //drone motor
  // Attach the ESC on pin 9
  ESC1.attach(5, 1000, 2000); // (pin, min pulse width, max pulse width in microseconds)
  ESC2.attach(6, 1000, 2000);
  ESC3.attach(7, 1000, 2000);
  ESC4.attach(8, 1000, 2000);

  //dht22
  dht.begin();


  //LoRa

  LoRa.setPins(ss, rst, dio0);
  while (!Serial);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Receiver Started");
}

void loop()
{
  //MPU
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  valx = map(ax, -17000, 17000, 0, 1023); //left and right tilting,left goes toward 1023 and right goes toward 0,idealy remains at 512 flat
  valy = map(ay, -17000, 17000, 0, 1023); //forward and backward tilting, goes toward 1023 and right goes toward ,idealy remains at 512 flat
  valz = map(az, -17000, 17000, 0, 1023); //parallel to the ground,ard faces upward toward the sky perfectly parallel at 1023 and lower to 0 as the top face toward the ground
//  for visualization/
  Serial.print(valx) ;
  Serial.print("/") ;
  Serial.print(valy) ;
  Serial.print("/") ;
  Serial.print(valz) ;
  Serial.println("/") ;

  //dht22
  if ((millis() % 5000 ) == 0) {
    Serial.println("getting dht data");
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    temp = event.temperature ;
    dht.humidity().getEvent(&event);
    humid = event.relative_humidity;
    Serial.println(kickback);
    Serial.println(temp);
    Serial.println(humid);
  }

  //LoRa reciever + drone control
  recieved = false;
  if (!kickback) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      recieved = true;
      lastPack = millis();
      String rawInput;
      while (LoRa.available()) {
        rawInput += ((char)LoRa.read());//recieve message as ascii char and put into string
      }
      Serial.print(rawInput);
      xValue = rawInput.substring(0, rawInput.indexOf(",")).toInt() ;
      yValue = rawInput.substring(rawInput.indexOf(",") + 1, rawInput.indexOf("<")).toInt();
      xValue2 = rawInput.substring(rawInput.indexOf("<") + 1, rawInput.indexOf(".")).toInt();
      yValue2 = rawInput.substring(rawInput.indexOf(".") + 1, rawInput.indexOf(">")).toInt();
      payload = rawInput.substring(rawInput.indexOf(">") + 1, rawInput.indexOf("/")).toInt();
      kickback = rawInput.substring(rawInput.indexOf("/") + 1, -1).toInt();
      Serial.print("x = ");
      Serial.print(xValue);
      Serial.print(", y = ");
      Serial.print(yValue);
      Serial.print(", x2 = ");
      Serial.print(xValue2);
      Serial.print(", y2 = ");
      Serial.print(yValue2);
      Serial.print(", payload = ");
      Serial.print(payload);
      Serial.print(", kickback = ");
      Serial.println(kickback);
    }
  }
  else {
    kickback = 0;
    kickround = 0;
    interval = millis() + 500;
    while (millis() < interval) {
      sendData();
    }
  }

  //drone motor
  //test pot
  //  potValue = analogRead(A0);   // reads the value of the potentiometer (value between 0 and 1023) for test
  //  potValue = map(potValue, 0, 1023, 0, 180);   // scale it to use it with the servo library (value between 0 and 180)
  //  Serial.println(potValue);
  //  ESC1.write(potValue);    // Send the signal to the ESC
  //  ESC2.write(potValue);
  //  ESC3.write(potValue);
  //  ESC4.write(potValue);

  if (!recieved) {
    if (millis() - lastPack > 1300) {
      if (yValue > 100) { // emergency landing protocol
        interval = millis() + floatTime;
        if (millis())
          ESC1.write(444); //top left -aclk
        ESC2.write(444); //top right - clk
        ESC3.write(444); //bottom left -aclk
        ESC4.write(444); //bottom left - clk
      }
      else  { // landing protocol
        ESC1.write(0); //top left -aclk
        ESC2.write(0); //top right - clk
        ESC3.write(0); //bottom left -aclk
        ESC4.write(0); //bottom left - clk
      }
    }
  }
  else  {

    //Joy1:thrust Y and turn X | Joy2:roll X and pitch Y
    // Send the signal to the ESC
    //x goes 0-4095 from left to right offset 25 up(~485)
    //y goes 0-4095 from top to bottom (bot-top with mapping)offset 20 down (~537)
    //yValue for drone thrust
    //xValue for turning the head of drone left or right - clk/aclk
    //yValue2 for forwarding and backwarding -top bottom
    //xValue2 for left and right rolling -left right
    ESC1.write(yValue); //top left -aclk
    ESC2.write(yValue); //top right - clk
    ESC3.write(yValue); //bottom left -aclk
    ESC4.write(yValue); //bottom left - clk
    //    ESC1.write(yValue - ((xValue - 512) / 4) - (yValue2 - 512 / 4) - ((512 - xValue2) / 4)); //top left -aclk
    //    ESC2.write(yValue - ((512 - xValue) / 4) - (512 - yValue2 / 4) - ((xValue2 - 512) / 4)); //top right - clk
    //    ESC3.write(yValue - ((xValue - 512) / 4) - (yValue2 - 512 / 4) - ((512 - xValue2) / 4)); //bottom left -aclk
    //    ESC4.write(yValue - ((512 - xValue) / 4) - (512 - yValue2 / 4) - ((512 - xValue2) / 4)); //bottom left - clk
  }
}

//LoRa send and recieves
void sendData() {
  Serial.println("sending back");
  LoRa.beginPacket();
  LoRa.print(temp);
  LoRa.print(",");
  LoRa.print(humid);
  LoRa.print("<");
  LoRa.print(valx);
  LoRa.print(".");
  LoRa.print(valy);
  LoRa.print(">");
  LoRa.print(valz);
  LoRa.endPacket();
}
