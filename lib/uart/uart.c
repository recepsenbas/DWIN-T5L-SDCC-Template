/* -----------------------------------------------------------------------------
 *  Project : DWIN-T5L-SDCC-Template
 *  File    : uart.c
 *  Author  : Recep Şenbaş (https://github.com/recepsenbas)
 *  License : CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
 *  Contact : recepsenbas@gmail.com
 *  Description : UART driver implementation for DWIN T5L (DGUS compatible).
 *                Supports UART2–UART5, interrupt-driven RX, polling TX, and DGUS
 *                protocol frame handling (0x82 / 0x83 commands with optional CRC).
 * ----------------------------------------------------------------------------- */
#include "uart.h"
#define VP_FLAGS_DEFINE
#include "vp_flags.h"
#define UART_FLAGS_DEFINE
#include "uart_flags.h"

__bit Crc_check_flog = 0;
__bit Response_flog = 0;
__bit g_in_download_mode = 0;
#if UART2_ENABLE
u8 __xdata R_u2[UART2_RX_LENTH];
volatile u8 __xdata R_OD2 = 0;  // Receive data flag
volatile u16 __xdata R_CN2 = 0; // Receive data count
volatile u8 __xdata T_O2 = 0;   // Receive data timeout
volatile __bit Busy2 = 0;       // Transmit busy flag
#endif

#if UART3_ENABLE
u8 __xdata R_u3[UART3_RX_LENTH];
volatile u8 __xdata R_OD3 = 0;  // Receive data flag
volatile u16 __xdata R_CN3 = 0; // Receive data count
volatile u8 __xdata T_O3 = 0;   // Receive data timeout
volatile __bit Busy3 = 0;       // Transmit busy flag
#endif

#if UART4_ENABLE
u8 __xdata R_u4[UART4_RX_LENTH];
volatile u8 __xdata R_OD4 = 0;  // Receive data flag
volatile u16 __xdata R_CN4 = 0; // Receive data count
volatile u8 __xdata T_O4 = 0;   // Receive data timeout
volatile __bit Busy4 = 0;       // Transmit busy flag
#endif

#if UART5_ENABLE
u8 __xdata R_u5[UART5_RX_LENTH];
volatile u8 __xdata R_OD5 = 0;  // Receive data flag
volatile u16 __xdata R_CN5 = 0; // Receive data count
volatile u8 __xdata T_O5 = 0;   // Receive data timeout
volatile __bit Busy5 = 0;       // Transmit busy flag
#endif

/**
 * @brief Initialize UART2, UART3, UART4, and UART5 based on configuration.
 *
 * This function sets up the specified UART channels with the configured baud rates,
 * modes, and interrupt settings. It also configures the necessary pins for TX/RX
 * functionality. Each UART channel is conditionally compiled based on the
 * corresponding `UARTx_ENABLE` macro.
 */
void Uart_Init(void)
{
#if UART2_ENABLE
    {
        u16 i = 1024 - FOSC / 64 / BAUD_UART2;
        SREL2H = (u8)(i >> 8); // Baud rate = FCLK/64*(1024-SREL)
        SREL2L = (u8)i;        //
        MUX_SEL |= 0x40;       // UART2 is led out, UART3 is not led out, and WDT is turned off
        ADCON = 0x80;          // Select SREL0H:L as the baud rate generator
        SCON2 = 0x50;          // Accept enable and mode settings
        PCON &= 0x7F;          // SMOD=0
        IEN0 |= 0X10;          // ES0=1 Serial port 2 receiving + sending interrupt
        EA = 1;
        R_OD2 = 0;
        R_CN2 = 0;
    }
#endif

#if UART3_ENABLE
    {
        u16 i = 1024 - FOSC / 32 / BAUD_UART3;
        SREL3H = (u8)(i >> 8);
        SREL3L = (u8)i;  // 1024 - Fclk/(32*baud)
        MUX_SEL |= 0x20; // UART3 lead
        P0MDOUT |= 0x40; // P0.6 TXD push-pull
        SCON3 = 0x90;    // Enable RX, mode settings
        IEN2 |= 0x01;    // UART3 interrupt enable
        EA = 1;
    }
#endif

#if UART4_ENABLE
    {
        u16 i = FOSC / 8 / BAUD_UART4;
        BODE4_DIV_H = (u8)(i >> 8);
        BODE4_DIV_L = (u8)i;
        SCON4T = 0x80; // TX enable, mode
        SCON4R = 0x80; // RX enable, mode
        ES4R = 1;      // RX interrupt enable
        ES4T = 1;      // TX interrupt enable (kept off in polling TX path)
        TR4 = 0;
        EA = 1;
    }
#endif

#if UART5_ENABLE
    {
        u16 i = FOSC / 8 / BAUD_UART5;
        BODE5_DIV_H = (u8)(i >> 8);
        BODE5_DIV_L = (u8)i;
        SCON5T = 0x80;       // TX enable, 8-bit mode
        /* SCON5T = 0xC0; */ // TX enable, 9-bit mode (optional)
        SCON5R = 0x80;       // RX enable
        ES5R = 1;            // RX interrupt enable
        ES5T = 1;            // TX interrupt enable (kept off in polling TX path)
        TR5 = 0;
        EA = 1;
    }
#endif
}

// UART2 ISR
void uart2_ISR(void) __interrupt(4)
{
#if UART2_ENABLE
    if (RI2) // receive interrupt
    {
        R_u2[R_CN2] = SBUF2; // store data
        SCON2 &= 0xFE;       // clear RI2
        R_OD2 = 1;           // set flag
        if (R_CN2 < UART2_RX_LENTH - 1)
            R_CN2++;
        T_O2 = 5; // timeout reload
    }
    if (TI2 == 1)
    {
        SCON2 &= 0xFD;
        Busy2 = 0;
    }
#endif
}

// UART3 ISR
void uart3_ISR(void) __interrupt(16)
{
#if UART3_ENABLE
    if (SCON3 & 0x01)
    {
        R_u3[R_CN3] = SBUF3;
        SCON3 &= 0xFE;
        //				        SCON3&=0xFE;
        R_OD3 = 1;
        if (R_CN3 < UART3_RX_LENTH - 1)
            R_CN3++;
        T_O3 = 5;
    }
    if (SCON3 & 0x02)
    {
        SCON3 &= 0xFD;
        SCON3 &= 0xFD;
        Busy3 = 0;
    }
#endif
}

// UART4 receive ISR
void uart4_RISR(void) __interrupt(11)
{
#if UART4_ENABLE
    R_u4[R_CN4] = SBUF4_RX;
    SCON4R &= 0xFE;
    R_OD4 = 1;
    if (R_CN4 < UART4_RX_LENTH - 1)
        R_CN4++;
    T_O4 = 5;
#endif
}

// UART4 transmit ISR
void uart4_TISR(void) __interrupt(10)
{
#if UART4_ENABLE
    SCON4T &= 0xFE;
    Busy4 = 0;
#endif
}

// UART5 receive ISR
void uart5_RISR(void) __interrupt(13)
{
#if UART5_ENABLE
    if (RI5) // receive flag
    {
        R_u5[R_CN5] = SBUF5_RX;
        SCON5R &= 0xFE;                 // clear receive flag
        R_OD5 = 1;                      // receive data flag
        if (R_CN5 < UART5_RX_LENTH - 1) // prevent overflow
            R_CN5++;                    // increase receive count
        T_O5 = 5;                       // set receive timeout
    }
#endif
}
// UART5 transmit ISR
void uart5_TISR(void) __interrupt(12)
{
#if UART5_ENABLE
    SCON5T &= 0xFE;
    Busy5 = 0;
#endif
}

/**
 * @brief Send a single byte over the specified UART.
 *
 * This function waits until the UART is not busy, then writes the byte to the
 * appropriate SBUF register to initiate transmission.
 *
 * @param Uart_number UART channel number (2, 3, 4, or 5)
 * @param Dat         Byte to send
 */
void uart_send_byte(u8 Uart_number, u8 Dat)
{
    if (Uart_number == 2)
    {
#if UART2_ENABLE
        while (Busy2)
            ;
        Busy2 = 1;
        SBUF2 = Dat;
#endif
    }
    else if (Uart_number == 3)
    {
#if UART3_ENABLE
        while (Busy3)
            ;
        Busy3 = 1;
        SBUF3 = Dat;
#endif
    }
    else if (Uart_number == 4)
    {
#if UART4_ENABLE
        while (Busy4)
            ;
        Busy4 = 1;
        SBUF4_TX = Dat;
#endif
    }
    else if (Uart_number == 5)
    {
#if UART5_ENABLE
        while (Busy5)
            ;
        Busy5 = 1;
        SBUF5_TX = Dat;
#endif
    }
}

/**
 * @brief Send a null-terminated string over the specified UART.
 *
 * If CRC is enabled (USE_CRC), it calculates the CRC16 of the string and appends
 * it to the transmission. The function controls the TX pins for UART4 and UART5
 * to manage half-duplex communication.
 *
 * @param Uart_number UART channel number (2, 3, 4, or 5)
 * @param str         Pointer to the null-terminated string to send
 */
void uart_send_str(u8 Uart_number, u8 *str)
{
#if USE_CRC
    u8 arr[256];
    u8 len = 0;
    uart_4_5_pin_ctrl(Uart_number, 1);
    while (*str)
    {
        arr[len++] = *str;
        uart_send_byte(Uart_number, *str++);
    }
    u16 crc = crc16table(arr, len);
    uart_send_byte(Uart_number, (u8)(crc & 0xFF));
    uart_send_byte(Uart_number, (u8)(crc >> 8));
    uart_4_5_pin_ctrl(Uart_number, 0);

#else
    uart_4_5_pin_ctrl(Uart_number, 1);
    while (*str)
        uart_send_byte(Uart_number, *str++);
    uart_4_5_pin_ctrl(Uart_number, 0);
#endif
}

/**
 * @brief Send an array of bytes over the specified UART.
 *
 * @param Uart_number UART channel number (2, 3, 4, or 5)
 * @param arr Pointer to the byte array to send
 * @param len Number of bytes to send from the array
 */
void uart_send_arr(u8 Uart_number, u8 *arr, u8 len)
{
#if USE_CRC
    uart_4_5_pin_ctrl(Uart_number, 1);
    for (u8 i = 0; i < len; i++)
        uart_send_byte(Uart_number, arr[i]);
    u16 crc = crc16table(arr, len);
    uart_send_byte(Uart_number, (u8)(crc & 0xFF));
    uart_send_byte(Uart_number, (u8)(crc >> 8));
    uart_4_5_pin_ctrl(Uart_number, 0);
#else
    uart_4_5_pin_ctrl(Uart_number, 1);
    for (u8 i = 0; i < len; i++)
        uart_send_byte(Uart_number, arr[i]);
    uart_4_5_pin_ctrl(Uart_number, 0);
#endif
}

/**
 * @brief Send a 16-bit word over the specified UART.
 *
 * @param Uart_number
 * @param data
 */
void uart_send_word(u8 Uart_number, u16 data)
{
    uart_4_5_pin_ctrl(Uart_number, 1);
    uart_send_byte(Uart_number, (u8)(data >> 8));
    uart_send_byte(Uart_number, (u8)(data & 0x00FF));
    uart_4_5_pin_ctrl(Uart_number, 0);
}

/**
 * @brief Monitor DGUS variable change flag and send updated data over UART.
 *
 * This function continuously checks DGUS system registers (0x0F00, 0x0F01) for variable
 * change events. When a change is detected, it reads the updated data from DGUS RAM,
 * formats it into a communication packet (5A A5 protocol frame), and sends it through
 * all enabled UART channels. After transmission, it clears the DGUS update flags.
 *
 * @note
 *  - Packet format: [5A A5 | Length | 0x83 | Address_H | Address_L | Data...]
 *  - UART channels are conditionally compiled with `UARTx_ENABLE`.
 *  - Clears DGUS registers 0x0F00 and 0x0F01 after sending.
 */
void DGUS_MonitorAndSendUpdates(void)
{
    if (monitor_ms < 100)
        return;
    monitor_ms = 0;
    u16 change_flag = DGUS_Read_VP(0x0F00); // Variable change indication
    u16 var_length = DGUS_Read_VP(0x0F01);  // Variable length
    u16 last_addr = 0;

    // Only proceed if change_flag high-byte == 0x5A
    if (((u8)(change_flag >> 8)) == 0x5A)
    {
        u8 i = 0;
        u16 temp_val = 0;
        static __xdata u8 packet[100] = {0};

        // Build packet header
        // Format (DGUS 0x83 response-like):
        // [5A A5 | LEN | 0x83 | AddrH | AddrL | Words | Data(2*Words) | (CRC Lo | CRC Hi if CRC ON)]
        packet[0] = 0x5A;
        packet[1] = 0xA5;
        {
            u8 words = (u8)var_length;                // var_length low-byte = number of words
            packet[2] = (u8)(2u * words + 4u);        // LEN (without CRC) = 4 + 2*Words
        }
        packet[3] = 0x83;
        packet[4] = (u8)change_flag;       // AddrH (0x0F00 high-byte==0x5A, low-byte carries AddrH)
        packet[5] = (u8)(var_length >> 8); // AddrL (0x0F01 high-byte)
        packet[6] = (u8)var_length;        // Words  (0x0F01 low-byte)

        last_addr = ((u16)packet[4] << 8) | packet[5];
        /* Check Flags */
        Flags_SetByVP(last_addr);
        // Read variable data from DGUS and append to packet
        for (i = 0; i < (u8)var_length; i++)
        {
            temp_val = DGUS_Read_VP(last_addr + i);
            packet[7 + 2 * i] = (u8)(temp_val >> 8);
            packet[8 + 2 * i] = (u8)(temp_val);
        }

#if USE_CRC
        // If CRC is enabled, LEN must include the CRC(2) and CRC is computed over [payload], i.e. from packet[3] for (LEN-2) bytes
        packet[2] = (u8)(packet[2] + 2u);
        {
            u16 crc = crc16table(packet + 3, (u16)(packet[2] - 2u));
            packet[3 + (u16)packet[2] - 2u] = (u8)(crc & 0xFF);  // CRC LOW
            packet[3 + (u16)packet[2] - 1u] = (u8)(crc >> 8);    // CRC HIGH
        }
#endif

        // Send packet over enabled UART channels (send exactly LEN+3 bytes; do not append CRC here)
#if UART2_ENABLE
        uart_4_5_pin_ctrl(2, 1);
        for (u16 i = 0; i < (u16)packet[2] + 3u; i++)
            uart_send_byte(2, packet[i]);
        uart_4_5_pin_ctrl(2, 0);
#endif
#if UART3_ENABLE
        uart_4_5_pin_ctrl(3, 1);
        for (u16 i = 0; i < (u16)packet[2] + 3u; i++)
            uart_send_byte(3, packet[i]);
        uart_4_5_pin_ctrl(3, 0);
#endif
#if UART4_ENABLE
        uart_4_5_pin_ctrl(4, 1);
        for (u16 i = 0; i < (u16)packet[2] + 3u; i++)
            uart_send_byte(4, packet[i]);
        uart_4_5_pin_ctrl(4, 0);
#endif
#if UART5_ENABLE
        uart_4_5_pin_ctrl(5, 1);
        for (u16 i = 0; i < (u16)packet[2] + 3u; i++)
            uart_send_byte(5, packet[i]);
        uart_4_5_pin_ctrl(5, 0);
#endif

        // Clear DGUS flags
        DGUS_Write_VP(0x0F00, 0);
        DGUS_Write_VP(0x0F01, 0);
    }
}

/**
 * @brief Handle DGUS "0x82" command received from UART.
 *
 * This function processes a command frame with command code 0x82:
 *  - If CRC check is disabled, it directly writes the received payload to DGUS RAM.
 *  - If CRC check is enabled, it verifies the CRC before writing.
 *  - If Response_flog is set, it sends an ACK response back on the same UART channel.
 *
 * @param uart UART channel number (e.g., 2, 3, 4, 5)
 * @param frame Pointer to received command frame buffer
 */
void DGUS_HandleCmd82(u8 uart, u8 *frame)
{
    if (Crc_check_flog == 0) // --- Case 1: No CRC check ---
    {
        DGUS_WriteBytes((frame[4] << 8) + frame[5], frame + 6, frame[2] - 3);

        if (Response_flog)
        {
            const u8 ack[] = {DTHD1, DTHD2, 0x03, 0x82, 0x4F, 0x4B};
            uart_4_5_pin_ctrl(uart, 1);
            for (u8 i = 0; i < 6; i++)
                uart_send_byte(uart, ack[i]);
            uart_4_5_pin_ctrl(uart, 0);
        }
    }
    else // --- Case 2: With CRC check ---
    {
        u16 crc_calc = crc16table(frame + 3, frame[2] - 2);

        // CRC is [low][high]
        u16 crc_recv = (u16)frame[frame[2] + 2] << 8 | frame[frame[2] + 1];

        if (crc_calc == crc_recv)
        {
            DGUS_WriteBytes((frame[4] << 8) + frame[5], frame + 6, frame[2] - 5);

            if (Response_flog)
            {
                u8 ack_crc[] = {DTHD1, DTHD2, 0x05, 0x82, 0x4F, 0x4B, 0x00, 0x00};

                // Calculate and append CRC
                u16 ack_crc_val = crc16table(ack_crc + 3, ack_crc[2] - 2);
                ack_crc[ack_crc[2] + 1] = (u8)(ack_crc_val & 0xFF); // low
                ack_crc[ack_crc[2] + 2] = (u8)(ack_crc_val >> 8);   // high
                uart_4_5_pin_ctrl(uart, 1);
                for (u8 i = 0; i < ack_crc[2] + 3; i++)
                    uart_send_byte(uart, ack_crc[i]);
                uart_4_5_pin_ctrl(uart, 0);
            }
        }
    }
}

/**
 * @brief Handle DGUS "0x83" command received from UART.
 *
 * This function processes a command frame with command code 0x83 (read request):
 *  - If CRC check is disabled, it directly reads the requested data from DGUS RAM
 *    and sends it back in a formatted packet.
 *  - If CRC check is enabled, it verifies the CRC, reads the requested data,
 *    appends CRC to the response, and sends it back.
 *
 * @param uart     UART channel number (e.g., 2, 3, 4, 5)
 * @param response Buffer to build the response frame
 * @param request  Pointer to received request frame
 */
void DGUS_HandleCmd83(u8 uart, u8 *response, const u8 *request)
{
    u16 addr;
    u8 words;

    if (Crc_check_flog == 0)
    { // ---- CRC OFF ----
        // İlk 7 baytı kopyala: 83, AddrH, AddrL, Words dahil
        for (u8 i = 0; i < 7; i++)
            response[i] = request[i];

        addr = ((u16)response[4] << 8) | response[5];
        words = response[6];

        DGUS_ReadBytes(addr, &response[7], words);

        response[2] = (u8)(2u * words + 4u); // LEN = 2*Words + 4
        uart_4_5_pin_ctrl(uart, 1);
        for (u16 i = 0; i < (u16)response[2] + 3u; i++)
            uart_send_byte(uart, response[i]);
        uart_4_5_pin_ctrl(uart, 0);
    }
    else
    {                        // ---- CRC ON ----
        u8 len = request[2]; // LEN alanı (payload+CRC)
        // CRC over [request+3 .. request+3+(len-2)-1]
        u16 crc_calc = crc16table((u8 *)(request + 3), (u16)(len - 2));
        u8 crc_lo = request[3 + (u16)len - 2];
        u8 crc_hi = request[3 + (u16)len - 1];
        u16 crc_recv = ((u16)crc_hi << 8) | crc_lo;

        if (crc_calc == crc_recv)
        {
            for (u8 i = 0; i < 7; i++)
                response[i] = request[i];

            addr = ((u16)response[4] << 8) | response[5];
            words = response[6];

            DGUS_ReadBytes(addr, &response[7], words);

            response[2] = (u8)(2u * words + 4u + 2u); // +2 CRC

            // Response CRC: over [response+3 .. +3+(LEN-2)-1]
            u16 resp_crc = crc16table(response + 3, (u16)(response[2] - 2));
            response[3 + (u16)response[2] - 2] = (u8)(resp_crc & 0xFF); // CRC LOW
            response[3 + (u16)response[2] - 1] = (u8)(resp_crc >> 8);   // CRC HIGH
            uart_4_5_pin_ctrl(uart, 1);
            for (u16 i = 0; i < (u16)response[2] + 3u; i++)
                uart_send_byte(uart, response[i]);
            uart_4_5_pin_ctrl(uart, 0);
        }
        // if CRC mismatch: silently ignore (kept for protocol compatibility)
    }
}

/**
 * @brief Parse and process incoming DGUS UART frames.
 *
 * This function scans the received buffer for valid DGUS frames (starting with 0x5A 0xA5),
 * identifies the command code (0x82 or 0x83), and dispatches to the corresponding
 * command handler. CRC and response flags are applied dynamically.
 *
 * @param rx_buf    Pointer to receive buffer
 * @param data_len  Pointer to variable holding number of received bytes
 * @param uart      UART channel number (e.g., 2, 3, 4, 5)
 * @param response  Response flag (1 = send response, 0 = silent)
 * @param crc_check CRC check flag (1 = verify CRC, 0 = no CRC)
 */
void DGUS_ParseUartFrame(u8 *Arr, u16 *Len, u8 uart, __bit resp, __bit crc_on)
{
    u16 N = 0;
    u16 total = *Len;
    static __xdata u8 frame[256];  // tek frame’lik güvenli buffer
    static __xdata u8 resp83[128]; // 83 için geçici cevap alanı

    while (N + 3u <= total)
    {
        if (Arr[N] != DTHD1 || Arr[N + 1] != DTHD2)
        {
            N++;
            continue;
        }

        u8 len = Arr[N + 2];
        u16 fbytes = (u16)len + 3u;
        if (fbytes > sizeof(frame))
            break; // too large: safety cutoff
        if (N + fbytes > total)
            break; // incomplete frame: try later

        for (u16 i = 0; i < fbytes; i++)
            frame[i] = Arr[N + i];

        Response_flog = resp;
        Crc_check_flog = crc_on;

        u16 addr = (u16)frame[4] << 8 | frame[5];
        Flags_SetByUart(addr);
        if (frame[3] == 0x82)
        {
            DGUS_HandleCmd82(uart, frame);
        }
        else if (frame[3] == 0x83)
        {
            DGUS_HandleCmd83(uart, resp83, frame);
        }
        N += fbytes;
    }
}

/**
 * @brief Dispatch UART receive buffers for DGUS frame parsing.
 *
 * This function checks if any UART channel (2..5) has completed receiving a frame,
 * then calls DGUS_ParseUartFrame() with the proper parameters. After processing,
 * it clears the receive flags and resets the counters.
 */
void DGUS_ProcessAllUarts(void)
{
    if (g_in_download_mode)
        return;
#if UART2_ENABLE
    if ((R_OD2) && (!T_O2))
    {
        DGUS_ParseUartFrame(R_u2, (u16 *)(&R_CN2), 2, RESPONSE_UART2, USE_CRC);
        R_OD2 = 0;
        R_CN2 = 0;
    }
#endif
#if UART3_ENABLE
    if ((R_OD3) && (!T_O3))
    {
        DGUS_ParseUartFrame(R_u3, (u16 *)(&R_CN3), 3, RESPONSE_UART3, USE_CRC);
        R_OD3 = 0;
        R_CN3 = 0;
    }
#endif
#if UART4_ENABLE
    if ((R_OD4 == 1) && (T_O4 == 0))
    {
        DGUS_ParseUartFrame(R_u4, (u16 *)(&R_CN4), 4, RESPONSE_UART4, USE_CRC);
        R_OD4 = 0;
        R_CN4 = 0;
    }
#endif
#if UART5_ENABLE
    if ((R_OD5 == 1) && (T_O5 == 0))
    {
        DGUS_ParseUartFrame(R_u5, (u16 *)(&R_CN5), 5, RESPONSE_UART5, USE_CRC);
        R_OD5 = 0;
        R_CN5 = 0;
    }
#endif
}

/**
 * @brief Control the TX pins for UART4 and UART5.
 */
void uart_4_5_pin_ctrl(u8 uart_num, u8 state)
{
#if UART4_ENABLE
    if (uart_num == 4)
    {
        if (state)
            TR4 = 1;
        else
        {
            while (Busy4)
                ;
            TR4 = 0;
        }
    }
#endif
#if UART5_ENABLE
    if (uart_num == 5)
    {
        if (state)
            TR5 = 1;
        else
        {
            while (Busy5)
                ;
            TR5 = 0;
        }
    }
#endif
}