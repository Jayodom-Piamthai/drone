//joystick setup
#define VRX_PIN 34  // Arduino pin connected to VRX pin
#define VRY_PIN 35  // Arduino pin connected to VRY pin
#define VRX2_PIN 32
#define VRY2_PIN 33
int xValue = 0;  // To store value of the X axis
int yValue = 0;  // To store value of the Y axis
int xValue2 = 0;
int yValue2 = 0;
int deadZone = 90;  // set according to joystick drift
//LoRa setup
#include <SPI.h>
#include <LoRa.h>
//this pin for esp32 wroom
#define ss 5
#define rst 27
#define dio0 35
// #define ss 5
// #define rst 14
// #define dio0 2
// LoRa.setPins(ss, rst, dio0);
int count = 0;
int interval = 0;
int kickback;
int kickround;

//drone data
#define payloadPin 4
#define hoverPin 2
int payload;
int hoverLock;
int humid;
int temp;
int mpux;
int mpuy;
int mpuz;

//wifi & data parsing
#include <WiFi.h>
//wifi config
const char* ssid = "Thor2558";
const char* password = "0831131238";
//mqtt config
#include <PubSubClient.h>
#include <ArduinoMqttClient.h>
const char* broker = "localhost";  //"test.mosquitto.org" -0.0.0.0 is hivemq
int port = 1883;
#define ID_MQTT "esp32_mqtt"
#define TOPIC_SUBSCRIBE_DRONEDATA "topic_droneSend"

WiFiClient wifiClient;
PubSubClient MQTT(wifiClient);

void setup() {
  Serial.begin(38400);
  Serial.println("drone begin");
  millis();
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(VRX_PIN, INPUT);
  pinMode(VRY_PIN, INPUT);
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
  //WiFi connection
  // WiFi.begin(ssid, password);
  // Serial.println("\nConnecting");
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(".");
  //   delay(100);
  // }
  // Serial.println("\nConnected to the WiFi network");
  // Serial.print("Local ESP32 IP: ");
  // Serial.println(WiFi.localIP());
  // initWiFi();
  // initMQTT(); 

  interval = millis() + 10000;
}

void loop() {
  // checkWiFIAndMQTT();
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
  if (digitalRead(hoverPin) == true) {
    if (!hoverLock) {
      if (yValue < 100) {
        yValue = 1;  //for lock at near-stop moving so can be safely turn off
      }
      hoverLock = yValue;
    }
    yValue = hoverLock;
  } else {
    hoverLock = 0;
  }

  // print data to Serial Monitor on Arduino IDE
  Serial.print("x = ");
  Serial.print(xValue);
  Serial.print(", y = ");
  Serial.print(yValue);
  Serial.print(", x2 = ");
  Serial.print(xValue2);
  Serial.print(", y2 = ");
  Serial.print(yValue2);
  Serial.print(", hover = ");
  Serial.print(hoverLock);
  Serial.print(", payload = ");
  Serial.println(payload);
  delay(1);

  if (millis() < interval && !kickback) {  //normally sends joy data
    sendData();
    Serial.println(count);
    Serial.println("sending");
  } else if (kickback) {        // waiting for data when in kickback
    if (millis() < interval) {  //enter 3 second period to catch data
      int packetSize = LoRa.parsePacket();
      if (packetSize) {  //if caught get out of kickback,set interval and var
        Serial.println("package recieved");
        count += 1;
        kickback = 0;
        interval = millis() + 5000;  // catch again in 5 sec
        String rawInput;
        while (LoRa.available()) {
          rawInput += ((char)LoRa.read());  //recieve message as ascii char and put into string
        }
        Serial.println(rawInput);
        MQTT.publish(TOPIC_SUBSCRIBE_DRONEDATA, "what");
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
    } else {  //cant catch in timeframe,get out of kickback and continue sending
      kickback = 0;
      interval = millis() + 5000;
      Serial.println("kickback failed,proceed");
    }
  } else {  //interval reached,entering kickback and sending signal to request data back from drone
    kickback = 1;
    kickround = 0;
    Serial.println("kicking");
    while (kickround < 5) {
      sendData();
      kickround += 1;
    }
    interval = millis() + 3000;
  }

  MQTT.loop();
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

void initWiFi(void) {
  delay(10);
  Serial.println("------Connect to WI-FI------");
  Serial.print("Connecting to : ");
  Serial.println(ssid);

  reconnectWiFi();
}

/* Initial MQTT  */
void initMQTT(void) {
  Serial.print("initial mqtt connecting");
  MQTT.setServer("local host", 1883);
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
}

/* FUnction reconnect MQTT Broker */
void reconnectMQTT(void) {
  while (!MQTT.connected()) {
    Serial.print("* Connecting MQTT Broker: ");
    Serial.println(broker);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Connecting the MQTT Broker success!");
      MQTT.subscribe(TOPIC_SUBSCRIBE_DRONEDATA);
    } else {
      Serial.println("Fail to connect then reconnect MQTT Broker again in 2 seconds.");
      delay(2000);
    }
  }
}

/* Function checking connected to WiFI and MQTT Broker */
void checkWiFIAndMQTT(void) {
  if (!MQTT.connected())
    reconnectMQTT();
  reconnectWiFi();
}

void reconnectWiFi(void) {

  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.println(digitalRead(12));
  delay(2000);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.print("Connected WiFi success. ");
  Serial.print(ssid);
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}
