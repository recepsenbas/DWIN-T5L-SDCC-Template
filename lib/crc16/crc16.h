/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : crc16.h
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : CRC-16/MODBUS interface (0xA001 poly, init 0xFFFF).
 *                Clean-room implementation for portability and licensing clarity.
 * ----------------------------------------------------------------------------- */

#ifndef CRC16_H
#define CRC16_H

#include "t5l1.h" // for u8/u16 typedefs

// Computes CRC-16/MODBUS over the given buffer.
// Polynomial: 0xA001 (LSB-first), Initial value: 0xFFFF
u16 crc16table(u8 *ptr, u16 len);

#endif /* CRC16_H */