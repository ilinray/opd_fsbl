/// @file
/// @brief SiFive UART defs and inline funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2020. All rights reserved.

#ifndef SCR_BSP_UART_SIFIVE_H
#define SCR_BSP_UART_SIFIVE_H

#include "arch.h"

#ifndef __ASSEMBLER__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _uartSifive {
    int32_t txdata;
    int32_t rxdata;
    int32_t txctrl;
    int32_t rxctrl;
    int32_t ie;
    int32_t ip;
    int32_t baud;
} volatile uartSifive;

#if defined(PLF_UART0_BASE) || defined(UART0_SIFIVE)

#ifndef UART0_SIFIVE
#define UART0_SIFIVE (*(uartSifive*)PLF_UART0_BASE)
#endif

// inlines

static inline void uart0_sifive_tx_flush(void)
{
}

static inline int uart0_sifive_put(int data)
{
    while (UART0_SIFIVE.txdata < 0);
    UART0_SIFIVE.txdata = data;
    return data;
}

static inline int uart0_sifive_getch_nowait(void)
{
    int ch = UART0_SIFIVE.rxdata;
    if (ch < 0)
        return -1; // no input

    return ch;
}

static inline int uart0_sifive_init(void)
{
    UART0_SIFIVE.txctrl = 1;
    UART0_SIFIVE.rxctrl = 1;
    UART0_SIFIVE.ie = 0;
}

#ifdef PLF_UART0_SIFIVE
static inline void plf_con_init(void)
{
#ifndef PLF_UART0_NOINIT
    uart0_sifive_init();
#endif
}

static inline void plf_con_put(int c)
{
    uart0_sifive_put(c);
}

static inline int plf_con_getch_nowait(void)
{
    return uart0_sifive_getch_nowait();
}

static inline void plf_con_tx_flush(void)
{
    uart0_sifive_tx_flush();
}
#endif // PLF_UART0_SIFIVE
#endif // PLF_UART0_BASE || UART0_SIFIVE

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !__ASSEMBLER__
#endif // SCR_BSP_UART_SIFIVE_H
