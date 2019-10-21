//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   CLEARTORECIEVEPIN D8
#define   SETCONFIGMODEPIN D2

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
boolean isRecieving = false;
boolean hasToSend = false;
String incoming = "";
String toSend = "";
int incomingByte = 0;    // for incoming serial data
const size_t capacity = JSON_OBJECT_SIZE(1);
// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void serialListener();
void tryReceiving();


Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSerialListen(TASK_SECOND * 1 , TASK_FOREVER, &tryReceiving );

void tryReceiving(){
  digitalWrite(CLEARTORECIEVEPIN,HIGH);
  digitalWrite(CLEARTORECIEVEPIN,LOW);
  delayMicroseconds(20000);
  serialListener();
  
}

void configureSniffer(){
  digitalWrite(SETCONFIGMODEPIN,HIGH);
  digitalWrite(SETCONFIGMODEPIN,LOW);
  //delayMicroseconds(20000);
  DynamicJsonDocument doc(capacity);
  doc["MESHTIME"] = mesh.getNodeTime();
  Serial.print("#");
  serializeJson(doc, Serial);
  Serial.print("$");
  doc.clear();
}

void serialListener() {
  
  while ((Serial.available() > 0)&&!hasToSend) {

    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:s
    char a = (char) incomingByte;
   // Serial.print(a);
    if (a == '#') { //message start
      isRecieving = true;
      incoming = "";
    }
    else if (isRecieving) {
      if (a == '$') { //message end
        isRecieving = false;
        toSend = incoming;
        hasToSend = true;
        digitalWrite(LED_BUILTIN, LOW);
      }
      else {
        incoming += a;
      }
    }
  }

}
void sendMessage() {
  if(hasToSend){
    mesh.sendBroadcast("Message From Sniffer:"+toSend);
    toSend = "";
    hasToSend = false;
    digitalWrite(LED_BUILTIN, HIGH);
  }
  //taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 2 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    //Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  configureSniffer();
}



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CLEARTORECIEVEPIN,OUTPUT);
  pinMode(SETCONFIGMODEPIN,OUTPUT);

  digitalWrite(CLEARTORECIEVEPIN,HIGH);
  digitalWrite(SETCONFIGMODEPIN,HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  userScheduler.addTask( taskSerialListen );
  taskSerialListen.enable();
}

void loop() {
  // it will run the user scheduler as well
    mesh.update();
}
