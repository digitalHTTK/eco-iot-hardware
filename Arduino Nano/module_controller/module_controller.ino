#include <DHT.h>

#define LED_PIN 3
#define DHT_PIN 2
#define MQ135_PIN 14
#define MQ2_PIN 15

#define SDA_PIN 18
#define SCL_PIN 19

#define DHTTYPE DHT11

DHT dht(DHT_PIN, DHT11);
bool flag = false;

void setup() {
  pinMode(3, OUTPUT);
  dht.begin();

  // Отладочный монитор
  Serial.begin(9600);
}

void loop() {
  delay(2000); // 2 секунды задержки
  if (flag) {
    digitalWrite(LED_PIN, LOW);
    flag = false;
  }
  else {
    digitalWrite(LED_PIN, HIGH);
    flag = true;
  }

  float h = dht.readHumidity(); //Измеряем влажность
  float t = dht.readTemperature(); //Измеряем температуру
  if (isnan(h) || isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
    Serial.println("Ошибка считывания");
    return;
  }
  Serial.print("Влажность: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Температура: ");
  Serial.print(t);
  Serial.println(" *C "); //Вывод показателей на экран

  
}
