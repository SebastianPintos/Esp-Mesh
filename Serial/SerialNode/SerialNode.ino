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
#define   CLEARTORECIEVEPIN 0
#define   SETCONFIGMODEPIN 2

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
boolean isRecieving = false;
boolean hasToSend = false;
String incoming = "";
String toSend = "";
int incomingByte = 0;    // for incoming serial data

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void serialListener();
void tryReceiving();


Task taskSendMessage( TASK_SECOND * 0.3 , TASK_FOREVER, &sendMessage );
Task taskSerialListen(TASK_SECOND*1, TASK_FOREVER, &tryReceiving);

void tryReceiving(){
 // Serial.println("NODE: Sending Interrupt To Sniffer");
  digitalWrite(CLEARTORECIEVEPIN,LOW);
  digitalWrite(CLEARTORECIEVEPIN,HIGH);
  serialListener();


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
void sendMessage() {
  if(hasToSend){
    mesh.sendBroadcast("Message From Sniffer:"+toSend);
    //mesh.sendBroadcast("Sent!");
    //Serial.println("tosend: "+toSend);
    hasToSend = false;
    toSend = "";
  }
 
  
  
  
  //taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CLEARTORECIEVEPIN,OUTPUT);
  pinMode(SETCONFIGMODEPIN,OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  userScheduler.addTask( taskSerialListen );
  taskSerialListen.enable();
  taskSendMessage.enable();
}

void loop() {

  // it will run the user scheduler as well
  if(hasToSend){
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)                
    }
  else{
    digitalWrite(LED_BUILTIN, HIGH);
    }
  
    mesh.update();
}
