#include <Servo.h>
Servo servo, motor;

//===pins===
#define ESC 9
#define SERVO 3

//===constants===
#define SERVO_ZERO 80
#define KP1 0.24 //#define KP1 0.1
#define KP2 0.27 //#define KP2 0.27 !!!!!
#define KI 0 //#define KI 0
#define KD1 0.2 //#define KD1 0.35
#define KD2 0.15 //#define KD2 0.1 !!!!!

uint8_t US_PIN[][2] = { { 4, 5 }, { 10, 11 }, { 6, 7 } };  //L -- M -- R ;;; TRIG -- ECHO
uint16_t cm[3];
uint16_t prevcm[][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
uint16_t averange[3];
float integral, PIDHAHA;
int16_t err, prevErr;
long prevTime;
uint8_t dt;


void setup() {
  //Serial.begin(9600);
  
  //===ultrasonics attaching===
  for (int i = 0; i < 3; i++) {
    pinMode(US_PIN[i][0], OUTPUT);
    pinMode(US_PIN[i][1], INPUT);
  }
  
  //===modules attaching===
  servo.attach(SERVO);
  motor.attach(ESC, 1000, 2000);
  motor.write(90);
  delay(2100);
  motor.write(98);
}

void loop() {
  for (int i = 0; i < 3; i++) {
    //===rangefinder find===
    digitalWrite(US_PIN[i][0], LOW);
    delayMicroseconds(5);
    digitalWrite(US_PIN[i][0], HIGH);
    delayMicroseconds(10);
    digitalWrite(US_PIN[i][0], LOW);
    cm[i] = (pulseIn(US_PIN[i][1], HIGH)) / 20;
    if (cm[i] < 0) cm[i] = -1 * cm[i];

    //===arifmetic filter===
    prevcm[i][2] = prevcm[i][1];
    prevcm[i][1] = prevcm[i][0];
    prevcm[i][2] = cm[i];
    averange[i] = (prevcm[i][0] + prevcm[i][1] + prevcm[i][2]) / 3;
  }

  //===PID Regulator===
  dt = millis() - prevTime;
  integral += err * dt;
  err = averange[2] - averange[0];
  if (averange[1] >= 50) PIDHAHA = err * KP1 + (err - prevErr) / dt * KD1;
  if (averange[1] < 50) PIDHAHA = err * KP2 + (err - prevErr) / dt * KD2;
  
  //===PID filter===
  if (PIDHAHA >= 20) PIDHAHA = 20;
  if (PIDHAHA < -20) PIDHAHA = -20;
  
  //===Serial printing===
  Serial.print("PID: "); Serial.print(PIDHAHA);
  Serial.print(" I: ");  Serial.print(integral * KI);
  Serial.print(" US: ");
  Serial.print(averange[0]); Serial.print(" - ");
  Serial.print(averange[1]); Serial.print(" - ");
  Serial.println(averange[2]);
  
  //===Rotation===
  servo.write(SERVO_ZERO + PIDHAHA);

  prevTime = millis();
  prevErr = err;
}