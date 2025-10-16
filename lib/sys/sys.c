/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : sys.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : System initialization and DGUS RAM/NOR Flash access layer.
 *                Provides read/write helpers, page control, graph ops, and reset
 *                handling for DWIN T5L-based HMIs.
 * ----------------------------------------------------------------------------- */

#include "sys.h"

void Sys_Init(void)
{

    DPC = 0x00;       // Is DPTR +1 C51=0
    CKCON = 0x00;     // CPU running=0, 1T mode
    D_PAGESEL = 0x02; // set DPTR upper page for MOVX

    IP0 = 0x00;
    IP1 = 0x00; // Interrupt priority default
    RAMMODE = 0x00;
    /* IO mode */
    P0MDOUT = 0x53; // p0.0 p0.1 forced push for 485 direction control P0.4 leads to serial port 2 P0.6 leads to serial port 3
    P2MDOUT = 0xC0;
    P3MDOUT = 0x0C; // Turn on the output of P3_2 and P3_3 P3_2=RTC_SCL P3_3=RTC_SDA
    PORTDRV = 0x01; // IO drive capability setting 4mA=0,8mA=1,16mA=2,32mA=3
    P2 = 0XC0;
    P3 = 0x00;
    MUX_SEL |= 0x00; // WDT off, UART2, UART3 and CAN are not led out

    // Timer0_Init();
    // Timer1_Init();
    Timer2_Init();
}

/* DGUS RAM access modes */
#define DGUS_MODE_READ 0xAF  ///< DGUS RAM read mode
#define DGUS_MODE_WRITE 0x8F ///< DGUS RAM write mode
#define DGUS_MODE_IDLE 0x00  ///< DGUS RAM idle mode
#define DGUS_TIMEOUT 10000   ///< Timeout for DGUS operations

/* Wait for APP_ACK to go high */
static void dgus_wait_ack_high(void)
{
    while (!APP_ACK)
        ;
}
/* Kick the APP and wait for it to finish */
static void dgus_kick_and_wait_done(void)
{
    APP_EN = 1;
    while (APP_EN)
        ;
    while (!APP_ACK)
        ;
}

/**
 * @brief Read a 16-bit value from DGUS RAM (addr = BYTE address).
 *        Even addr -> DATA3:DATA2, Odd addr -> DATA1:DATA0
 */
u16 DGUS_Read_VP(u16 addr)
{
    u16 word_addr = addr >> 1; // DGUS word address
    u16 val;

    // Set address
    ADR_H = 0x00;
    ADR_M = (u8)(word_addr >> 8);
    ADR_L = (u8)(word_addr);

    // Read cycle
    RAMMODE = 0xAF; // DGUS_MODE_READ
    dgus_wait_ack_high();
    dgus_kick_and_wait_done();

    // Byte parity check for correct lane
    if (addr % 2)
        val = ((u16)DATA1 << 8) | (u16)DATA0; // odd
    else
        val = ((u16)DATA3 << 8) | (u16)DATA2; // even

    RAMMODE = 0x00; // idle
    return val;
}

/**
 * @brief Write a 16-bit value to DGUS RAM (addr = BYTE address).
 *        Even addr -> write DATA3:DATA2 (0x8C), Odd addr -> DATA1:DATA0 (0x83)
 */
void DGUS_Write_VP(u16 addr, u16 val)
{
    u16 word_addr = addr >> 1;
    // Set address
    ADR_H = 0x00;
    ADR_M = (u8)(word_addr >> 8);
    ADR_L = (u8)(word_addr);

    /* Some revisions require an initial read cycle for reliability */
    RAMMODE = 0xAF; // READ
    dgus_wait_ack_high();
    dgus_kick_and_wait_done();

    // Parity check for correct write mode and lanes
    if (addr & 0x01)
    {
        RAMMODE = 0x83; // DATA1 + DATA0 (odd word)
        dgus_wait_ack_high();
        DATA1 = (u8)(val >> 8);
        DATA0 = (u8)(val & 0xFF);
        dgus_kick_and_wait_done();
    }
    else
    {
        RAMMODE = 0x8C; // DATA3 + DATA2 (even word)
        dgus_wait_ack_high();
        DATA3 = (u8)(val >> 8);
        DATA2 = (u8)(val & 0xFF);
        dgus_kick_and_wait_done();
    }

    RAMMODE = 0x00; // idle
}

/**
 * @brief Write a byte stream to DGUS RAM starting at BYTE address 'addr'.
 *        Uses ONLY DGUS_Write_VP() and DGUS_Read_VP() with safe read-modify-write.
 *        - odd start: first byte goes to current word LSB (RMW), then align
 *        - aligned loop: write full words (2 bytes) via DGUS_Write_VP()
 *        - tail 1 byte: write MSB of final word via RMW
 */
void DGUS_WriteBytes(u16 addr, const u8 *buf, u16 len)
{
    u16 idx = 0;

    // full words
    while ((len - idx) >= 2)
    {
        u16 w = ((u16)buf[idx] << 8) | (u16)buf[idx + 1]; // MSB, LSB
        DGUS_Write_VP((u16)(addr + (idx >> 1)), w);       // addr + word_index
        idx += 2;
    }

    // tail 1 byte?
    if ((len - idx) == 1)
    {
        u16 word_vp = (u16)(addr + (idx >> 1)); // target word VP
        u16 old = DGUS_Read_VP(word_vp);
        if (((addr + idx) & 0x01) == 0)
        {
            // last byte lands at even VP → MSB of word
            u16 v = ((u16)buf[idx] << 8) | (old & 0x00FF);
            DGUS_Write_VP(word_vp, v);
        }
        else
        {
            // last byte lands at odd VP  → LSB of word
            u16 v = (old & 0xFF00) | (u16)buf[idx];
            DGUS_Write_VP(word_vp, v);
        }
    }
    delay_ms(20);
}

/**
 * @brief Read 'words' DGUS VPs and unpack to bytes as [HI, LO] pairs.
 * @param addr   Starting VP address (BYTE-addressed, e.g. 0x2000)
 * @param buf    Destination buffer (size >= 2 * words)
 * @param words  Number of 16-bit words to read
 * @return 1 on success
 */
void DGUS_ReadBytes(u16 addr, u8 *buf, u16 words)
{
    for (u16 i = 0; i < words; i++)
    {
        u16 v = DGUS_Read_VP((u16)(addr + i)); // consecutive VPs
        buf[2u * i] = (u8)(v >> 8);            // HI
        buf[2u * i + 1] = (u8)(v & 0xFF);      // LO
    }
}

/**
 * @brief Write a null-terminated ASCII string to DGUS RAM.
 *
 * This function takes a C string (e.g. "HELLO") and writes its characters
 * as ASCII codes into consecutive DGUS VP addresses using DGUS_WriteBytes().
 *
 * @param addr DGUS VP address where the string should be written
 * @param text Null-terminated string to write
 */
void DGUS_WriteText(u16 addr, const char *text)
{
    u16 len = 0;
    const char *p = text;

    // Calculate string length
    while (*p++)
        len++;

    // Write bytes to DGUS RAM
    DGUS_WriteBytes(addr, (const u8 *)text, len);
}

/**
 * @brief Read the current DGUS page ID from the system register.
 *
 * The DGUS system maintains the currently active page ID at address 0x0014.
 *
 * @return uint8_t  Current active page ID.
 */
u8 DGUS_GetPageID(void)
{
    u8 page_id_reg[2] = {0x00, 0x00};

    // 0x0014 = Page ID register (1 word)
    DGUS_ReadBytes(PIC_Now_VP, page_id_reg, 2);

    return page_id_reg[1];
}

/**
 * @brief Switch DGUS to a new page ID.
 *
 * Writes to the system page control register (0x0084) with the requested page.
 *
 * @param page_id  Target page ID to switch to.
 */
void DGUS_SetPageID(u8 page_id)
{
    // Switch page packet template
    u8 packet[4] = {0x5A, 0x01, 0x00, 0x00};

    // Replace with target page
    packet[3] = page_id;

    // 0x0084 = Page control register
    DGUS_WriteBytes(PIC_Set_VP, packet, 4);
}

/**
 * @brief Wait for NOR Flash operation to complete or timeout.
 * @param timeout_ms  Maximum wait time in milliseconds
 * @return 1 if done, 0 if timeout
 */
static u8 DGUS_NOR_WaitDone(u16 timeout_ms)
{
    u8 st[2];
    while (timeout_ms--)
    {
        DGUS_ReadBytes(NOR_FLASH_RW_VP, st, 1); // first word: [HI,LO], HI=status
        if (st[0] == 0x00)
            return 1;
        delay_ms(1);
    }
    return 0;
}

/**
 * @brief Copy from DGUS RAM to NOR Flash.
 * @param nor_addr   24-bit NOR Target address (byte)
 * @param vp_addr    DGUS source VP (byte-addressed)
 * @param len_bytes  number of bytes to copy (rounded up to even if necessary)
 * @return 1 success, 0 timeout
 */
u8 DGUS_NOR_Write(u32 nor_addr, u16 vp_addr, u16 len_bytes)
{
    u8 pkt[8];
    if (len_bytes & 1)
        len_bytes++; // word alignment

    pkt[0] = 0xA5;
    pkt[1] = (u8)(nor_addr >> 16);
    pkt[2] = (u8)(nor_addr >> 8);
    pkt[3] = (u8)(nor_addr);
    pkt[4] = (u8)(vp_addr >> 8);
    pkt[5] = (u8)(vp_addr);
    pkt[6] = (u8)(len_bytes >> 8);
    pkt[7] = (u8)(len_bytes);

    DGUS_WriteBytes(NOR_FLASH_RW_VP, pkt, 8);
    return DGUS_NOR_WaitDone(2000); // can be increased if needed
}

/**
 * @brief Copy from NOR Flash to DGUS RAM.
 * @param nor_addr   24-bit NOR source address (byte)
 * @param vp_addr    DGUS target VP (byte-addressed)
 * @param len_bytes  number of bytes to copy (rounded up to even if necessary)
 * @return 1 success, 0 timeout
 */
u8 DGUS_NOR_Read(u32 nor_addr, u16 vp_addr, u16 len_bytes)
{
    u8 pkt[8];
    if (len_bytes & 1)
        len_bytes++; // word alignment

    pkt[0] = 0x5A;
    pkt[1] = (u8)(nor_addr >> 16);
    pkt[2] = (u8)(nor_addr >> 8);
    pkt[3] = (u8)(nor_addr);
    pkt[4] = (u8)(vp_addr >> 8);
    pkt[5] = (u8)(vp_addr);
    pkt[6] = (u8)(len_bytes >> 8);
    pkt[7] = (u8)(len_bytes);

    DGUS_WriteBytes(NOR_FLASH_RW_VP, pkt, 8);
    return DGUS_NOR_WaitDone(2000);
}

/**
 * @brief Push one sample into DGUS graph FIFO.
 *
 * DGUS expects a 10-byte command:
 *   [5A A5] [01] [00] [CH] [02] [VAL_H] [VAL_L] [VAL_H] [VAL_L]
 * which we write to VP 0x0310.
 *
 * @param channel  Graph channel index (0..7).
 * @param value    16-bit sample value.
 */
void DGUS_GraphPush(u8 channel, u16 value)
{
    if (channel >= 8)
        return;

    u8 pkt[10] = {
        0x5A, 0xA5, // header
        0x01,       // command: push
        0x00,       // reserved
        0x00,       // channel (filled below)
        0x02,       // sample length (2 bytes)
        0x00, 0x00, // value hi, lo
        0x00, 0x00  // duplicated value hi, lo (per DGUS spec)
    };

    pkt[4] = channel;
    pkt[6] = (u8)(value >> 8);
    pkt[7] = (u8)(value);
    pkt[8] = pkt[6];
    pkt[9] = pkt[7];

    DGUS_WriteBytes(Curve_Data_VP, pkt, sizeof(pkt));
}

/**
 * @brief Clear one graph channel or all channels.
 *
 * Writes 0x0000 to the channel value registers:
 *   CH0..CH7 at VP: 0x0301, 0x0303, ..., 0x030F
 *
 * @param channel  0..7: clear that channel, 8: clear all.
 */
void DGUS_GraphClear(u8 channel)
{
    if (channel < 8)
    {
        u16 vp = (u16)(Curve_Ch0_VP + (u16)channel * 2);
        DGUS_Write_VP(vp, 0x0000);
        return;
    }

    if (channel == 8) // 8 => clear all
    {
        for (u8 ch = 0; ch < 8; ch++)
        {
            u16 vp = (u16)(Curve_Ch0_VP + (u16)ch * 2);
            DGUS_Write_VP(vp, 0x0000);
        }
    }
}

/**
 * @brief Send a reset command to the DGUS HMI.
 *
 * This function writes a specific 4-byte sequence to the system reset VP (0x0004)
 * to trigger a software reset of the connected DGUS HMI module.
 */
void DGUS_ResetHmi(void)
{
    // Reset command packet
    u8 pkt[4] = {0x55, 0xAA, 0x5A, 0xA5};
    DGUS_WriteBytes(System_Reset_VP, pkt, sizeof(pkt));
}