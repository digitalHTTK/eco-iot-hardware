#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <painlessMesh.h>

#define HOSTNAME "MQTT_Node_1"

const char* ssid = "Qwerty_IoT";
const char* password = "iot_vkr_27";
const char* mqtt_server = "192.168.0.94";
bool hasIp = false;
bool mqttConnection = false;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

int    h     = 0;
float  t     = 0;
int    co    = 0;
int    co2   = 0;
int    lpg   = 0;
int    smoke = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
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

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    sendEvent("C");
    if (mqttClient.connect(HOSTNAME)) {
      mqttConnection = true;
      Serial.println("connected");  
      mqttClient.subscribe("ecoiot/to");
      mqttClient.publish("ecoiot/to","Ready, 1!");
    } else {
      mqttConnection = false; 
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D1, D2);
  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void sendEvent(String msg) {
  Wire.beginTransmission(8); 
  Wire.write(msg.c_str()); 
  Wire.endTransmission();
}

void loop() {
  if (!mqttClient.connected()) reconnect();
  if (!mqttClient.loop()) mqttClient.connect(HOSTNAME);
  
  if (mqttClient.connected()) {
    sendEvent("D");
    mqttConnection = true;
    mqttClient.subscribe("ecoiot/to");
  } 

  getDataByI2C();
  if (h == 0 && t == 0 && co == 0 && co2 == 0 && lpg == 0 && smoke == 0) sendEvent("B");
  else sendEvent("A");

  if(mqttConnection) {
    sendMessageToMqtt();
    sendEvent("D");
  }
  else sendEvent("C");
    
  mqttClient.loop();
  delay(2000);
}

void getDataByI2C() {
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
}
