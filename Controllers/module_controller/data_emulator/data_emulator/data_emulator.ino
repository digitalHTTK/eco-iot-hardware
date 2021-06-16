#include <ArduinoJson.h>
#include <Wire.h>


// Конфигурация датчиков и актуаторов
#define DATA_SIGNAL_PIN 3        // Чтобы сигнализировать об успешном (или не очень) сборе данных
#define CONNECTION_SIGNAL_PIN 4  // Чтобы сигнализировать о подключении NodeMCU сначала друг с другом, а затем к серверу с MQTT-брокером
#define WORK_SIGNAL_PIN 2        // Чтобы сигнализировать о работе самого микроконтроллера

// Настройка I2C-подключения к NodeMCU
#define SDA_PIN A4
#define SCL_PIN A5

// Данные, которые будут рандомно подаваться
int  h     = 0;
int  t     = 0;
int  co    = 0;
int  co2   = 0;
int  lpg   = 0;
int  smoke = 0;

bool isPartTwo = false;

void setup() {
  pinMode(DATA_SIGNAL_PIN, OUTPUT);
  pinMode(CONNECTION_SIGNAL_PIN, OUTPUT);
  pinMode(WORK_SIGNAL_PIN, OUTPUT);
  digitalWrite(WORK_SIGNAL_PIN, HIGH); // Если плата работает - заработает светодиод. Так лучше, чем индикация по питанию, т.к. появляется возможность сигнализировать об ошибках
  
  Serial.begin(9600);
  
  Wire.begin(8);              
  Wire.onReceive(receiveEvent); 
  Wire.onRequest(requestEvent);
}

void loop() { 
  h = random(0, 100);
  t = random(-40, 40);
  co = random(0, 1000);
  co2 = random(0, 1000);
  lpg = random(0, 1000);
  smoke = random(0, 100);

  int a1;
  int a5;
  int i = 0;
  
  String a = "10/20/30/40/50/60";
  char* str = strtok(a.c_str(), "/");
  while (str != NULL)
   {
    switch(i) {
      case 1:
        a1 = atoi(str);
        break;
      case 5:
        a5 = atoi(str);
        break;
    }
      i++;
      // Вывод очередной выделенной части
      Serial.println(String(str));
      // Выделение очередной части строки
      str = strtok (NULL, "/");
   }

  Serial.println("update");
   Serial.println(String(a1));
   Serial.println(String(a5));

  delay(2000);
}

void dataLedWork() {
    digitalWrite(DATA_SIGNAL_PIN, HIGH);
    delay(1000);
    digitalWrite(DATA_SIGNAL_PIN, LOW);
    delay(1000);
}

void signalLedWork(int mode) {
  if (mode == 1) {
    digitalWrite(CONNECTION_SIGNAL_PIN, HIGH);
    delay(1000);
    digitalWrite(CONNECTION_SIGNAL_PIN, LOW);
    delay(1000);
  }
  else if (mode == 2) {
    digitalWrite(CONNECTION_SIGNAL_PIN, HIGH);
  }
}

void receiveEvent(int howMany) {
  String msg = "";
  while (0 < Wire.available()) {
    char c = Wire.read();      
    msg += c;
  }
  
  if (msg[0] == 'A') dataLedWork();
  else if (msg[0] == 'C') signalLedWork(1);
  else if (msg[0] == 'D') signalLedWork(2);
  Serial.print("RECIEVED: ");
  Serial.println(msg);          
}
 
void requestEvent() {
    String msg = "";
    msg += h;
    msg += "/";
    msg += t;
    msg += "/";
    msg += co;
    msg += "/";
    msg += co2;
    msg += "/";
    msg += lpg;
    msg += "/";
    msg += smoke;
    Serial.println("READY TO SEND: ");
    Serial.println(msg);
    Wire.write(msg.c_str());
    delay(2000);
  
//  if (!isPartTwo) {
//    DynamicJsonDocument doc(64);
//    String msg = "";
//    doc["hum"] = h;
//    doc["temp"] = t;
//    doc["co"] = co;
//    serializeJson(doc, msg);
//    Serial.println("READY TO SEND: ");
//    Serial.println(msg);
//    delay(2000);
//    Wire.write(msg.c_str());
//    isPartTwo = true;
//  }
//  else {
//    DynamicJsonDocument doc(64);
//    String msg = "";
//    doc["co2"] = co2;
//    doc["lpg"] = lpg;
//    doc["smk"] = smoke;
//    serializeJson(doc, msg);
//    Serial.println("READY TO SEND: ");
//    Serial.println(msg);
//    delay(2000);
//    Wire.write(msg.c_str());
//    isPartTwo = false;
//  }
}
