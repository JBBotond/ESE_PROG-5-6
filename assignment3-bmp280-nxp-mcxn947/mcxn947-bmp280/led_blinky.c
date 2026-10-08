/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "app.h"
#include "fsl_tsi_v6.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TOUCH_CHANNEL       BOARD_TSI_ELECTRODE_1
#define TOUCH_DELTA         20U

#if defined(TSI0)
#define TOUCH_TSI           TSI0
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint16_t touch_baseline;

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

static bool TouchPad_IsTouched(void)
{
    uint16_t threshold = (touch_baseline > TOUCH_DELTA) ?
                             (touch_baseline - TOUCH_DELTA) : 0U;

    TSI_ClearStatusFlags(TOUCH_TSI, kTSI_EndOfScanFlag | kTSI_OutOfRangeFlag);
    TSI_StartSoftwareTrigger(TOUCH_TSI);
    while ((TSI_GetStatusFlags(TOUCH_TSI) & kTSI_EndOfScanFlag) == 0U)
    {
    }

    return TSI_GetCounter(TOUCH_TSI) < threshold;
}

/*******************************************************************************
 * Code
 ******************************************************************************/
/*! 
 * @brief Main function
 */
int main(void)
{
    bool touch_was_active = false;

    /* Board pin init */
    BOARD_InitHardware();
    TouchPad_Init();

    while (1)
    {
        bool touch_is_active = TouchPad_IsTouched();

        if (touch_is_active && !touch_was_active)
        {
            LED_RED_TOGGLE();
        }
        touch_was_active = touch_is_active;
    }
}
