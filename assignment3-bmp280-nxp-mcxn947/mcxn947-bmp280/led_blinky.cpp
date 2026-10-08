/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_tsi_v6.h"

#include "NxpI2cBus.hpp"
#include "bmp280.hpp"
#include "i2c.h"

#include <cstdint>

extern "C" {
#include "app.h"
}

#define TOUCH_CHANNEL       BOARD_TSI_ELECTRODE_1
#define TOUCH_DELTA         20U

#if defined(TSI0)
#define TOUCH_TSI           TSI0
#endif

static uint16_t touch_baseline;

static void DebugUart_InitPins(void)
{
    const port_pin_config_t uart_pin_config = {
        kPORT_PullDisable,
        kPORT_LowPullResistor,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt2,
        kPORT_InputBufferEnable,
        kPORT_InputNormal,
        kPORT_UnlockRegister};

    // LPUART4 is connected to the FRDM-MCXN947 MCU-Link virtual COM port.
    PORT_SetPinConfig(PORT1, 8U, &uart_pin_config); // FC4_P0 / UART TX
    PORT_SetPinConfig(PORT1, 9U, &uart_pin_config); // FC4_P1 / UART RX
}

static void TouchPad_Init(void)
{
    tsi_selfCap_config_t config;
    tsi_calibration_data_t calibration;

    TSI_GetSelfCapModeDefaultConfig(&config);
    TSI_InitSelfCapMode(TOUCH_TSI, &config);
    TSI_EnableModule(TOUCH_TSI, true);

    TSI_SelfCapCalibrate(TOUCH_TSI, &calibration);
    touch_baseline = calibration.calibratedData[TOUCH_CHANNEL];

    TSI_EnableHardwareTriggerScan(TOUCH_TSI, false);
    TSI_DisableInterrupts(TOUCH_TSI, kTSI_EndOfScanInterruptEnable);
    TSI_SetSelfCapMeasuredChannel(TOUCH_TSI, TOUCH_CHANNEL);
}

int main(void)
{

    BOARD_InitHardware();
    DebugUart_InitPins();
    BOARD_InitDebugConsole();
    TouchPad_Init();

    i2c_master_init();
    bmp280::platform::NxpI2cBus bus(bmp280::Bmp280::kDefaultAddress);
    bmp280::Bmp280 sensor(bus);

    sensor.reset();
    // Bosch status.F3[0] (im_update) stays high while NVM trim data is being
    // copied into the calibration image registers.
    uint8_t status = 0x01U;
    for (unsigned int attempt = 0; attempt < 100U && (status & 0x01U) != 0U; ++attempt)
    {
        SDK_DelayAtLeastUs(1000U, SystemCoreClock);
        bus.read(0xF3U, &status, 1);
    }
    DbgConsole_Printf("BMP280 ID: 0x%02x\r\n", sensor.getId());
    // Read the NVM trim values before starting measurements. The BMP280
    // copies NVM into its calibration image registers during reset and before
    // conversions; ctrl_meas=0x27 starts normal-mode conversions.
    sensor.getCalibration();
    sensor.config();
    SDK_DelayAtLeastUs(100000U, SystemCoreClock);

    while (1)
    {
        float temperature_centi_degrees = sensor.convertTemp((uint32_t)sensor.getTemp()) /100;
        DbgConsole_Printf("Temperature: %.2f C\r\n",
                          temperature_centi_degrees);


        float pressure_centi_degrees = sensor.convertPress((uint32_t)sensor.getPress()) /100;

        DbgConsole_Printf("Pressure: %.2f hPa\r\n",
                          pressure_centi_degrees);
        
        SDK_DelayAtLeastUs(1000000U, SystemCoreClock);
    }
}
