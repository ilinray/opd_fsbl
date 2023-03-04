/// @file
/// @brief Syntacore MTimer
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2022. All rights reserved.

#ifndef SCR_BSP_MTIMER_H
#define SCR_BSP_MTIMER_H

#include "arch.h"

#ifndef PLF_MTIMER_BASE
#error Unknown PLF_MTIMER_BASE
#endif

// MTimer control register bits
#define SCR_MTIMER_CTRL_EN (1 << 0)
#define SCR_MTIMER_CTRL_SI (0 << 1)
#define SCR_MTIMER_CTRL_SE (1 << 1)

#define SCR_PLF_TIMEBASE_DIV ((PLF_SYS_CLK) / (PLF_RTC_TIMEBASE) - 1)

#ifdef PLF_RTC_SRC_EXTERNAL
#define SCR_PLF_RTC_SRC SCR_MTIMER_CTRL_SE
#else
#define SCR_PLF_RTC_SRC SCR_MTIMER_CTRL_SI
#endif

#define SCR_RTC_CTL_OFF   0
#define SCR_RTC_DIV_OFF   4
#define SCR_RTC_TIME_OFF  8
#define SCR_RTC_TIMEH_OFF 12
#define SCR_RTC_CMP_OFF   16
#define SCR_RTC_CMPH_OFF  20

#define SCR_RTC_CTL       (PLF_MTIMER_BASE+SCR_RTC_CTL_OFF)
#define SCR_RTC_DIVIDER   (PLF_MTIMER_BASE+SCR_RTC_DIV_OFF)
#define SCR_RTC_MTIME     (PLF_MTIMER_BASE+SCR_RTC_TIME_OFF)
#define SCR_RTC_MTIMEH    (PLF_MTIMER_BASE+SCR_RTC_TIMEH_OFF)
#define SCR_RTC_MTIMECMP  (PLF_MTIMER_BASE+SCR_RTC_CMP_OFF)
#define SCR_RTC_MTIMECMPH (PLF_MTIMER_BASE+SCR_RTC_CMPH_OFF)
#define SCR_RTC_CTL_EN (1 << 0)
#define SCR_RTC_CTL_INTERNAL_SRC (0 << 1)
#define SCR_RTC_CTL_EXTERNAL_SRC (1 << 1)

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scr_mtimer_struct {
    uint32_t ctrl;
    uint32_t div;
#if __riscv_xlen == 32
    uint32_t time;
    uint32_t timeh;
    uint32_t cmp;
    uint32_t cmph;
#else // __riscv_xlen  == 32
    uint64_t time;
    uint64_t cmp;
#endif // __riscv_xlen == 32
} volatile scr_mtimer;

#define SCR_MTIMER (*(scr_mtimer*)(PLF_MTIMER_BASE))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

static inline sys_tick_t scr_mtimer_current(void)
{
#if __riscv_xlen == 32
    return read_csr64(time);
#else //  __riscv_xlen == 32
    return read_csr(time);
#endif // __riscv_xlen == 32
}

// basic functions

static inline void scr_mtimer_setcmp(sys_tick_t t)
{
#if __riscv_xlen == 32
    SCR_MTIMER.cmph = ~0;
    SCR_MTIMER.cmp  = (uint32_t)t;
    SCR_MTIMER.cmph = (uint32_t)(t >> 32);
#else //  __riscv_xlen == 32
    SCR_MTIMER.cmp  = t;
#endif //  __riscv_xlen == 32
}

static inline void scr_mtimer_setcmp_offset(sys_tick_t to)
{
    scr_mtimer_setcmp(scr_mtimer_current() + to);
}

static inline void scr_mtimer_set(sys_tick_t t)
{
#if __riscv_xlen == 32
    SCR_MTIMER.timeh = 0;
    SCR_MTIMER.time  = (uint32_t)t;
    SCR_MTIMER.timeh = (uint32_t)(t >> 32);
#else //  __riscv_xlen == 32
    SCR_MTIMER.time  = t;
#endif //  __riscv_xlen == 32
}

static inline void scr_mtimer_reset(void)
{
    // reset counter and cmp, set divisor
    SCR_MTIMER.ctrl  = 0;
    SCR_MTIMER.div   = SCR_PLF_TIMEBASE_DIV;
    SCR_MTIMER.time  = 0;
#if __riscv_xlen == 32
    SCR_MTIMER.timeh = 0;
#endif // __riscv_xlen == 32
    SCR_MTIMER.cmp   = ~0;
#if __riscv_xlen == 32
    SCR_MTIMER.cmph  = ~0;
#endif // __riscv_xlen == 32
}

static inline void scr_mtimer_enable(void)
{
    SCR_MTIMER.ctrl = SCR_MTIMER_CTRL_EN | SCR_PLF_RTC_SRC;
}

static inline void scr_mtimer_disable(void)
{
    SCR_MTIMER.ctrl = 0;
}

static inline void scr_mtimer_init(void)
{
    scr_mtimer_reset();
    scr_mtimer_enable();
}

#pragma GCC diagnostic pop

#ifdef __cplusplus
}
#endif

#endif // SCR_BSP_MTIMER_H
