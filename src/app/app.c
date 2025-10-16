/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : app.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    Application layer entry point. This file contains user-defined logic,
 *    DGUS event handling, and system-level coordination functions.
 *    Use this space to implement project-specific behavior and extend
 *    the base template with your own features.
 * ----------------------------------------------------------------------------- */
#include "t5l1.h"
#include "uart.h"
#include "sys.h"
#include "timer.h"
#include "rtc.h"
#include "vp_flags.h"
#include "app_defs.h"

void App_Init(void)
{
    // Initialize application-specific settings
    DGUS_Write_VP(ICON_WORKING_ADDR, working_status);
}
