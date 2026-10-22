#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "MockBus.hpp"
#include "bme280/Bme280.hpp"

using namespace bme280;
using bme280::test::MockBus;

namespace {
// Datasheet example calibration, registers 0x88..0x9F, little-endian.
void loadDatasheetCalibration(MockBus& bus)
{
    bus.load(0x88, {
        0x70, 0x6B,   // T1 = 27504
        0x43, 0x67,   // T2 = 26435
        0x18, 0xFC,   // T3 = -1000
        0x7D, 0x8E,   // P1 = 36477
        0x43, 0xD6,   // P2 = -10685
        0xD0, 0x0B,   // P3 = 3024
        0x27, 0x0B,   // P4 = 2855
        0x8C, 0x00,   // P5 = 140
        0xF9, 0xFF,   // P6 = -7
        0x8C, 0x3C,   // P7 = 15500
        0xF8, 0xC6,   // P8 = -14600
        0x70, 0x17    // P9 = 6000
    });
}
} // namespace

TEST_CASE("getId returns the chip id register")
{
    MockBus bus;
    Bme280 sensor(bus);
    CHECK(sensor.getId() == 0x58);

    bus.reg(MockBus::kRegChipId) = 0x60;   // a real BME280
    CHECK(sensor.getId() == 0x60);
}

TEST_CASE("reset writes 0xB6 to the reset register")
{
    MockBus bus;
    Bme280 sensor(bus);
    sensor.reset();

    REQUIRE(bus.writes().size() == 1);
    CHECK(bus.writes()[0].reg == 0xE0);
    CHECK(bus.writes()[0].value == 0xB6);
}

TEST_CASE("config sets temp x1, pressure x1, normal mode")
{
    MockBus bus;
    Bme280 sensor(bus);
    sensor.config();

    REQUIRE(bus.writes().size() == 1);
    CHECK(bus.writes()[0].reg == 0xF4);
    CHECK(bus.writes()[0].value == 0x27);
}

TEST_CASE("getTemp and getPress assemble the 20-bit raw value")
{
    MockBus bus;
    bus.load(0xFA, {0x7E, 0xED, 0x00});   // 519888
    bus.load(0xF7, {0x65, 0x5A, 0xC0});   // 415148
    Bme280 sensor(bus);

    CHECK(sensor.getTemp() == 519888);
    CHECK(sensor.getPress() == 415148);
}

TEST_CASE("compensation matches datasheet example")
{
    MockBus bus;
    loadDatasheetCalibration(bus);
    Bme280 sensor(bus);
    sensor.getCalibration();

    // Temperature first: it sets t_fine, which pressure depends on.
    CHECK(sensor.convertTemp(519888) == 2508);   // 25.08 degC
    CHECK(sensor.convertPress(415148) == doctest::Approx(100653.27).epsilon(1e-6));
}
