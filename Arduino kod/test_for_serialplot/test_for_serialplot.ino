const int rssi_values [] = {-76, -56, -87, -53, -88, -54, -52, -89, -99, -102, -65, -64, -76, -86, -60};

const int altitude_values [] = {10.0, 9.5, 9.3, 8.9, 8.4, 8.2, 7.7, 7.4, 7.0, 6.4, 6.1, 5.0, 3.5, 1.7, 0.2, 0.0};

int s;

void setup() {
  // put your setup code here, to run once:
  Serial.begin (9600);
}

void loop() {
  s = sizeof (rssi_values) / 2;
  for (int i = 0; i < s; i++) {
    Serial.print(i);
    Serial.print(",");
    Serial.println(rssi_values[random(s)]);
    delay(500);
  }
  /*
  s = sizeof (altitude_values) / 2;
  for (int i = 0; i < s; i++) {
    Serial.print(i);
    Serial.print(",");
    Serial.println(rssi_values[random(s)]);
    delay(500);
  }
  */
  delay(1);
}
