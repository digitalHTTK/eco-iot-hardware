#include <Wire.h>

void setup() {
 Serial.begin(115200); /* открываем серийный порт для дебаггинга */
 Wire.begin(D1, D2); /* задаем i2c мост через контакты SDA=D1 и SCL=D2 на NodeMCU */
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
 delay(1000);
}
