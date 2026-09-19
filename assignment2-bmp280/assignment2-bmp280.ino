#include "bmp280.h"
#include "i2c.h"
#include <avr/io.h>

int main(void) {
  Serial.begin(9600);
  i2c_master_init();
  Serial.println("Ok");

  Bmp280 bmp280;

  bmp280.reset();

  while(1) {
    //uint8_t myId = bmp280.getId();

    //int press = bmp280.getPress();

    //Serial.println(press, HEX);
  }
}