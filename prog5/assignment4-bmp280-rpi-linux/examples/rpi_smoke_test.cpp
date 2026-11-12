// Smoke test for the sensor library on a Raspberry Pi.
//
//   i2cdetect -y 1            # sensor should show up at 0x76 or 0x77
//   ./rpi_smoke_test [addr]   # default 0x76
//
// Prints one measurement per second until Ctrl-C.

#include "bme280/Bme280.hpp"
#include "rpi/LinuxI2cBus.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace {
std::atomic<bool> keepRunning{true};
void onSigint(int) { keepRunning = false; }

constexpr uint8_t kBmp280ChipId = 0x58;
constexpr uint8_t kBme280ChipId = 0x60;
} // namespace

int main(int argc, char** argv)
{
    const auto address = static_cast<uint8_t>(argc > 1 ? std::strtoul(argv[1], nullptr, 0) : 0x76);
    std::signal(SIGINT, onSigint);

    // --- composition root: the only place that knows about Linux -----------
    rpi::LinuxI2cBus bus("/dev/i2c-1", address);
    if (!bus.isOpen()) {
        std::fprintf(stderr, "cannot open /dev/i2c-1 (is I2C enabled in raspi-config? are you in group i2c?)\n");
        return 1;
    }

    bme280::Bme280 sensor(bus);

    // --- identify the chip ---------------------------------------------------
    const uint8_t id = sensor.getId();
    if (id == kBme280ChipId) {
        std::printf("chip id 0x60: real BME280 (humidity is not read by this driver)\n");
    } else if (id != kBmp280ChipId) {
        std::fprintf(stderr, "unexpected chip id 0x%02X at address 0x%02X (expected 0x58 or 0x60)\n", id, address);
        return 2;
    }

    // --- bring-up ------------------------------------------------------------
    sensor.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));    // reset takes ~2 ms
    sensor.getCalibration();
    sensor.config();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // first normal-mode conversion
    std::printf("sensor ready at 0x%02X\n", address);

    // --- main loop -----------------------------------------------------------
    while (keepRunning) {
        // convertTemp() must run first: it sets t_fine, which convertPress() uses.
        const int   t_centi = sensor.convertTemp(sensor.getTemp());
        const float p_pa    = sensor.convertPress(sensor.getPress());

        std::printf("T = %6.2f C   p = %9.1f Pa\n", t_centi / 100.0, p_pa);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::printf("\nbye\n");
    return 0;   // ~Bme280, then ~LinuxI2cBus closes /dev/i2c-1: RAII
}
