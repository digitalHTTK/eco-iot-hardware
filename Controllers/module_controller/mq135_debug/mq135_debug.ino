#include <MQ135.h> 

#define analogPin A0 
MQ135 gasSensor = MQ135(analogPin); // инициализация объекта датчика

void setup() {
  Serial.begin(9600); 
  delay(1000); 
}

void loop() {
  float ppm = gasSensor.getPPM(); 
  Serial.print("PPM: ");
  Serial.println(ppm);

  Serial.print("R0: ");
  float rzero = gasSensor.getRZero();
  Serial.println(rzero);
  
  delay(1000); 
}
