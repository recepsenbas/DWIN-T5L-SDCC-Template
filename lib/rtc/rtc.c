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

#include "rtc.h"

/* ========= Local configuration ================================================= */

/* RX8130 / SD2058 I2C addresses (7-bit: 0x32/0x32 -> 8-bit RW: 0x64 / 0x65) */
#define RTC_I2C_ADDR_W 0x64
#define RTC_I2C_ADDR_R 0x65

/* Busy-wait "us" delay tuned for SDCC on T5L1.
   NOTE: These are crude cycle burns; do not change counts unless measured. */
static void rtc_delay_ticks(uint8_t n)
{
	u8 i;
	for (i = 0; i < n; i++)
		;
}

/* Legacy names kept (DWIN originals) */
static void Delayus(void) { rtc_delay_ticks(80); } /* ~50us (as original comment) */
static void delayus(uint8_t t)
{
	while (t--)
		rtc_delay_ticks(50);
}

/* ========= Driver state ======================================================== */

__bit RTC_Flog = 0;						/* one-shot init flag */
static u8 Select_rtc = SELECT_RTC_TYPE; /* runtime selected RTC type */
static u8 __xdata Rtcdata[8] = {0};		/* temporary time buffer */

/* ========= Low-level GPIO/I2C helpers (legacy API preserved) =================== */

/* Switch SDA pin direction by toggling P3MDOUT bit. Implementation relies on
 * T5L1H.h to provide P3MDOUT, RTC_SDA, RTC_SCL symbols. */
void SDA_IN(void) { RTC_DIR_REG &= ~0x08; } /* P3.3 input (open drain)  */
void SDA_OUT(void) { RTC_DIR_REG |= 0x08; } /* P3.3 output (push/pull)  */

/* START condition: SDA 1->0 while SCL high */
void RTC_I2C_Start(void)
{
	SDA_OUT();
	RTC_SDA = 1;
	RTC_SCL = 1;
	Delayus();
	RTC_SDA = 0;
	Delayus();
	RTC_SCL = 0;
	Delayus();
}

/* STOP condition: SDA 0->1 while SCL high */
void RTC_I2C_Stop(void)
{
	SDA_OUT();
	RTC_SDA = 0;
	RTC_SCL = 1;
	Delayus();
	RTC_SDA = 1;
	Delayus();
	SDA_IN();
}

/* Master ACK (drive SDA low during SCL pulse) */
void RTC_I2C_Mack(void)
{
	SDA_OUT();
	RTC_SDA = 0;
	Delayus();
	RTC_SCL = 1;
	Delayus();
	RTC_SCL = 0;
	Delayus();
}

/* Master NACK (drive SDA high during SCL pulse) */
void RTC_I2C_Mnak(void)
{
	SDA_OUT();
	RTC_SDA = 1;
	Delayus();
	RTC_SCL = 1;
	Delayus();
	RTC_SCL = 0;
	Delayus();
}

/* Wait for slave ACK after a written byte (sample SDA low while SCL high) */
void RTC_I2C_Cack(void)
{
	u8 i;
	SDA_IN();
	RTC_SDA = 1;
	Delayus();
	RTC_SCL = 1;
	Delayus();
	for (i = 0; i < 50; i++)
	{ /* timeout guard to avoid deadlock */
		if (!RTC_SDA)
			break; /* ACK observed */
		Delayus();
	}
	RTC_SCL = 0;
	Delayus();
	SDA_OUT();
}

/* Write one byte on I2C (MSB first) */
static void i2c_write_byte(u8 dat)
{
	u8 i;
	SDA_OUT();
	for (i = 0; i < 8; i++)
	{
		RTC_SDA = (dat & 0x80) ? 1 : 0;
		dat <<= 1;
		Delayus();
		RTC_SCL = 1;
		Delayus();
		RTC_SCL = 0;
		Delayus();
	}
	RTC_I2C_Cack();
}

/* Read one byte on I2C (MSB first) */
static u8 i2c_read_byte(void)
{
	u8 i, dat = 0;
	SDA_IN();
	for (i = 0; i < 8; i++)
	{
		Delayus();
		RTC_SCL = 1;
		Delayus();
		dat <<= 1;
		if (RTC_SDA)
			dat |= 0x01;
		RTC_SCL = 0;
		Delayus();
	}
	return dat;
}

/* ========= RX8130 backend ====================================================== */

/**
 * @brief Compute weekday (0..6) for given date (year as 0..99 → 2000+year).
 *        Matches original algorithm; kept for behavioral compatibility.
 */
static u8 RTC_Get_Week(u8 years, u8 month, u8 day)
{
	u8 weak;
	u16 year = (u16)years + 2000;

	if (month == 1 || month == 2)
	{
		month += 12;
		year--;
	}

	if ((year < 1752) || (year == 1752 && month < 9) || (year == 1752 && month == 9 && day < 3))
	{
		weak = (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 + 6) % 7;
	}
	else
	{
		weak = (day + 1 + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
	}
	return weak;
}

/**
 * @brief Read current time from RX8130 and mirror to DGUS VPs 0x0010..0x0013.
 *        Call periodically (handled by RTC_Service()).
 */
static void RTC_ReadAndSync(void)
{
	u8 i, N, M;

	RTC_I2C_Start();
	i2c_write_byte(RTC_I2C_ADDR_W);
	i2c_write_byte(0x10); /* register base */
	RTC_I2C_Stop();

	RTC_I2C_Start();
	i2c_write_byte(RTC_I2C_ADDR_R);
	for (i = 6; i > 0; i--)
	{
		Rtcdata[i] = i2c_read_byte();
		RTC_I2C_Mack();
	}
	Rtcdata[0] = i2c_read_byte();
	RTC_I2C_Mnak();
	RTC_I2C_Stop();

	/* BCD to decimal */
	for (i = 0; i < 3; i++)
	{
		N = Rtcdata[i] / 16;
		M = Rtcdata[i] % 16;
		Rtcdata[i] = N * 10 + M;
	}
	for (i = 4; i < 7; i++)
	{
		N = Rtcdata[i] / 16;
		M = Rtcdata[i] % 16;
		Rtcdata[i] = N * 10 + M;
	}

	Rtcdata[3] = RTC_Get_Week(Rtcdata[0], Rtcdata[1], Rtcdata[2]);

	/* Mirror to DGUS variable VPs (Year/Month, Day/Week, Hour/Minute, Second/Reserved) */
	DGUS_Write_VP(0x0010, (Rtcdata[0] << 8) + Rtcdata[1]);
	DGUS_Write_VP(0x0011, (Rtcdata[2] << 8) + Rtcdata[3]);
	DGUS_Write_VP(0x0012, (Rtcdata[4] << 8) + Rtcdata[5]);
	DGUS_Write_VP(0x0013, (Rtcdata[6] << 8) + Rtcdata[7]);
}

/**
 * @brief Initialize RX8130 after power loss if needed, and set a default time.
 */
static void RTC_InitDefault(void)
{
	u8 i;

	RTC_I2C_Start();
	i2c_write_byte(RTC_I2C_ADDR_W);
	i2c_write_byte(0x1D);
	RTC_I2C_Stop();

	RTC_I2C_Start();
	i2c_write_byte(RTC_I2C_ADDR_R);
	i = i2c_read_byte(); /* 0x1D content */
	RTC_I2C_Mack();
	(void)i2c_read_byte(); /* dummy */
	RTC_I2C_Mnak();
	RTC_I2C_Stop();

	if ((i & 0x02) == 0x02)
	{
		/* Reconfigure default time */
		RTC_I2C_Start(); /* 30 = 00 */
		i2c_write_byte(RTC_I2C_ADDR_W);
		i2c_write_byte(0x30);
		i2c_write_byte(0x00);
		RTC_I2C_Stop();

		RTC_I2C_Start(); /* 1C-1F = 48 00 40 10 */
		i2c_write_byte(RTC_I2C_ADDR_W);
		i2c_write_byte(0x1C);
		i2c_write_byte(0x48);
		i2c_write_byte(0x00);
		i2c_write_byte(0x40);
		i2c_write_byte(0x10);
		RTC_I2C_Stop();

		RTC_I2C_Start(); /* 10-16 = default BCD time */
		i2c_write_byte(RTC_I2C_ADDR_W);
		i2c_write_byte(0x10);
		i2c_write_byte(0x00); /* sec */
		i2c_write_byte(0x00); /* min */
		i2c_write_byte(0x00); /* hour */
		i2c_write_byte(0x01); /* week */
		i2c_write_byte(0x01); /* day  */
		i2c_write_byte(0x01); /* mon  */
		i2c_write_byte(0x17); /* year */
		RTC_I2C_Stop();

		RTC_I2C_Start(); /* 1E-1F 00 10 */
		i2c_write_byte(RTC_I2C_ADDR_W);
		i2c_write_byte(0x1E);
		i2c_write_byte(0x00);
		i2c_write_byte(0x10);
		RTC_I2C_Stop();
	}
}

/**
 * @brief Apply time set request coming from DGUS VPs 0x009D..0x009F.
 *        Triggered when VP 0x009C == 0x5AA5.
 */
static void RTC_Set_Time(void)
{
	u16 V1 = DGUS_Read_VP(0x009C); /* 0x5AA5 */
	if (V1 == 0x5AA5)
	{
		u8 N;
		u8 TimeS[7]; /* Y, M, D, W, H, M, S in BCD */
		__bit Flog = 1;

		V1 = DGUS_Read_VP(0x009D); /* Year, Month */
		N = (u8)(V1 >> 8);
		TimeS[0] = ((N / 10) * 16) + (N % 10);
		N = (u8)(V1);
		TimeS[1] = ((N / 10) * 16) + (N % 10);

		V1 = DGUS_Read_VP(0x009E); /* Day, Hour   */
		N = (u8)(V1 >> 8);
		TimeS[2] = ((N / 10) * 16) + (N % 10);
		N = (u8)(V1);
		TimeS[4] = ((N / 10) * 16) + (N % 10);

		V1 = DGUS_Read_VP(0x009F); /* Minute, Sec */
		N = (u8)(V1 >> 8);
		TimeS[5] = ((N / 10) * 16) + (N % 10);
		N = (u8)(V1);
		TimeS[6] = ((N / 10) * 16) + (N % 10);

		DGUS_Write_VP(0x009C, 0); /* clear trigger */

		while (Flog)
		{
			/* Unlock & write sequence (mirrors original) */
			RTC_I2C_Start();
			i2c_write_byte(RTC_I2C_ADDR_W);
			i2c_write_byte(0x30);
			i2c_write_byte(0x00);
			RTC_I2C_Stop();

			RTC_I2C_Start();
			i2c_write_byte(RTC_I2C_ADDR_W);
			i2c_write_byte(0x1C);
			i2c_write_byte(0x48);
			i2c_write_byte(0x00);
			i2c_write_byte(0x40);
			i2c_write_byte(0x10);
			RTC_I2C_Stop();

			RTC_I2C_Start(); /* 10-16: set BCD time */
			i2c_write_byte(RTC_I2C_ADDR_W);
			i2c_write_byte(0x10);
			i2c_write_byte(TimeS[6]); /* sec  */
			i2c_write_byte(TimeS[5]); /* min  */
			i2c_write_byte(TimeS[4]); /* hour */
			i2c_write_byte(TimeS[3]); /* week */
			i2c_write_byte(TimeS[2]); /* day  */
			i2c_write_byte(TimeS[1]); /* mon  */
			i2c_write_byte(TimeS[0]); /* year */
			RTC_I2C_Stop();

			RTC_I2C_Start(); /* 1E-1F: confirm */
			i2c_write_byte(RTC_I2C_ADDR_W);
			i2c_write_byte(0x1E);
			i2c_write_byte(0x00);
			i2c_write_byte(0x10);
			RTC_I2C_Stop();

			Flog = 0;
		}
	}
}

/* ========= SD2058 backend (kept functionally identical) ======================= */

/* Simple helpers kept for compatibility with original names */
static void i2cstart(void) { RTC_I2C_Start(); }
static void i2cstop(void) { RTC_I2C_Stop(); }
static void mack(void) { RTC_I2C_Mack(); }
static void mnak(void) { RTC_I2C_Mnak(); }
static void cack(void) { RTC_I2C_Cack(); }
static void i2cbw(u8 d) { i2c_write_byte(d); }
static u8 i2cbr(void) { return i2c_read_byte(); }

/* Check whether SD2058 has lost power; if so, initialize with default time. */
static void RTC_InitDefault_1(void)
{
	u8 dat1, dat2;

	/* Read status at 0x0F.. */
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x0F);
	i2cstart();
	i2cbw(RTC_I2C_ADDR_R);
	dat2 = i2cbr();
	mack();
	dat1 = i2cbr();
	mnak();
	i2cstop();

	if (dat2 & 0x01)
	{
		if (dat2 & 0x84)
		{ /* clear WRTC2/WRTC3 if set */
			dat2 &= (u8)~0x84;
			i2cstart();
			i2cbw(RTC_I2C_ADDR_W);
			i2cbw(0x0F);
			i2cbw(dat2);
			i2cstop();
		}
		if (dat1 & 0x80)
		{ /* clear WRTC1 if set */
			dat1 &= (u8)~0x80;
			i2cstart();
			i2cbw(RTC_I2C_ADDR_W);
			i2cbw(0x10);
			i2cbw(dat1);
			i2cstop();
		}

		/* write enable */
		dat1 |= 0x80;
		i2cstart();
		i2cbw(RTC_I2C_ADDR_W);
		i2cbw(0x10);
		i2cbw(dat1);
		i2cstop();
		dat2 |= 0x84;
		i2cstart();
		i2cbw(RTC_I2C_ADDR_W);
		i2cbw(0x0F);
		i2cbw(dat2);
		i2cstop();

		/* Default time 2021-01-01 Friday 00:00:00, 24h mode */
		i2cstart();
		i2cbw(RTC_I2C_ADDR_W);
		i2cbw(0x00);
		i2cbw(0x00); /* sec  */
		i2cbw(0x00); /* min  */
		i2cbw(0x80); /* hour (24h flag) */
		i2cbw(0x05); /* week */
		i2cbw(0x01); /* day  */
		i2cbw(0x01); /* mon  */
		i2cbw(0x21); /* year */
		i2cstop();

		/* write disable */
		dat2 &= (u8)~0x84;
		dat1 &= (u8)~0x80;
		i2cstart();
		i2cbw(RTC_I2C_ADDR_W);
		i2cbw(0x10);
		i2cbw(dat1);
		i2cstop();
		i2cstart();
		i2cbw(RTC_I2C_ADDR_W);
		i2cbw(0x0F);
		i2cbw(dat2);
		i2cstop();
	}
}

/* Convert decimal to BCD */
static u8 BCD(u8 dat) { return (u8)(((dat / 10) << 4) | (dat % 10)); }

/* Simple weekday helper (kept from original) */
static u8 RTC_WeekdayCalc(u8 year, u8 month, u8 day)
{
	u16 tmp, mon, y;
	u8 week;

	if (month == 1 || month == 2)
	{
		mon = month + 12;
		y = year - 1;
	}
	else
	{
		mon = month;
		y = year;
	}

	tmp = y + (y / 4) + (((mon + 1) * 26) / 10) + day - 36;
	week = (u8)(tmp % 7);
	return week;
}

/* SD2058: apply time configuration passed in BCD array prtc_set[0..6]. */
static void rtc_config(u8 *prtc_set)
{
	u8 dat, dat1;

	/* Read status */
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x0F);
	i2cstart();
	i2cbw(RTC_I2C_ADDR_R);
	dat = i2cbr();
	mack();
	dat1 = i2cbr();
	mnak();
	i2cstop();

	/* write enable */
	dat1 |= 0x80;
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x10);
	i2cbw(dat1);
	i2cstop();
	dat |= 0x84;
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x0F);
	i2cbw(dat);
	i2cstop();

	/* write time */
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x00);
	i2cbw(prtc_set[6]); /* sec  */
	i2cbw(prtc_set[5]); /* min  */
	i2cbw(prtc_set[4]); /* hour */
	i2cbw(prtc_set[3]); /* week */
	i2cbw(prtc_set[2]); /* day  */
	i2cbw(prtc_set[1]); /* mon  */
	i2cbw(prtc_set[0]); /* year */
	i2cstop();

	/* write disable */
	dat &= (u8)~0x84;
	dat1 &= (u8)~0x80;
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x10);
	i2cbw(dat1);
	i2cstop();
	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x0F);
	i2cbw(dat);
	i2cstop();
}

/* SD2058: read time, convert and mirror to DGUS VPs (0x0010..). */
static void RTC_ReadDGUS(void)
{
	u8 rtcdata[8];
	u8 i, n, m;

	i2cstart();
	i2cbw(RTC_I2C_ADDR_W);
	i2cbw(0x00);
	i2cstart();
	i2cbw(RTC_I2C_ADDR_R);
	for (i = 6; i > 0; i--)
	{
		rtcdata[i] = i2cbr();
		mack();
	}
	rtcdata[0] = i2cbr();
	mnak();
	i2cstop();

	rtcdata[4] &= 0x7F; /* clear 12/24h flag bit for conversion */

	for (i = 0; i < 3; i++)
	{
		n = rtcdata[i] / 16;
		m = rtcdata[i] % 16;
		rtcdata[i] = n * 10 + m;
	}
	for (i = 4; i < 7; i++)
	{
		n = rtcdata[i] / 16;
		m = rtcdata[i] % 16;
		rtcdata[i] = n * 10 + m;
	}

	rtcdata[7] = 0; /* reserved */

	DGUS_WriteBytes(0x0010, (u8 *)rtcdata, 8);
}

/* Check DGUS time set trigger at 0x009C.. and apply to SD2058 */
static void RTC_CheckSetCommand(void)
{
	u8 rtc_parm[8], rtc_set[8];

	DGUS_ReadBytes(0x009C, rtc_parm, 4);
	if ((rtc_parm[0] == 0x5A) && (rtc_parm[1] == 0xA5))
	{
		rtc_set[0] = BCD(rtc_parm[2]); /* year  */
		rtc_set[1] = BCD(rtc_parm[3]); /* month */
		rtc_set[2] = BCD(rtc_parm[4]); /* day   */
		rtc_set[3] = RTC_WeekdayCalc(rtc_parm[2], rtc_parm[3], rtc_parm[4]);
		rtc_set[4] = BCD(rtc_parm[5]) | 0x80; /* hour + 24h flag */
		rtc_set[5] = BCD(rtc_parm[6]);		  /* min   */
		rtc_set[6] = BCD(rtc_parm[7]);		  /* sec   */

		rtc_config(rtc_set);

		rtc_parm[0] = 0;
		rtc_parm[1] = 0;
		DGUS_WriteBytes(0x009C, rtc_parm, 2);
	}
}

/* SD2058 periodic service */
static void RTC_Service_SD2058(void)
{
	if (RTC_Flog == 0)
	{
		RTC_Flog = 1;
		RTC_InitDefault_1();
	}
	if (sys_tick_rtc >= 500)
	{
		RTC_ReadDGUS();
		sys_tick_rtc = 0;
	}
	RTC_CheckSetCommand();
}

/* ========= Public entry ======================================================== */

/**
 * @brief Periodic RTC service. Call in main loop.
 *        Select backend via SELECT_RTC_TYPE and/or Select_rtc.
 */
void RTC_Service(void)
{
#if SELECT_RTC_TYPE
	if (Select_rtc == 1)
	{ /* RX8130 */
		if (RTC_Flog == 0)
		{
			RTC_Flog = 1;
			RTC_InitDefault();
		}
		if (sys_tick_rtc >= 500)
		{
			RTC_ReadAndSync();
			sys_tick_rtc = 0;
		}
		RTC_Set_Time();
	}
	else if (Select_rtc == 2)
	{ /* SD2058 */
		RTC_Service_SD2058();
	}
#else
	(void)Select_rtc; /* unused */
#endif
}