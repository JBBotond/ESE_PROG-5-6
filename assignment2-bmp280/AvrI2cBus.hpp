#pragma once

#include "bus.hpp"
#include <avr/io.h>

namespace bmp280::platform {

/// AVR TWI implementation of Bus, built on i2c.h (Hugo Arends, HAN).
///
/// This is the ONLY file in the whole driver that knows i2c.h exists.
/// Porting bmp280:: to a different MCU means writing one of these against
/// that MCU's I2C driver — nothing under include/bmp280 or src/ changes.
class AvrI2cBus : public bmp280::Bus {
public:
    /// `address` is the 7-bit I2C address (see Bmp280::kDefaultAddress).
    explicit AvrI2cBus(uint8_t address);

    bool read(uint8_t reg, uint8_t* data, int len);
    bool write(uint8_t reg, const uint8_t* data, int len);

private:
    // Largest single register write this driver ever issues (ctrl_meas,
    // reset are both 1 byte). Bump if a future write needs more.
    static constexpr int kMaxWriteLen = 8;

    uint8_t address_;
};

} // namespace bmp280::platform
