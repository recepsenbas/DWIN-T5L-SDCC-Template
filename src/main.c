/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : main.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : Main entry point for DWIN T5L firmware using SDCC.
 *                Initializes system, UART, and DGUS handling loop.
 * ----------------------------------------------------------------------------- */

#include "t5l1.h"
#include "uart.h"
#include "sys.h"
#include "timer.h"
#include "rtc.h"
#include "vp_flags.h"
#include "uart_flags.h"
#include "app_defs.h"
#include "app.c"

void main(void)
{
    Sys_Init();
    Uart_Init();
    RTC_Service();
    App_Init();

    while (1)
    {
        /* Monitor DGUS register and send updates */
        DGUS_MonitorAndSendUpdates();

        /* Check for incoming UART data and process it */
        DGUS_ProcessAllUarts();
    }
}
