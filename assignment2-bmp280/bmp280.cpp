#include "bmp280.h"
#include "i2c.h"

#define bmp280_address 0x76
#define bmp280_chipId_reg 0xD0
#define bmp280_rst_reg 0xE0
#define bmp280_press_reg 0xF7
#define bmp280_status_reg 0xF3  
#define bmp280_ctrl_meas_reg 0xF4

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

  uint8_t rst_value = 0xB6;

  i2c_write(devAddress, &reg, 1);
  i2c_write(devAddress, &rst_value, 1);
}

void Bmp280::config() {
  uint32_t* i2c_buf;

  //Mode[1:0] = 11
  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_ctrl_meas_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, *i2c_buf, 3);

  return *i2c_buf;
}

// gets data, but measurement not done yet !!
int Bmp280::getPress() {
  uint32_t* i2c_buf;

  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_press_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, *i2c_buf, 3);

  return *i2c_buf;
}

int Bmp280::getTemp() {}