/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : crc16.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : CRC-16/MODBUS implementation (poly 0xA001, init 0xFFFF).
 *                Table-free to minimize ROM usage and avoid third-party code.
 * ----------------------------------------------------------------------------- */

#include "crc16.h"

u16 crc16table(u8 *ptr, u16 len)
{
	u16 crc = 0xFFFF; // initial value per MODBUS spec
	while (len--)
	{
		crc ^= (u16)(*ptr++);
		for (u8 i = 0; i < 8; i++)
		{
			if (crc & 0x0001)
			{
				crc = (crc >> 1) ^ 0xA001; // reflected poly
			}
			else
			{
				crc >>= 1;
			}
		}
	}
	return crc; // LSB first when transmitted
}
