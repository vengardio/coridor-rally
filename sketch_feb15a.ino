#include <Servo.h>
Servo motor;

#define ESC 9

int value;

void setup() {
  Serial.begin(9600);
  motor.attach(ESC, 2000, 1000);
  pinMode(A3, INPUT);
  motor.write(0);
  delay(2500);
  motor.write(180);
  motor.write(90);
  delay(3000);
}

void loop() {
  /*
  value = map(analogRead(A3), 0, 1023, 0, 180);
  Serial.println(value);
  motor.write(value);
*/
  motor.write(90);
  delay(3000);
  motor.write(105);
  delay(1000);
  motor.write(72);
  delay(1000);
  motor.write(90);
  delay(3000);
}
