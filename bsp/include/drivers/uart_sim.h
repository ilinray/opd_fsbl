/// @file
/// @brief UART defs and inline funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2022. All rights reserved.

#ifndef SCR_BSP_UART_SIM_H
#define SCR_BSP_UART_SIM_H

#include "arch.h"

#ifdef PLF_UART_SIM_BASE

#ifndef __ASSEMBLER__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// inlines
#define sim_con_init() do {} while (0)

static inline int sim_con_put(int data)
{
    *(volatile uint8_t*)PLF_UART_SIM_BASE = data;

    return data;
}

static inline int sim_con_putc(int data) { return sim_con_put(data); }

static inline int sim_con_getc(void) { return 0; }

static inline void sim_con_flush(void) {}

static inline int sim_con_getch_nowait(void) { return -1; } // no input

static inline void sim_con_tx_flush(void) {}

#ifdef PLF_SIM_CONSOLE
static inline void plf_con_init(void) { sim_con_init(); }

static inline void plf_con_put(int c) { sim_con_putc(c); }

static inline int plf_con_getch_nowait(void) { return sim_con_getc(); }

static inline void plf_con_tx_flush(void) { sim_con_flush(); }
#endif // PLF_SIM_CONSOLE

#ifdef __cplusplus
}
#endif //  __cplusplus
#endif // !__ASSEMBLER__
#endif // PLF_UART_SIM_BASE
#endif // SCR_BSP_UART_SIM_H
