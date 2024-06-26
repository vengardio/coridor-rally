#include <Servo.h>
Servo servo, motor;

//===pins===
#define ESC 9
#define SERVO 3

//===constants===
#define SERVO_ZERO 80

uint8_t US_PIN[][2] = {
  { 4, 5 }, { 10, 11 }, { 6, 7 }  //L -- M -- R ;;; TRIG -- ECHO
};
uint16_t cm[3];
uint16_t prevcm[][3] = {
  { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
};
uint16_t averange[3];
int16_t err, prevErr;
uint8_t dt;
uint8_t lineCount = 0;
bool lines[][2] = {
  { 0, 0 }, { 0, 0 }, { 0, 0 }  // L -- F -- R ;;; PREVIOUS -- NOW
};
float PIDKs[] = { 0.17, 0.26, 0.4, 0.15 };  //KP1 -- KD1 -- KP2 -- KD2
float PIDHAHA;
long prevTime, prevTimeLiners, StartTime, PrevNoWallsTime;

void setup() {
  Serial.begin(9600);
  //===ultrasonics attaching===
  for (int i = 0; i < 3; i++) {
    pinMode(US_PIN[i][0], OUTPUT);
    pinMode(US_PIN[i][1], INPUT);
  }

  //===liners attaching===
  for (int i = 0; i < 3; i++) {
    pinMode(i, INPUT);
  }

  //===modules attaching===
  servo.attach(SERVO);
  motor.attach(ESC, 1000, 2000);

  //===motor calibration===
  motor.write(0);
  delay(2100);
  motor.write(180);
  motor.write(90);
  delay(3000);

  StartTime = millis();
  PrevNoWallsTime = millis();
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

  //===Liner's analog values to boolean===
  if (analogRead(A1) >= 970) lines[0][1] = true;  //===Left pin===
  if (analogRead(A2) >= 970) lines[1][1] = true;  //===Back pin===
  if (analogRead(A0) >= 970) lines[2][1] = true;  //===Right pin===
  if (analogRead(A1) < 970) lines[0][1] = false;
  if (analogRead(A2) < 970) lines[1][1] = false;
  if (analogRead(A0) < 970) lines[2][1] = false;

  //===PID Regulator===
  dt = millis() - prevTime;
  err = averange[2] - averange[0];
  if (averange[1] >= 50) PIDHAHA = err * PIDKs[0] + (err - prevErr) / dt * PIDKs[1];
  if (averange[1] < 50) PIDHAHA = err * PIDKs[2] + (err - prevErr) / dt * PIDKs[3];

  //===there's wall in front===
  if (averange[1] <= 40) {
    motor.write(75);
    PIDHAHA = -1 * PIDHAHA;
    servo.write(SERVO_ZERO + PIDHAHA);
    delay(700);
    PIDHAHA = -1 * PIDHAHA;
    StartTime = millis();
  }

  //===PID filter===
  if (PIDHAHA >= 20) PIDHAHA = 20;
  if (PIDHAHA < -20) PIDHAHA = -20;

  //===switching the line-crossing===
  if (lineCount <= 2) {  //===just way or stones. No reason to up car's speed, because stones are small===
    servo.write(SERVO_ZERO + PIDHAHA);
    if (millis() - StartTime <= 400) {
      motor.write(102);
    } else {
      motor.write(99);
    }
  }
  if (lineCount == 3) {  //===no walls===
    if (millis() - PrevNoWallsTime <= 7000) {
      if (lines[0][1] and !lines[2][1]) servo.write(SERVO_ZERO - 15);
      if (!lines[0][1] and lines[2][1]) servo.write(SERVO_ZERO + 15);
      if (lines[0][1] and lines[2][1]) servo.write(SERVO_ZERO);
      if (!lines[0][1] and !lines[2][1]) servo.write(SERVO_ZERO);
    }
    if (millis() - PrevNoWallsTime > 7000) {
      lineCount = 0;
    }
    motor.write(98);
  }
  if (lineCount == 4) {  //===stop-line===
    servo.write(SERVO_ZERO + PIDHAHA);
    if (lines[0][1] and lines[1][1]) {
      motor.write(70);
      delay(200);
      motor.write(90);
      delay(4000);
      motor.write(98);
      lineCount = 0;
    }
  }
  if (lineCount > 4) {  //===we catched error, don't worry===
    servo.write(SERVO_ZERO + PIDHAHA);
    motor.write(98);
  }

  //счётчик линий
  if (!lines[0][0] and lines[0][1]) {
    if ((millis() - prevTimeLiners) > 400 and lineCount != 4) {
      lineCount = 1;  //обнуление счётчика. Давно не было линий, считаем заново.
    } else {
      lineCount++;
      if (lineCount == 4) {
        delay(50);  //важен делей, а не миллис. Машинка за это время должна уже задним датчиком коснуться четвёртой линии (не стоп линии)
      }
      if (lineCount == 3) {
        PrevNoWallsTime = millis();  //важен делей, а не миллис. Машинка за это время должна уже задним датчиком коснуться четвёртой линии (не стоп линии)
      }
    }
    prevTimeLiners = millis();
  }
  Serial.println(lineCount);

  //===previous values===
  for (int i = 0; i < 3; i++) {
    lines[i][0] = lines[i][1];
  }
  prevTime = millis();
  prevErr = err;
}
