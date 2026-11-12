#include "AvrI2cBus.hpp"

#include "i2c.h"

namespace bmp280::platform {

AvrI2cBus::AvrI2cBus(uint8_t address) : address_(address) {}

bool AvrI2cBus::read(uint8_t reg, uint8_t* data, int len)
{
    // Same two transactions the original code did by hand: set the
    // register pointer, then read `len` bytes from it.
    uint8_t r = reg;
    i2c_write(address_, &r, 1);
    i2c_read(address_, data, static_cast<uint8_t>(len));
    return true; // i2c_write()/i2c_read() have no failure return of their own
}

bool AvrI2cBus::write(uint8_t reg, const uint8_t* data, int len)
{
    // i2c_write() is a single transaction: [start][addr][bytes...][stop].
    // To write a register we need reg + payload in that SAME transaction
    // (that's what the original {bmp280_rst_reg, 0xB6}-style arrays did),
    // so combine them into one buffer before handing it to i2c_write().
    if (len > kMaxWriteLen) {
        return false;
    }

    uint8_t buf[1 + kMaxWriteLen];
    buf[0] = reg;
    for (int i = 0; i < len; ++i) {
        buf[i + 1] = data[i];
    }

    i2c_write(address_, buf, static_cast<uint8_t>(len + 1));
    return true;
}

} // namespace bmp280::platform
