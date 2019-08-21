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
String msg = "";
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
short int cont = 0;
int sumCaract = 0;
int maxCont = 5;
float primerMsg = 0;
float ultimoMsg = 0;
float deltaMsg = 0;
float caracteresPorSeg = 0;

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  
  sumCaract += msg.length();
  
  if(maxCont == 5){
    primerMsg = millis();
  }
  if(maxCont == 1){
    maxCont = 6;
    ultimoMsg = millis();
    deltaMsg = ultimoMsg - primerMsg;
    caracteresPorSeg = (sumCaract/(deltaMsg/1000));
    sumCaract = 0;
  }
  maxCont--;
  Serial.print(String(caracteresPorSeg)+ " -> ");
  Serial.println(msg.c_str());

}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  //  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
