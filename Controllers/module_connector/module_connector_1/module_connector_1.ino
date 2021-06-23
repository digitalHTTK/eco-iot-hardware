#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "Qwerty_IoT"
#define   STATION_PASSWORD "iot_vkr_27"

#define HOSTNAME "MQTT_Bridge"

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
IPAddress mqttBroker(192, 168, 0, 94);

int    h     = 0;
float  t     = 0;
int    co    = 0;
int    co2   = 0;
int    lpg   = 0;
int    smoke = 0;

void sendMessage() ;
bool hasIp = false;
bool mqttConnection = false;

int flag = 0;

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
   if (mqttClient.publish(topic.c_str(), msg.c_str())) Serial.println("Succesful publishing");
   else Serial.println("Publishing failed");
}

void sendMessageToMqtt() {
  Serial.println("SEND");
  DynamicJsonDocument doc(1024);
  doc["NodeNum"] = 1;
  doc["PublishData"] = 1;
  doc["hum"] = h;
  doc["temp"] = t;
  doc["co"] = co;
  doc["co2"] = co2;
  doc["lpg"] = lpg;
  doc["smk"] = smoke;
  String msg;
  serializeJson(doc, msg);
  
  String topic = "ecoiot/from/system";
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
  Serial.begin(115200);
  Wire.begin(D1, D2);

  mesh.setDebugMsgTypes( ERROR | STARTUP );
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

void sendEvent(String msg) {
  Wire.beginTransmission(8); 
  Wire.write(msg.c_str()); 
  Wire.endTransmission();
}

void loop() {
  if (hasIp) {
    if (!mqttClient.connected()) reconnect();
    if (!mqttClient.loop()) mqttClient.connect("MQTT_Bridge");
  }
  else {
    Serial.print(".");
  }
  
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
    hasIp = true;

    if (mqttClient.connect("MQTT_Bridge")) {
      sendEvent("D");
      mqttConnection = true;
      mqttClient.subscribe("ecoiot/to");
    } 
  }

  if(flag == 5) {
    flag = -1;
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

    if(mqttConnection) sendMessageToMqtt();
  }
  flag++;
    
  mesh.update();
  mqttClient.loop();
  delay(2000);
}
