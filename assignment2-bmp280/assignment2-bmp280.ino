#include "bmp280.h"
#include "i2c.h"
#include <avr/io.h>

void setup() {
  Serial.begin(9600);

  // csb pin HIGH for i2c
  // sdo pin LOW for address 0x76
  i2c_master_init();
  Serial.println("Ok");
}

void loop() {
  uint8_t *i2c_buf;
  i2c_read(0xD0, i2c_buf, 8);

  Serial.print(*i2c_buf);
}