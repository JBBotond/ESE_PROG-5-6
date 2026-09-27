#pragma once

#include <avr/io.h>

namespace bmp280 {

/// Minimal byte-transport interface the sensor needs (DIP: the driver
/// depends on this abstraction, never on i2c.h/Wire/HAL directly).
/// Implementations: AvrI2cBus (platform/), anything else you port to later.
class Bus {
public:
    virtual ~Bus() = default;

    /// Read `len` bytes starting at register `reg`. Return true on success.
    virtual bool read(uint8_t reg, uint8_t* data, int len) = 0;

    /// Write `len` bytes starting at register `reg`. Return true on success.
    virtual bool write(uint8_t reg, const uint8_t* data, int len) = 0;
};

} // namespace bmp280
