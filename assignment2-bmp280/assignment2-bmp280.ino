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

  while(1) {
    //uint32_t newPress = bmp280.getPress();
    //uint32_t newTemp = bmp280.getTemp();
    bmp280.getCalibration();

    //Serial.println(newTemp);
    
  }
}