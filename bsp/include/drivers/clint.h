/// @file
/// @brief SiFive CLINT
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2022. All rights reserved.

#ifndef SCR_BSP_CLINT_H
#define SCR_BSP_CLINT_H

#include "arch.h"

#ifdef PLF_CLINT_BASE

#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLINT_MTIMER_BASE (PLF_CLINT_BASE + 0xbff8)
#define CLINT_MTIMER_CMP_BASE (PLF_CLINT_BASE + 0x4000)

#if __riscv_xlen == 32
#define CLINT_MTIMER (*(volatile uint32_t*)(CLINT_MTIMER_BASE))
#define CLINT_MTIMERH (*(volatile uint32_t*)(CLINT_MTIMER_BASE + 4))
#define CLINT_MTIMER_CMP (*(volatile uint32_t*)(CLINT_MTIMER_CMP_BASE))
#define CLINT_MTIMER_CMPH (*(volatile uint32_t*)(CLINT_MTIMER_CMP_BASE + 4))
#else // __riscv_xlen == 32
#define CLINT_MTIMER (*(volatile uint64_t*)(CLINT_MTIMER_BASE))
#define CLINT_MTIMER_CMP (*(volatile uint64_t*)(CLINT_MTIMER_CMP_BASE))
#endif // __riscv_xlen == 32

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

static inline sys_tick_t clint_mtimer_current(void)
{
#if __riscv_xlen == 32
    uint32_t lo, hi, tmp;

    do {
        hi = CLINT_MTIMERH;
        lo = CLINT_MTIMER;
        tmp = CLINT_MTIMERH;
    } while (hi != tmp);

    return ((sys_tick_t)hi << 32) | lo;
#else //  __riscv_xlen == 32
    return (sys_tick_t)CLINT_MTIMER;
#endif // __riscv_xlen == 32
}

// basic functions

static inline void clint_mtimer_setcmp(sys_tick_t t)
{
#if __riscv_xlen == 32
    CLINT_MTIMER_CMPH = ~0;
    CLINT_MTIMER_CMP  = (uint32_t)t;
    CLINT_MTIMER_CMPH = (uint32_t)(t >> 32);
#else //  __riscv_xlen == 32
    CLINT_MTIMER_CMP  = t;
#endif // __riscv_xlen == 32
}

static inline void clint_mtimer_setcmp_offset(sys_tick_t to)
{
    clint_mtimer_setcmp(clint_mtimer_current() + to);
}

static inline void clint_mtimer_set(sys_tick_t t)
{
#if __riscv_xlen == 32
    CLINT_MTIMERH = 0;
    CLINT_MTIMER  = (uint32_t)t;
    CLINT_MTIMERH = (uint32_t)(t >> 32);
#else //  __riscv_xlen == 32
    CLINT_MTIMER  = t;
#endif // __riscv_xlen == 32
}

static inline void clint_mtimer_reset(void)
{
    // reset counter and cmp
    clint_mtimer_setcmp(~0L);
    clint_mtimer_set(0);
}

static inline void clint_mtimer_enable(void)
{
}

static inline void clint_mtimer_disable(void)
{
}

static inline void clint_mtimer_init(void)
{
    clint_mtimer_reset();
    clint_mtimer_enable();
}

#pragma GCC diagnostic pop

#ifdef __cplusplus
}
#endif

#endif // PLF_CLINT_BASE
#endif // SCR_BSP_CLINT_H
