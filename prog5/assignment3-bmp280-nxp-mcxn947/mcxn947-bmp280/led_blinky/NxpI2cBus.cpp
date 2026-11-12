#include "NxpI2cBus.hpp"

#include "i2c.h"
#include <cstdint>

namespace bmp280::platform {

NxpI2cBus::NxpI2cBus(uint8_t address) : address_(address) {}

bool NxpI2cBus::read(uint8_t reg, uint8_t* data, int len)
{
    if (data == nullptr || len <= 0 || len > 255) {
        return false;
    }

    // Same two transactions the original code did by hand: set the
    // register pointer, then read `len` bytes from it.
    uint8_t r = reg;
    if (!i2c_write(address_, &r, 1)) {
        return false;
    }
    return i2c_read(address_, data, static_cast<uint8_t>(len));
}

bool NxpI2cBus::write(uint8_t reg, const uint8_t* data, int len)
{
    // i2c_write() is a single transaction: [start][addr][bytes...][stop].
    // To write a register we need reg + payload in that SAME transaction
    // (that's what the original {bmp280_rst_reg, 0xB6}-style arrays did),
    // so combine them into one buffer before handing it to i2c_write().
    if (data == nullptr || len <= 0 || len > kMaxWriteLen) {
        return false;
    }

    uint8_t buf[1 + kMaxWriteLen];
    buf[0] = reg;
    for (int i = 0; i < len; ++i) {
        buf[i + 1] = data[i];
    }

    return i2c_write(address_, buf, static_cast<uint8_t>(len + 1));
}

} // namespace bmp280::platform
