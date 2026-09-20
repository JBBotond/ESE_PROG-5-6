#include "bmp280.h"
#include "i2c.h"

#define bmp280_address 0x76
#define bmp280_chipId_reg 0xD0
#define bmp280_rst_reg 0xE0
#define bmp280_press_reg 0xF7
#define bmp280_status_reg 0xF3  
#define bmp280_ctrl_meas_reg 0xF4
#define bmp280_temp_reg 0xFA
#define bmp280_calib_reg 0x88

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
  uint8_t reg[] = {bmp280_rst_reg, 0xB6};

  i2c_write(devAddress, reg, 2);
}

// set to normal mode to ensure continous data acquisition 
void Bmp280::config() {

  // ctrl_meas: temperature x1, pressure x1, normal mode.
  // Setting only mode (0x03) leaves both oversampling fields at 0 (skipped),
  // so the measurement registers remain at their reset value, 0x80000.
  const uint8_t ctrl_meas = 0b00100111;

  const uint8_t devAddress = bmp280_address;
  uint8_t reg[] = {bmp280_ctrl_meas_reg, ctrl_meas};

  i2c_write(devAddress, reg, 2);
}

uint32_t Bmp280::getPress() {
  uint8_t i2c_buf[3];

  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_press_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, i2c_buf, 3);

  // Registers F7..F9 contain a left-aligned 20-bit ADC value.
  return ((uint32_t)i2c_buf[0] << 12) |
         ((uint32_t)i2c_buf[1] << 4) |
         (i2c_buf[2] >> 4);
}

uint32_t Bmp280::getTemp() {
  uint8_t i2c_buf[3];

  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_temp_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, i2c_buf, 3);

  // Registers F7..F9 contain a left-aligned 20-bit ADC value.
  return ((uint32_t)i2c_buf[0] << 12) |
         ((uint32_t)i2c_buf[1] << 4) |
         (i2c_buf[2] >> 4);
}

void Bmp280::getCalibration() {
    uint8_t i2c_buf[24];

  const uint8_t devAddress = bmp280_address;
  uint8_t reg = bmp280_calib_reg;

  i2c_write(devAddress, &reg, 1);
  i2c_read(devAddress, i2c_buf, 24);

  dig_T1 = (uint16_t)(i2c_buf[1] << 8 | i2c_buf[0]);
  dig_T2 = (uint16_t)(i2c_buf[3] << 8 | i2c_buf[2]);
  dig_T3 = (uint16_t)(i2c_buf[5] << 8 | i2c_buf[4]);

  dig_P1 = (uint16_t)(i2c_buf[7] << 8 | i2c_buf[6]);
  dig_P2 = (int16_t)(i2c_buf[9] << 8 | i2c_buf[8]);
  dig_P3 = (int16_t)(i2c_buf[11] << 8 | i2c_buf[10]);
  dig_P4 = (int16_t)(i2c_buf[13] << 8 | i2c_buf[12]);
  dig_P5 = (int16_t)(i2c_buf[15] << 8 | i2c_buf[14]);
  dig_P6 = (int16_t)(i2c_buf[17] << 8 | i2c_buf[16]);
  dig_P7 = (int16_t)(i2c_buf[19] << 8 | i2c_buf[18]);
  dig_P8 = (int16_t)(i2c_buf[21] << 8 | i2c_buf[20]);
  dig_P9 = (int16_t)(i2c_buf[23] << 8 | i2c_buf[22]);
}

int Bmp280::convertTemp(uint32_t adc_T) {
  int32_t var1, var2;

  var1 = (((int32_t)((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
  var2 = (((((int32_t)(adc_T >> 4) - ((int32_t)dig_T1)) *
            ((int32_t)(adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
          ((int32_t)dig_T3)) >> 14;

  t_fine = var1 + var2;

  return (t_fine * 5 + 128) >> 8;
}
