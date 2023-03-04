/// @file
/// @brief UART defs and inline funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#ifndef SCR_BSP_UART_16550_H
#define SCR_BSP_UART_16550_H

#include "arch.h"

#if defined(PLF_UART0_BASE) || defined(UART0_16550)

#ifndef PLF_UART_CLK
#define PLF_UART_CLK PLF_SYS_CLK
#endif

#ifndef PLF_UART_BAUDRATE
#define PLF_UART_BAUDRATE 115200
#endif // PLF_UART_BAUDRATE

#ifndef PLF_UART0_MMIO
#define PLF_UART0_MMIO 32
#endif

#include "drivers/16550_regs.h"

#ifndef __ASSEMBLER__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UART0_16550
#define UART0_16550 (*(uart16550*)PLF_UART0_BASE)
#endif

// inlines

static inline int uart0_16550_rx_ready(void)
{
    return UART0_16550.lsr & UART_16550_LSR_DR;
}

static inline int uart0_16550_tx_ready(void)
{
    return UART0_16550.lsr & UART_16550_LSR_THRE;
}

static inline void uart0_16550_tx_flush(void)
{
    while ((UART0_16550.lsr & UART_16550_LSR_TEMT) == 0);
}

static inline void uart0_16550_rx_flush(void)
{
    while (UART0_16550.lsr & UART_16550_LSR_DR)
        (void)UART0_16550.data;
}

static inline int uart0_16550_put(int data)
{
    while (!uart0_16550_tx_ready());

    return (int)(UART0_16550.data = (scr_uart_port_t)data);
}

static inline int uart0_16550_getch_nowait(void)
{
    if (uart0_16550_rx_ready())
        return (int)UART0_16550.data;

    return -1; // no input
}

static inline void uart0_16550_init(void)
{
    const unsigned div = (PLF_UART_CLK / PLF_UART_BAUDRATE + 7) / 16;
    // disable interrupts
    UART0_16550.ier = 0;
    // init MCR
    UART0_16550.mcr = 0;
    // setup LCR, baud rate
    UART0_16550.lcr = UART_16550_LCR_INIT | UART_16550_LCR_DIVL;
    UART0_16550.divlo = div & 0xff;
    UART0_16550.divhi = (div >> 8) & 0xff;
    UART0_16550.lcr = UART_16550_LCR_INIT;
    // init FIFO
    UART0_16550.fcr = UART_16550_FCR_R_RST | UART_16550_FCR_T_RST | UART_16550_FCR_EN;
    UART0_16550.fcr = UART_16550_FCR_INIT;
}

#ifdef PLF_UART0_16550
static inline void plf_con_init(void)
{
#ifndef PLF_UART0_NOINIT
    uart0_16550_init();
#endif
}

static inline void plf_con_put(int c)
{
    uart0_16550_put(c);
}

static inline int plf_con_getch_nowait(void)
{
    return uart0_16550_getch_nowait();
}

static inline void plf_con_tx_flush(void)
{
    uart0_16550_tx_flush();
}
#endif // PLF_UART0_16550

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !__ASSEMBLER__
#endif // PLF_UART0_BASE || UART0_16550
#endif // SCR_BSP_UART_16550_H
