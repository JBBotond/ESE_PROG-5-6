#pragma once

#include "bus.hpp"
#include <avr/io.h>

namespace bmp280 {

/// C++ driver for the Bosch BMP280 pressure/temperature sensor.
///
/// SRP: this class only configures/reads the sensor and runs the
/// compensation maths. Byte transport lives in Bus, so this file has no
/// idea whether it's talking over AVR TWI, an STM32 HAL, or a mock in a
/// test — porting to a new MCU means writing a new Bus, not touching this.
class Bmp280 {
public:
    /// BMP280 default I2C address (SDO pin tied low). 0x77 if SDO is high.
    static constexpr uint8_t kDefaultAddress = 0x76;

    /// Does not touch hardware; call reset()/config() for that.
    explicit Bmp280(Bus& bus) : bus_(bus) {}

    // Non-copyable: holds a reference back to its bus.
    Bmp280(const Bmp280&)            = delete;
    Bmp280& operator=(const Bmp280&) = delete;

    uint8_t getId();
    void reset();

    // Set to normal mode, temperature x1, pressure x1 (continuous acquisition).
    void config();

    // Returns the uncompensated 20-bit ADC value. Calibration data must be
    // applied separately via convertPress()/convertTemp() (see getCalibration()).
    uint32_t getPress();
    uint32_t getTemp();

    float convertPress(uint32_t adc_p);
    int   convertTemp(uint32_t adc_t);

    void getCalibration();

private:
    Bus& bus_;

    uint16_t dig_T1, dig_T2, dig_T3;
    uint16_t dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint32_t t_fine, p_fine;
};

} // namespace bmp280
