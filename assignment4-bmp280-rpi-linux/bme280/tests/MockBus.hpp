#pragma once

#include "bme280/Bus.hpp"

#include <array>
#include <cstring>
#include <initializer_list>
#include <vector>

namespace bme280::test {

/// Fake sensor: a 256-byte register map plus a log of what was written.
class MockBus : public Bus {
public:
    static constexpr uint8_t kRegChipId     = 0xD0;
    static constexpr uint8_t kChipIdBmp280  = 0x58;

    MockBus() { regs_[kRegChipId] = kChipIdBmp280; }

    bool read(uint8_t reg, uint8_t* data, size_t len) override
    {
        if (failNextRead_) { failNextRead_ = false; return false; }
        if (reg + len > regs_.size()) { return false; }
        std::memcpy(data, &regs_[reg], len);
        return true;
    }

    bool write(uint8_t reg, const uint8_t* data, size_t len) override
    {
        if (failNextWrite_) { failNextWrite_ = false; return false; }
        if (reg + len > regs_.size()) { return false; }
        for (size_t i = 0; i < len; ++i) {
            regs_[reg + i] = data[i];
            writes_.push_back({static_cast<uint8_t>(reg + i), data[i]});
        }
        return true;
    }

    void delayUs(uint32_t us) override { delayedUs_ += us; }

    // --- test helpers -----------------------------------------------------
    struct Write { uint8_t reg; uint8_t value; };

    uint8_t& reg(uint8_t addr) { return regs_[addr]; }

    /// Pre-load consecutive registers without logging them as driver writes.
    void load(uint8_t start, std::initializer_list<uint8_t> bytes)
    {
        size_t i = 0;
        for (uint8_t b : bytes) { regs_[start + i++] = b; }
    }

    const std::vector<Write>& writes() const { return writes_; }
    uint32_t delayedUs() const { return delayedUs_; }
    void failNextRead()  { failNextRead_ = true; }
    void failNextWrite() { failNextWrite_ = true; }

private:
    std::array<uint8_t, 256> regs_{};
    std::vector<Write> writes_;
    uint32_t delayedUs_ = 0;
    bool failNextRead_  = false;
    bool failNextWrite_ = false;
};

} // namespace bme280::test
