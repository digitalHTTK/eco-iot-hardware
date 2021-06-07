// Чо надо:
// + DHT11
// + MQ2
// + MQ135
// - Подключение к NodeMCU
// - Светодиоды

// Дебаг-режим, ставим false для отключения
static bool debugFlag = true;

#include <DHT.h>
#include <MQ2.h>
#include <MQ135.h>
#include <Wire.h>

// Конфигурация датчиков и актуаторов
#define DATA_SIGNAL_PIN 3        // Чтобы сигнализировать об успешном (или не очень) сборе данных
#define CONNECTION_SIGNAL_PIN 4  // Чтобы сигнализировать о подключении NodeMCU сначала друг с другом, а затем к серверу с MQTT-брокером
#define WORK_SIGNAL_PIN 5        // Чтобы сигнализировать о работе самого микроконтроллера
#define DHT_PIN 2
#define MQ135_PIN A0
#define MQ2_PIN A1
#define DHTTYPE DHT11

// Настройка I2C-подключения к NodeMCU
#define SDA_PIN A4
#define SCL_PIN A5

// MQ2 и MQ135 работают с разными библиотеками
MQ2 mq2(MQ2_PIN);
MQ135 mq135 = MQ135(MQ135_PIN);
DHT dht(DHT_PIN, DHT11);

void setup() {
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH); // Если плата работает - заработает светодиод. Так лучше, чем индикация по питанию, т.к. появляется возможность сигнализировать об ошибках
  
  dht.begin();
  mq2.begin();
  
  // Инструменты отладки
  if (debugFlag) {
    Serial.begin(9600);
  }
  
  Wire.begin(8);                /* задаем на шине i2c 8 адрес */
  Wire.onReceive(receiveEvent); /* регистрируем полученное событие */
  Wire.onRequest(requestEvent); /* регистрируем запрошенное событие */
}

void loop() {
  float h = dht.readHumidity(); 
  float t = dht.readTemperature(); 
  
  if (debugFlag) {
    Serial.print("А ето LPG: ");
    Serial.println(mq2.readLPG());
    Serial.print("А ето монооксид углерада: ");
    Serial.println(mq2.readCO());
    Serial.print("А ето диоксид углерада: ");
    Serial.println(mq135.getPPM()); 

    if (isnan(h) || isnan(t)) {  
      Serial.println("Ошибка считывания");
    }
    else {
      Serial.print("Влажность: ");
      Serial.print(h);
      Serial.println(" %\t");
      Serial.print("Температура: ");
      Serial.print(t);
      Serial.println(" *C ");  
    }
  }
  delay(1000);
}

// Функция для извлечения любых принимаемых данных от мастера на шину
void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    char c = Wire.read();      
    Serial.print(c);           
  }
  Serial.println();             
}
 
// Функция для извлечения любых отправляемых данных от мастера на шину
void requestEvent() {
  Wire.write("Hello NodeMCU");  /* отправляем по запросу строку "Hello NodeMCU" */
}
