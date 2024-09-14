// Motor A

int dir1PinA = 2;

int dir2PinA = 3;

int speedPinA = 6; //   เพื่อให้ PWM สามารถควบคุมความเร็วมอเตอร์ 


void setup()
{
  Serial.begin(9600);

  //กำหนด ขา เพื่อใช้ในการควบคุมการทำงานของ  Motor ผ่านทาง L298N

  pinMode(dir1PinA,OUTPUT);

  pinMode(dir2PinA,OUTPUT);

  pinMode(speedPinA,OUTPUT);
}

void loop()
{

// Motor A

  analogWrite(speedPinA, 255); //ตั้งค่าความเร็ว PWM ผ่านตัวแปร ค่าต่ำลง มอเตอร์จะหมุนช้าลง

  digitalWrite(dir1PinA, LOW);

  digitalWrite(dir2PinA, HIGH);

  Serial.println("clock");

  delay(3000);

  analogWrite(speedPinA, 100); //ตั้งค่าความเร็ว PWM ผ่านตัวแปร ค่าต่ำลง มอเตอร์จะหมุนช้าลง

  Serial.println("Unclock");
  delay(3000);
}