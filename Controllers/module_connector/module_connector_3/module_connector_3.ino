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

bool isPartTwo = false;

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage() {
  DynamicJsonDocument doc(1024);
  doc["NodeNum"] = "3";
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
  Serial.print("OH SHIT I GET SOMETHING FROM ");
  Serial.println(from);
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
//  DeserializationError error = deserializeJson(doc, json);
//  if (error) {
//    Serial.print("deserializeJson() failed: ");
//    Serial.println(error.c_str());
//  }
//  String data1 = doc["NodeName"];
//  String data2 = doc["H"];
//  String data3 = doc["T"];
//  
//   Serial.print(data1);
//   Serial.println(data2);
//   Serial.print("h: ");
//   Serial.println(data3);
//   Serial.print("t: ");
//   Serial.println(data4);
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
  Serial.begin(115200); /* открываем серийный порт для дебаггинга */
  Wire.begin(D1, D2); /* задаем i2c мост через контакты SDA=D1 и SCL=D2 на NodeMCU */

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
    if (!isPartTwo) {
      Wire.requestFrom(8, 32); 
      String requestMsg = "";
      while(Wire.available()){
         char c = Wire.read();
         requestMsg += c;
      }
      Serial.print("RECIEVED 1: ");
      Serial.println(requestMsg);
    
      String json;
      DynamicJsonDocument doc(1024);
      json = requestMsg.c_str();
      DeserializationError error = deserializeJson(doc, json);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }
      else Serial.println(requestMsg.c_str());
      String data1 = doc["hum"];
      String data2 = doc["temp"];
      String data3 = doc["co"];
      h = data1.toInt();
      t = data2.toFloat();
      co = data3.toInt();
      Serial.println(h);
      Serial.println(co);
      isPartTwo = true;
  }
  else {
      Wire.requestFrom(8, 32); 
      String requestMsg = "";
      while(Wire.available()){
         char c = Wire.read();
         requestMsg += c;
      }
      Serial.print("RECIEVED 2: ");
      Serial.println(requestMsg);
    
      String json;
      DynamicJsonDocument doc(1024);
      json = requestMsg.c_str();
      DeserializationError error = deserializeJson(doc, json);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }
      else Serial.println(requestMsg.c_str());
      String data4 = doc["co2"];
      String data5 = doc["lpg"];
      String data6 = doc["smk"];
      co2 = data4.toInt();
      lpg = data5.toInt();
      smoke = data6.toInt();
      Serial.println(smoke);
      Serial.println(co2);
      isPartTwo = false;
  }
  
  mesh.update();
  
  delay(1000);
}
