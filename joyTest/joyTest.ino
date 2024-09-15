//joystick setup
#define VRX_PIN 25  // Arduino pin connected to VRX pin
#define VRY_PIN 26  // Arduino pin connected to VRY pin
#define VRX_PIN2 32  // Arduino pin connected to VRX pin
#define VRY_PIN2 33  // Arduino pin connected to VRY pin
int xValue;
int yValue;
int xValue2;
int yValue2;
int deadZone = 40;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
}

void loop() {
  // put your main code here, to run repeatedly:
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
  xValue = map(xValue, 0, 4095, 0, 1023);
  yValue = map(yValue, 0, 4095, 1023, 0);
  xValue2 = analogRead(VRX_PIN2);
  yValue2 = analogRead(VRY_PIN2);
  xValue2 = map(xValue2, 0, 4095, 0, 1023);
  yValue2 = map(yValue2, 0, 4095, 1023, 0);
  // if ((512 + deadZone > xValue) && (xValue > 512) || (512 - deadZone < xValue) && (xValue < 512)) {
  //   xValue = 512;
  // }
  // if ((512 + deadZone > xValue2) && (xValue2 > 512) || (512 - deadZone < xValue2) && (xValue2 < 512)) {
  //   xValue2 = 512;
  // }
  // if ((512 + deadZone > yValue) && (yValue > 512) || (512 - deadZone < yValue) && (yValue < 512)) {
  //   yValue = 512;
  // }
  // if ((512 + deadZone > yValue2) && (yValue2 > 512) || (512 - deadZone < yValue2) && (yValue2 < 512)) {
  //   yValue2 = 512;
  // }

  // Serial.print("x = ");
  // Serial.print(xValue);
  // Serial.print(", y = ");
  // Serial.println(yValue);
   Serial.print(", x2 = ");
   Serial.print(xValue2);
   Serial.print(", y2 = ");
   Serial.println(yValue2);

  delay(10);
}
