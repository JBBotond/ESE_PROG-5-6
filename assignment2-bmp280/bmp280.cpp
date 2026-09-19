#include "bmp280.h"
#include "i2c.h"

#define bmp280_address 0x76
#define bmp280_chipId_reg 0xD0
#define bmp280_rst_reg 0xE0

uint8_t Bmp280::getId() {
  uint8_t i2c_buf[1];

  // Set the BMP280's register pointer, then read its chip ID (normally 0x58).
  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_chipId_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, i2c_buf, 1);

  return i2c_buf[0];
}

void Bmp280::reset() {
  uint8_t i2c_buf[1];

  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_rst_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, i2c_buf, 1);
}