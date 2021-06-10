#include <ArduinoJson.h>
#include <Wire.h>


// Конфигурация датчиков и актуаторов
#define DATA_SIGNAL_PIN 5        // Чтобы сигнализировать об успешном (или не очень) сборе данных
#define CONNECTION_SIGNAL_PIN 4  // Чтобы сигнализировать о подключении NodeMCU сначала друг с другом, а затем к серверу с MQTT-брокером
#define WORK_SIGNAL_PIN 3        // Чтобы сигнализировать о работе самого микроконтроллера

// Настройка I2C-подключения к NodeMCU
#define SDA_PIN A4
#define SCL_PIN A5

// Данные, которые будут рандомно подаваться
int    h     = 0;
float  t     = 0;
int    co    = 0;
int    co2   = 0;
int    lpg   = 0;
int    smoke = 0;

bool isPartTwo = false;

void setup() {
  pinMode(DATA_SIGNAL_PIN, OUTPUT);
  pinMode(CONNECTION_SIGNAL_PIN, OUTPUT);
  pinMode(WORK_SIGNAL_PIN, OUTPUT);
  digitalWrite(WORK_SIGNAL_PIN, HIGH); // Если плата работает - заработает светодиод. Так лучше, чем индикация по питанию, т.к. появляется возможность сигнализировать об ошибках
  
  Serial.begin(9600);
  
  Wire.begin(8);                /* задаем на шине i2c 8 адрес */
  Wire.onReceive(receiveEvent); /* регистрируем полученное событие */
  Wire.onRequest(requestEvent); /* регистрируем запрошенное событие */
}

void loop() { 
  Serial.println("Произошел залуп");
  
  h = random(0, 100);
  t = random(-40, 40);
  co = random(0, 1000);
  co2 = random(0, 1000);
  lpg = random(0, 1000);
  smoke = random(0, 100);
  Serial.println(h);
  Serial.println(t);
  Serial.println(lpg);

  delay(2000);
}

// Функция для извлечения любых принимаемых данных от мастера на шину
void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    char c = Wire.read();      
//    Serial.print(c);           
  }
//  Serial.println();             
}
 
// Функция для извлечения любых отправляемых данных от мастера на шину
void requestEvent() {
  
//  for (int i = 0; i < msg.length(); i++) {
//    char c = msg[i];
//    
//    Wire.write(c); 
//  }

  if (!isPartTwo) {
    DynamicJsonDocument doc(1024);
    String msg = "";
    doc["hum"] = h;
    doc["temp"] = t;
    doc["co"] = co;
    serializeJson(doc, msg);
    Serial.println("READY TO SEND: ");
    Serial.println(msg);
    Wire.write(msg.c_str());
    isPartTwo = true;
  }
  else {
    DynamicJsonDocument doc(1024);
    String msg = "";
    doc["co2"] = co2;
    doc["lpg"] = lpg;
    doc["smk"] = smoke;
    serializeJson(doc, msg);
    Serial.println("READY TO SEND: ");
    Serial.println(msg);
    Wire.write(msg.c_str());
    isPartTwo = false;
  }
 
//  msg = "";
//  serializeJson(doc2, msg);
//  Serial.println("READY TO SEND: ");
//  Serial.println(msg);
//  Wire.println(msg); 
}
