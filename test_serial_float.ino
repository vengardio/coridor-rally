void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5000);
}

float poh[4] = {};

void loop() {

  if (Serial.available() > 0) {
    if (Serial.parseFloat() != 0.0) {
      poh[0] = Serial.parseFloat();
      poh[1] = Serial.parseFloat();
      poh[2] = Serial.parseFloat();
      poh[3] = Serial.parseFloat();
    }
  }

  /*
   * 
  if (Serial.available() > 0) {
    Serial.println(Serial.readString());
  }*/ 

  for (byte i = 0; i < 4; ++i) {
    Serial.print("poh ");
    Serial.print(i);
    Serial.print(" ");
    Serial.println(poh[i]);
  }
  delay(5000);
}
