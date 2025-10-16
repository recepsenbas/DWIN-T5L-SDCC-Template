/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : rtc.c / rtc.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    Clean-room reimplementation of the DWIN T5L real-time clock interface.
 *    Fully rewritten for SDCC / 8051 architecture with safe I2C bit-banging
 *    and DGUS variable synchronization.
 *
 *    Supports RX8130 and SD2058 RTC chips with legacy timing compatibility.
 *    Preserves functional behavior for DGUS panels without using any DWIN SDK code.
 * ----------------------------------------------------------------------------- */

#ifndef __RTC_H__
#define __RTC_H__

#include <stdint.h>
#include "t5l1.h"
#include "config.h"
#include "timer.h"
#include "sys.h"

extern __bit RTC_Flog;

void Clock(void);

#endif