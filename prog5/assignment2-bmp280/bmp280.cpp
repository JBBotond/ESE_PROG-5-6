#include "bmp280.hpp"
#include <avr/io.h>

namespace bmp280 {

namespace {
constexpr uint8_t kChipIdReg   = 0xD0;
constexpr uint8_t kRstReg      = 0xE0;
constexpr uint8_t kPressReg    = 0xF7;
constexpr uint8_t kCtrlMeasReg = 0xF4;
constexpr uint8_t kTempReg     = 0xFA;
constexpr uint8_t kCalibReg    = 0x88;
} // namespace

uint8_t Bmp280::getId()
{
    uint8_t buf[1];

    // bus_.read() sets the register pointer and reads the chip ID
    // (normally 0x58) in one call.
    bus_.read(kChipIdReg, buf, 1);

    return buf[0];
}

void Bmp280::reset()
{
    const uint8_t value = 0xB6;
    bus_.write(kRstReg, &value, 1);
}

// set to normal mode to ensure continuous data acquisition
void Bmp280::config()
{
    // ctrl_meas: temperature x1, pressure x1, normal mode.
    // Setting only mode (0x03) leaves both oversampling fields at 0 (skipped),
    // so the measurement registers remain at their reset value, 0x80000.
    const uint8_t ctrl_meas = 0b00100111;
    bus_.write(kCtrlMeasReg, &ctrl_meas, 1);
}

uint32_t Bmp280::getPress()
{
    uint8_t buf[3];
    bus_.read(kPressReg, buf, 3);

    // Registers F7..F9 contain a left-aligned 20-bit ADC value.
    return ((uint32_t)buf[0] << 12) |
           ((uint32_t)buf[1] << 4) |
           (buf[2] >> 4);
}

uint32_t Bmp280::getTemp()
{
    uint8_t buf[3];
    bus_.read(kTempReg, buf, 3);

    // Registers FA..FC contain a left-aligned 20-bit ADC value.
    return ((uint32_t)buf[0] << 12) |
           ((uint32_t)buf[1] << 4) |
           (buf[2] >> 4);
}

void Bmp280::getCalibration()
{
    uint8_t buf[24];
    bus_.read(kCalibReg, buf, 24);

    dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
    dig_T2 = (uint16_t)(buf[3] << 8 | buf[2]);
    dig_T3 = (uint16_t)(buf[5] << 8 | buf[4]);

    dig_P1 = (uint16_t)(buf[7] << 8 | buf[6]);
    dig_P2 = (int16_t)(buf[9] << 8 | buf[8]);
    dig_P3 = (int16_t)(buf[11] << 8 | buf[10]);
    dig_P4 = (int16_t)(buf[13] << 8 | buf[12]);
    dig_P5 = (int16_t)(buf[15] << 8 | buf[14]);
    dig_P6 = (int16_t)(buf[17] << 8 | buf[16]);
    dig_P7 = (int16_t)(buf[19] << 8 | buf[18]);
    dig_P8 = (int16_t)(buf[21] << 8 | buf[20]);
    dig_P9 = (int16_t)(buf[23] << 8 | buf[22]);
}

int Bmp280::convertTemp(uint32_t adc_T)
{
    int32_t var1, var2;

    var1 = (((int32_t)((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((int32_t)(adc_T >> 4) - ((int32_t)dig_T1)) *
              ((int32_t)(adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;

    return (t_fine * 5 + 128) >> 8;
}

float Bmp280::convertPress(uint32_t adc_p)
{
    double var1, var2;

    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 +
            ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);

    if (var1 == 0.0) return 0.0f;  // avoid divide-by-zero

    p_fine = 1048576.0 - (double)adc_p;
    p_fine = (p_fine - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p_fine * p_fine / 2147483648.0;
    var2 = p_fine * ((double)dig_P8) / 32768.0;
    p_fine = p_fine + (var1 + var2 + ((double)dig_P7)) / 16.0;

    return p_fine;
}

} // namespace bmp280
