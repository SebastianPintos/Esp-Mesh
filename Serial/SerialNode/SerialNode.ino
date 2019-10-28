//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
#include <ArduinoJson.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define   CLEARTORECIEVEPIN D2 //WEEMOSD1R1
#define   SETCONFIGMODEPIN D8
//#define CLEARTORECIEVEPIN D0  //D1_MINI
//#define SETCONFIGMODEPIN D4

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
boolean isRecieving = false;
boolean hasToSend = false;
String incoming = "";
String toSend = "";
int incomingByte = 0;    // for incoming serial data
const size_t capacity = JSON_OBJECT_SIZE(2);
// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void serialListener();
void tryReceiving();


Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSerialListen(TASK_SECOND * 1 , TASK_FOREVER, &tryReceiving );

void recieveFromSniffer(){
  digitalWrite(CLEARTORECIEVEPIN,HIGH);
  digitalWrite(CLEARTORECIEVEPIN,LOW);
  delayMicroseconds(20000);
  serialListener();
}

void configureSniffer(){
  digitalWrite(SETCONFIGMODEPIN,HIGH);
  digitalWrite(SETCONFIGMODEPIN,LOW);
  //
  DynamicJsonDocument doc(capacity);
  doc["MESHTIME"] = mesh.getNodeTime();
  doc["CHANNEL"] = 1;
  Serial.print("#");
  serializeJson(doc, Serial);
  Serial.println("$");
  doc.clear();
}

void tryReceiving(){
  recieveFromSniffer();
  
} 


void toggleLED(){
  if(digitalRead(LED_BUILTIN)==LOW){
    digitalWrite(LED_BUILTIN, HIGH);

  }
  else{
    digitalWrite(LED_BUILTIN,LOW);
  }

}
void serialListener() {
  
  while ((Serial.available() > 0)&&!hasToSend) {

    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:s
    char a = (char) incomingByte;
    toggleLED();
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
       
      }
      else {
        incoming += a;
      }
    }
  }

}
void sendMessage() {
  if(hasToSend){

    mesh.sendBroadcast("{\"t\":"
    +String(mesh.getNodeTime())+",\"m\":"+toSend+"}");
    //mesh.sendBroadcast(toSend);   
    toSend = "";
    hasToSend = false;
    
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

    if(hasToSend){
       digitalWrite(LED_BUILTIN, LOW);
    }
    else{
       digitalWrite(LED_BUILTIN, HIGH);
    }
    mesh.update();
}
