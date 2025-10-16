/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : app_defs.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description :
 *    Application-level shared state definitions.
 *    Keep variables here minimal and well-documented; prefer function
 *    parameters or local scope when possible.
 * ----------------------------------------------------------------------------- */
#ifndef __APP_DEFS_H__
#define __APP_DEFS_H__
#include "t5l1.h"

/* Definitions */
#define ICON_WORKING_ADDR 0x8001

/* Variables */
extern __data u8 working_status;

#endif