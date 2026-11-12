/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
/*${header:end}*/

/*${function:start}*/
void BOARD_InitHardware(void)
{
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_SetClkDiv(kCLOCK_DivTsiClk, 1u);
    CLOCK_AttachClk(kCLK_IN_to_TSI);
    BOARD_InitPins();
    BOARD_BootClockFRO12M();
    LED_RED_INIT(LOGIC_LED_OFF);
}
/*${function:end}*/
