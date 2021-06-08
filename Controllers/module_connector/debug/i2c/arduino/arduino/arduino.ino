#include <Wire.h>
 
void setup() {
 Wire.begin(8);                /* задаем на шине i2c 8 адрес */
 Wire.onReceive(receiveEvent); /* регистрируем полученное событие */
 Wire.onRequest(requestEvent); /* регистрируем запрошенное событие */
 Serial.begin(9600);           /* открываем серийный порт для дебаггинга */
}
 
void loop() {
 delay(100);
}
 
// Функция для извлечения любых принимаемых данных от мастера на шину
void receiveEvent(int howMany) {
 while (0 <Wire.available()) {
    char c = Wire.read();      /* получаем байт как символ*/
    Serial.print(c);           /* выводим символ в серийный порт */
  }
 Serial.println();             /* переходим на новую строку */
}
 
// Функция для извлечения любых отправляемых данных от мастера на шину
void requestEvent() {
 Wire.write("Hello NodeMCU");  /* отправляем по запросу строку "Hello NodeMCU" */
}
