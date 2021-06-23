#include <Wire.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; 
painlessMesh mesh;

void sendMessage() ;

int    h     = 0;
float  t     = 0;
int    co    = 0;
int    co2   = 0;
int    lpg   = 0;
int    smoke = 0;

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage() {
  DynamicJsonDocument doc(1024);
  doc["NodeNum"] = 2;
  doc["PublishData"] = 1;
  doc["hum"] = h;
  doc["temp"] = t;
  doc["co"] = co;
  doc["co2"] = co2;
  doc["lpg"] = lpg;
  doc["smk"] = smoke;
  String msg;
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
  Serial.println(h);
  Serial.println(co);
  Serial.println(msg);
  taskSendMessage.setInterval((TASK_SECOND * 10));
}

void receivedCallback(uint32_t from, String &msg) {
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  Serial.println("принято: ");
  Serial.println(msg);
  
  String data1 = doc["NodeNum"];
  String data2 = doc["Info"];

  if (data1.toInt() == 1 && data2 == "NO_MQTT") {
    sendEvent("C");
  }
  else if (data1.toInt() == 1 && data2 == "MQTT") {
    sendEvent("D");
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}


void setup() {
  Serial.begin(115200); 
  Wire.begin(D1, D2); 

  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void sendEvent(String msg) {
  Wire.beginTransmission(8); 
  Wire.write(msg.c_str()); 
  Wire.endTransmission();
}

void loop() { 
  Wire.requestFrom(8, 32); 
  String requestMsg = "";
  while(Wire.available()){
     char c = Wire.read();
     requestMsg += c;
  }
  Serial.print("RECIEVED: ");
  Serial.println(requestMsg);

  int msg_len = requestMsg.length() + 1;
  char msg[msg_len];
  requestMsg.toCharArray(msg, msg_len);
  char* msg_c = strtok(msg, "/");
  int i = 0;
  while (msg_c != NULL) {
    switch(i) {
      case 0:
        h = atoi(msg_c);
        break;
      case 1:
        t = atoi(msg_c);
        break;
      case 2:
        co = atoi(msg_c);
        break;
      case 3:
        co2 = atoi(msg_c);
        break;
      case 4:
        lpg = atoi(msg_c);
        break;
      case 5:
        smoke = atoi(msg_c);
        break;
    }
    i++;
    msg_c = strtok (NULL, "/");
  }

  if (h == 0 && t == 0 && co == 0 && co2 == 0 && lpg == 0 && smoke == 0) sendEvent("B");
  else sendEvent("A");
  
  mesh.update();
  
  delay(5000);
}
