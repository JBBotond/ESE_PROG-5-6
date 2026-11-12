cmake_minimum_required(VERSION 3.16)
project(mqtt_station LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# --- libmosquitto ------------------------------------------------------------
find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h)
find_library(MOSQUITTO_LIBRARY mosquitto)
if(NOT MOSQUITTO_INCLUDE_DIR OR NOT MOSQUITTO_LIBRARY)
    message(FATAL_ERROR "libmosquitto not found: apt install libmosquitto-dev / brew install mosquitto")
endif()

# --- core + platform ---------------------------------------------------------
# The week-4 project is a sibling of this directory:
#   pi/
#   ├── mqtt_station/
#   └── rpi_platform/
get_filename_component(RPI_PLATFORM_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../rpi_platform" REALPATH)

# On Linux rpi_platform brings in both the bme280 core and LinuxI2cBus.
# Elsewhere only the bme280 core is built and the station runs with --fake.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # rpi_platform's default sensor path is relative to an older layout. Set
    # the path explicitly so its nested add_subdirectory() finds this tree.
    set(SENSOR_LIB_DIR "${RPI_PLATFORM_DIR}/bme280" CACHE PATH
        "Sensor library root" FORCE)
    add_subdirectory("${RPI_PLATFORM_DIR}" "${CMAKE_BINARY_DIR}/rpi_platform")
    set(STATION_HAS_HARDWARE ON)
else()
    add_subdirectory("${RPI_PLATFORM_DIR}/bme280" "${CMAKE_BINARY_DIR}/sensor_lib")
    set(STATION_HAS_HARDWARE OFF)
endif()

# --- station library: sampler + publishers, no platform code -------------------
add_library(station STATIC src/Sampler.cpp src/MqttPublisher.cpp)
target_include_directories(station PUBLIC include PRIVATE ${MOSQUITTO_INCLUDE_DIR})
target_link_libraries(station PUBLIC bme280 Threads::Threads PRIVATE ${MOSQUITTO_LIBRARY})
target_compile_options(station PRIVATE -Wall -Wextra -Wpedantic)

# --- executable ----------------------------------------------------------------
add_executable(station_main examples/station_main.cpp)
target_link_libraries(station_main PRIVATE station)
if(STATION_HAS_HARDWARE)
    target_link_libraries(station_main PRIVATE rpi_platform)
    target_compile_definitions(station_main PRIVATE STATION_HAS_HARDWARE)
endif()
