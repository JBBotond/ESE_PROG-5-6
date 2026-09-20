#include "bmp280.h"
#include "i2c.h"
#include <avr/io.h>

int main(void) {
  Serial.begin(9600);
  i2c_master_init();
  Serial.println("Ok");

  Bmp280 bmp280;

  bmp280.reset();

  uint8_t myId = bmp280.getId();
  Serial.println(myId, HEX);
  bmp280.config();
  bmp280.getCalibration();



  while(1) {
    //uint32_t newPress = bmp280.getPress();
    uint32_t rawTemp = bmp280.getTemp();
    float temp = bmp280.convertTemp(rawTemp);
    temp /= 100;
    
    Serial.print("Temperature in C: ");
    Serial.println(temp);

    uint32_t rawPress = bmp280.getPress();
    float press = bmp280.convertPress(rawPress);
    press /= 100;

    Serial.print("Pressure in hPa: ");
    Serial.println(press);
    
  }
}