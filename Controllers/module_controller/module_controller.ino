#include <DHT.h>
#include <MQ2.h>
#include <MQ135.h>
#include <Wire.h>

// Конфигурация датчиков и актуаторов
#define DATA_SIGNAL_PIN 4        // Чтобы сигнализировать об успешном (или не очень) сборе данных
#define CONNECTION_SIGNAL_PIN 3  // Чтобы сигнализировать о подключении NodeMCU сначала друг с другом, а затем к серверу с MQTT-брокером
#define WORK_SIGNAL_PIN 2        // Чтобы сигнализировать о работе самого микроконтроллера
#define DHT_PIN 5
#define MQ135_PIN A0
#define MQ2_PIN A1
#define DHTTYPE DHT11

// Настройка I2C-подключения к NodeMCU
#define SDA_PIN A4
#define SCL_PIN A5

int  h     = 0;
int  t     = 0;
int  co    = 0;
int  co2   = 0;
int  lpg   = 0;
int  smoke = 0;

// MQ2 и MQ135 работают с разными библиотеками
MQ2 mq2(MQ2_PIN);
MQ135 mq135 = MQ135(MQ135_PIN);
DHT dht(DHT_PIN, DHT11);

void setup() {
  pinMode(DATA_SIGNAL_PIN, OUTPUT);
  pinMode(CONNECTION_SIGNAL_PIN, OUTPUT);
  pinMode(WORK_SIGNAL_PIN, OUTPUT);
  
  
  Serial.begin(9600);
  delay(100);
  dht.begin();
  mq2.begin();

  digitalWrite(WORK_SIGNAL_PIN, HIGH); // Если плата работает - заработает светодиод. Так лучше, чем индикация по питанию, т.к. появляется возможность сигнализировать об ошибках
  
  Wire.begin(8);                /* задаем на шине i2c 8 адрес */
  Wire.onReceive(receiveEvent); /* регистрируем полученное событие */
  Wire.onRequest(requestEvent); /* регистрируем запрошенное событие */
}

void loop() {
  h = (int)dht.readHumidity(); 
  t = (int)dht.readTemperature(); 
  co = (int)mq2.readCO();
  co2 = (int)mq135.getPPM();
  lpg = (int)mq2.readLPG();
  smoke = (int)mq2.readSmoke();
  if (isnan(h) || isnan(t) || isnan(co) || isnan(co2) || isnan(lpg) || isnan(smoke)) {  
      h = 0;
      t = 0;
      co = 0;
      co2 = 0;
      lpg = 0;
      smoke = 0;
  }
  float* values= mq2.read(true);
  Serial.println(h);
  Serial.println(t);
  Serial.println(co);
  Serial.println(co2);
  Serial.println(lpg);
  Serial.println(smoke);
  delay(1000);
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
}
