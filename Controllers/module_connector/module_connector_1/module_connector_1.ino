#include <Wire.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "ecoiot"
#define   MESH_PASSWORD   "123456"
#define   MESH_PORT       5555

Scheduler userScheduler; 
painlessMesh mesh;

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage() {
  DynamicJsonDocument doc(1024);
  doc["Data1"] = "Message!1!";
  doc["Data2"] = "From node 1";
  String msg;
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
  Serial.println(msg);
  taskSendMessage.setInterval((TASK_SECOND * 1));
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
  String data1 = doc["Data1"];
  String data2 = doc["Data2"];
   Serial.print(data1);
   Serial.println(data2);
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
  Serial.begin(9600); /* открываем серийный порт для дебаггинга */
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
  Wire.beginTransmission(8); /* Начинаем передачу на адресе 8 */
  Wire.write("Hello Arduino");  /* Отправляем "hello Arduino" */
  Wire.endTransmission();    /* прекращаем передачу */
  
  Wire.requestFrom(8, 13); /* запрашиваем и считываем данные с 8 и 13 адреса slave устройства */
  while(Wire.available()){
    char c = Wire.read();
  Serial.print(c);
  }
  Serial.println();

  mesh.update();
  
  delay(1000);
}
