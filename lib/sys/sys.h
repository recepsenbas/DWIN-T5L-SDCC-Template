/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : sys.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : Header for system-level DGUS RAM and NOR Flash access routines.
 *                Defines prototypes for VP read/write and graph/page control.
 * ----------------------------------------------------------------------------- */
#ifndef __SYS_H__
#define __SYS_H__

#include <stdint.h>
#include "t5l1.h"
#include "config.h"
#include "uart.h"
#include "addresses.h"

void Sys_Init(void);
u16 DGUS_Read_VP(u16 addr);
void DGUS_Write_VP(u16 addr, u16 val);
void DGUS_WriteBytes(u16 addr, const u8 *buf, u16 len);
void DGUS_ReadBytes(u16 addr, u8 *buf, u16 words);
void DGUS_WriteText(u16 addr, const char *text);
u8 DGUS_GetPageID(void);
void DGUS_SetPageID(u8 page_id);
u8 DGUS_NOR_Write(u32 nor_addr, u16 vp_addr, u16 len_bytes);
u8 DGUS_NOR_Read(u32 nor_addr, u16 vp_addr, u16 len_bytes);
void DGUS_GraphPush(u8 channel, u16 value);
void DGUS_GraphClear(u8 channel);
void DGUS_ResetHmi(void);
#endif