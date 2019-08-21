void setup() {
  Serial.begin(230400);
}

void loop() {
  char json[] = "#{\"sensor\":\"gps\",\"time\":1351824120,{}\"data\":{48.756080,2.302038}}$";
  Serial.write(json); //Write the serial data
} 
