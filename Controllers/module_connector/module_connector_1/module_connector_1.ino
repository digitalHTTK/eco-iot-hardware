#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "Qwerty_Beeline"
#define   STATION_PASSWORD "rukavAAA"

#define HOSTNAME "MQTT_Bridge"

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 0, 94);

void sendMessage() ;
bool hasIp = false;
bool mqttConnection = false;

void mqttCallback(char* topic, byte* payload, unsigned int length);
Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

Scheduler userScheduler; 
painlessMesh mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);

void sendMessage() {
  if (!mqttConnection) {
      DynamicJsonDocument doc(1024);
      doc["NodeNum"] = 1;
      doc["Info"] = "NO_MQTT";
      String msg;
      serializeJson(doc, msg);
      mesh.sendBroadcast(msg);
  }
  else {
      DynamicJsonDocument doc(1024);
      doc["NodeNum"] = 1;
      doc["Info"] = "MQTT";
      String msg;
      serializeJson(doc, msg);
      mesh.sendBroadcast(msg);
  }
  taskSendMessage.setInterval((TASK_SECOND * 10));
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.print("OH I GET SOMETHING FROM ");
  Serial.println(from);
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  String data1 = doc["hum"];
  String data2 = doc["temp"];
  String data3 = doc["co"];
  String data4 = doc["co2"];
  String data5 = doc["lpg"];
  String data6 = doc["smk"];

  String dataNum = doc["NodeNum"];
  String dataPublish = doc["PublishData"];

  Serial.println(data1);
  Serial.println(data2);
  Serial.println(data3);
  Serial.println(data4);
  Serial.println(data5);
  Serial.println(data6);
   
   String topic = "ecoiot/from/system";
   if (dataNum.toInt() == 3 && dataPublish.toInt() == 1) topic = "ecoiot/from/node3";
   else if (dataNum.toInt() == 2 && dataPublish.toInt() == 1) topic = "ecoiot/from/node2";
   if (mqttClient.publish(topic.c_str(), msg.c_str())) Serial.println("Succesful publishing");
   else Serial.println("Publishing failed");
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {

}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
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

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("MQTT_Bridge")) {
      mqttConnection = true;
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      mqttClient.subscribe("ecoiot/to");
      mqttClient.publish("ecoiot/from","Ready!");
    } else {
      mqttConnection = false; 
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200); /* открываем серийный порт для дебаггинга */
//  Wire.begin(D1, D2); /* задаем i2c мост через контакты SDA=D1 и SCL=D2 на NodeMCU */

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);
  
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  if (hasIp) {
    if (!mqttClient.connected()) reconnect();
    if (!mqttClient.loop()) mqttClient.connect("MQTT_Bridge");
  }
  else {
    Serial.print(".");
  }
  
  mesh.update();
  mqttClient.loop();

  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    hasIp = true;

    if (mqttClient.connect("MQTT_Bridge")) {
      mqttClient.subscribe("ecoiot/to");
    } 
  }
  
  delay(3000);
}
