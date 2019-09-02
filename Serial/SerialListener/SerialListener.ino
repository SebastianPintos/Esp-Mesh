boolean isRecieving = false;
boolean hasToSend = false;
String incoming = "";
String toSend = "";
int incomingByte = 0;    // for incoming serial data
void setup() {
  Serial.begin(230400);

}

void loop() {
  serialListener();
  if (hasToSend) {
    send(toSend);
  }

}
void send(String msg) {
  Serial.println(msg);
  hasToSend = false;
  toSend = "";
}
void serialListener() {
  if (Serial.available() > 0) {

    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:
    char a = (char) incomingByte;
    if (a == '#') { //message start
      isRecieving = true;
      incoming = "";
    }
    else if (isRecieving) {
      if (a == '$') { //message end
        isRecieving = false;
        hasToSend = true;
        toSend = "";
        toSend += incoming;
      }
      else {
        incoming += a;
      }
    }
  }

}
