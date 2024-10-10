
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

//wifi mqtt setup
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>




const char* ssid = "Thor2558";
const char* password = "0831131238";
const char* mqtt_server = "broker.mqtt.cool";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  randomSeed(micros());


  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  for (int i = 0; i < length; i++) {
    // Serial.print((char)payload[i]);
  }
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
    } else {
      // Serial.print("failed, rc=");
      // Serial.print(client.state());
      // Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(38400);
  // Serial.println("drone begin");
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
    // Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  // Serial.println("LoRa Receiver Started");
  //mqtt
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  interval = millis() + 10000;
}

void loop() {
  // checkWiFIAndMQTT();
  // put your main code here, to run repeatedly:

  //mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
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
  // Serial.print("x = ");
  // Serial.print(xValue);
  // Serial.print(", y = ");
  // Serial.print(yValue);
  // Serial.print(", x2 = ");
  // Serial.print(xValue2);
  // Serial.print(", y2 = ");
  // Serial.print(yValue2);
  // Serial.print(", hover = ");
  // Serial.print(hoverLock);
  // Serial.print(", payload = ");
  // Serial.println(payload);
  // delay(1);

  if (millis() < interval && !kickback) {  //normally sends joy data
    sendData();
    // Serial.println(count);
    // Serial.println("sending");
  } else if (kickback) {        // waiting for data when in kickback
    if (millis() < interval) {  //enter 3 second period to catch data
      int packetSize = LoRa.parsePacket();
      if (packetSize) {  //if caught get out of kickback,set interval and var
        // Serial.println("package recieved");
        count += 1;
        kickback = 0;
        interval = millis() + 5000;  // catch again in 5 sec
        String rawInput;
        while (LoRa.available()) {
          rawInput += ((char)LoRa.read());  //recieve message as ascii char and put into string
        }
        // Serial.println(rawInput);
        temp = rawInput.substring(0, rawInput.indexOf(",")).toInt();
        humid = rawInput.substring(rawInput.indexOf(",") + 1, rawInput.indexOf("<")).toInt();
        mpux = rawInput.substring(rawInput.indexOf("<") + 1, rawInput.indexOf(".")).toInt();
        mpuy = rawInput.substring(rawInput.indexOf(".") + 1, rawInput.indexOf(">")).toInt();
        mpuz = rawInput.substring(rawInput.indexOf(">") + 1, -1).toInt();
        // Serial.print("temp = ");
        // Serial.print(temp);
        // Serial.print(", humid = ");
        // Serial.print(humid);
        // Serial.print(", mpux = ");
        // Serial.print(mpux);
        // Serial.print(", mpuy = ");
        // Serial.print(mpuy);
        // Serial.print(", mpuz = ");
        // Serial.println(mpuz);
        // mqtt send
        std::string s = std::to_string(humid);
        std::string t = std::to_string(temp);
        std::string u = s + "," + t;
        char const* c = u.c_str();
        client.publish("Dronedata", c);
        Serial.println(c);
      }
    } else {  //cant catch in timeframe,get out of kickback and continue sending
      kickback = 0;
      interval = millis() + 5000;
      // Serial.println("kickback failed,proceed");
    }
  } else {  //interval reached,entering kickback and sending signal to request data back from drone
    kickback = 1;
    kickround = 0;
    // Serial.println("kicking");
    while (kickround < 5) {
      sendData();
      kickround += 1;
    }
    interval = millis() + 3000;
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
