#include "bmp280.hpp"
#include <cstdint>

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
    uint8_t buf[1] = {0};

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
    uint8_t buf[3] = {0};
    bus_.read(kPressReg, buf, 3);

    // Registers F7..F9 contain a left-aligned 20-bit ADC value.
    return ((uint32_t)buf[0] << 12) |
           ((uint32_t)buf[1] << 4) |
           (buf[2] >> 4);
}

uint32_t Bmp280::getTemp()
{
    uint8_t buf[3] = {0};
    bus_.read(kTempReg, buf, 3);

    // Registers FA..FC contain a left-aligned 20-bit ADC value.
    return ((uint32_t)buf[0] << 12) |
           ((uint32_t)buf[1] << 4) |
           (buf[2] >> 4);
}

void Bmp280::getCalibration()
{
    uint8_t buf[24] = {0};
    // Read each 16-bit coefficient separately. This keeps every transaction
    // within the LPI2C FIFO/adapter's small transfer path and prevents a
    // repeated byte pair from replacing the complete calibration block.
    for (int offset = 0; offset < 24; offset += 2) {
        bus_.read(static_cast<uint8_t>(kCalibReg + offset), &buf[offset], 2);
    }

    dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
    dig_T2 = (int16_t)(buf[3] << 8 | buf[2]);
    dig_T3 = (int16_t)(buf[5] << 8 | buf[4]);

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
    // Bosch BMP280 datasheet, Appendix 1, section 8.2.
    // Return value is temperature in 0.01 degrees Celsius.
    const int32_t adc = static_cast<int32_t>(adc_T);
    int32_t var1 = ((((adc >> 3) - (static_cast<int32_t>(dig_T1) << 1)) *
                     static_cast<int32_t>(dig_T2)) >> 11);
    int32_t var2 = (((((adc >> 4) - static_cast<int32_t>(dig_T1)) *
                      ((adc >> 4) - static_cast<int32_t>(dig_T1))) >> 12) *
                    static_cast<int32_t>(dig_T3)) >> 14;

    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

float Bmp280::convertPress(uint32_t adc_p)
{
    // Bosch BMP280 datasheet, Appendix 1, section 8.2.
    // Bosch returns pressure in unsigned Q24.8 format. Keep the complete
    // calculation signed and 64-bit, then convert Q24.8 pascals to float.
    int64_t var1 = static_cast<int64_t>(t_fine) - 128000;
    int64_t var2 = var1 * var1 * static_cast<int64_t>(dig_P6);
    var2 += (var1 * static_cast<int64_t>(dig_P5)) << 17;
    var2 += static_cast<int64_t>(dig_P4) << 35;
    var1 = ((var1 * var1 * static_cast<int64_t>(dig_P3)) >> 8) +
           ((var1 * static_cast<int64_t>(dig_P2)) << 12);
    var1 = (((static_cast<int64_t>(1) << 47) + var1) *
            static_cast<int64_t>(dig_P1)) >> 33;

    if (var1 == 0) {
        return 0.0F;
    }

    int64_t pressure = 1048576 - static_cast<int64_t>(adc_p);
    pressure = (((pressure << 31) - var2) * 3125) / var1;
    var1 = (static_cast<int64_t>(dig_P9) * (pressure >> 13) *
            (pressure >> 13)) >> 25;
    var2 = (static_cast<int64_t>(dig_P8) * pressure) >> 19;
    pressure = ((pressure + var1 + var2) >> 8) +
               (static_cast<int64_t>(dig_P7) << 4);

    return static_cast<float>(static_cast<uint32_t>(pressure)) / 256.0F;
}

} // namespace bmp280
