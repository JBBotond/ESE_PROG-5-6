#pragma once

// Adapter: makes the low-level Bme280 driver look like an EnvironmentSensor.
// The driver exposes chip operations and ADC conversion separately; the
// station sees only initialise + one complete measurement.

#include "bme280/Bme280.hpp"
#include "station/EnvironmentSensor.hpp"

#include <chrono>
#include <cstdint>
#include <thread>

namespace station {

class Bme280Adapter final : public EnvironmentSensor {
public:
    explicit Bme280Adapter(bme280::Bme280& sensor) : sensor_(sensor) {}

    bme280::Error init() override
    {
        // This implementation supports both the BMP280 (0x58) and BME280
        // (0x60) chip IDs. The driver reports bus failures as an invalid ID.
        const uint8_t id = sensor_.getId();
        if (id != kBmp280ChipId && id != kBme280ChipId) {
            return bme280::Error::WrongChipId;
        }

        sensor_.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        sensor_.getCalibration();
        sensor_.config();

        // Allow the first normal-mode conversion to complete.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        initialised_ = true;
        return bme280::Error::None;
    }

    bme280::Error readForced(bme280::Measurement& out) override
    {
        if (!initialised_) {
            return bme280::Error::NotInitialised;
        }

        // convertTemp() must run first because it updates t_fine, which is
        // needed by convertPress(). This driver does not expose humidity.
        const int temperatureCenti = sensor_.convertTemp(sensor_.getTemp());
        out.temperatureC = static_cast<float>(temperatureCenti) / 100.0F;
        out.pressurePa   = sensor_.convertPress(sensor_.getPress());
        out.humidityPct  = 0.0F;
        return bme280::Error::None;
    }

private:
    static constexpr uint8_t kBmp280ChipId = 0x58;
    static constexpr uint8_t kBme280ChipId = 0x60;

    bme280::Bme280& sensor_;
    bool initialised_ = false;
};

} // namespace station
