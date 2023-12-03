#include <Servo.h>
Servo servo, motor;

//===pins===
#define ESC 9
#define SERVO 3

//===constants===
#define SERVO_ZERO 80
#define KP1 0.24  //#define KP1 0.1
#define KP2 0.27  //#define KP2 0.27 !!!!!
#define KI 0      //#define KI 0
#define KD1 0.2   //#define KD1 0.35
#define KD2 0.15  //#define KD2 0.1 !!!!!

uint8_t US_PIN[][2] = { { 4, 5 }, { 10, 11 }, { 6, 7 } };  //L -- M -- R ;;; TRIG -- ECHO
uint16_t cm[3];
uint16_t prevcm[][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
uint16_t averange[3];
float integral, PIDHAHA;
int16_t err, prevErr;
long prevTime, prevTimeLiners;
uint8_t dt, lineCount;
bool lines[][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };   // L -- F -- R ;;; PREVIOUS -- NOW 



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
  motor.write(90);
  delay(2100);
  motor.write(90);
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
  if (analogRead(A0) <= 980) lines[0][1] = true;   //===Left pin===
  if (analogRead(A1) <= 966) lines[1][1] = true;   //===Back pin===
  if (analogRead(A2) <= 942) lines[2][1] = true;   //===Right pin===
  if (analogRead(A0) > 985) lines[0][1] = false; 
  if (analogRead(A1) > 971) lines[1][1] = false;
  if (analogRead(A2) > 947) lines[2][1] = false;

  //счётчик линий
  if (!lines[1][0] and lines[1][1]) {
    if ((millis() - prevTimeLiners) > 500 and lineCount != 4) {
      lineCount = 1;  //обнуление счётчика. Давно не было линий, считаем заново.
    } else {
      lineCount++;
      if (lineCount == 4) {
        delay(50);  //важен делей, а не миллис. Машинка за это время должна уже задним датчиком коснуться четвёртой линии (не стоп линии)
      }
    }
    prevTimeLiners = millis();
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
  Serial.print("DT: "); Serial.print(dt);
  Serial.print(" Lines: ");
  Serial.print(analogRead(A0)); Serial.print(" - ");
  Serial.print(analogRead(A1)); Serial.print(" - ");
  Serial.print(analogRead(A2)); Serial.print(" ==bool== ");
  Serial.print(lines[0][1]); Serial.print(" - ");
  Serial.print(lines[1][1]); Serial.print(" - ");
  Serial.println(lines[2][1]);

  //===switching the line-crossing===x
  switch () {
    case 1: //===we crossed 1 line resently===
    case 2: //===we crossed 1 line resently===
    case 3: //===we crossed 1 line resently===
    case 4: //===we crossed 1 line resently===
    default: //===we catched error, don't worry===
       servo.write(SERVO_ZERO + PIDHAHA);
       motor.write(98);
  }

  //===Rotation===
  servo.write(SERVO_ZERO + PIDHAHA);

  //===previous values===
  for (int i = 0; i<3; i++) {
    lines[i][0] = lines[i][1];
  }
  prevTime = millis();
  prevErr = err;
}
