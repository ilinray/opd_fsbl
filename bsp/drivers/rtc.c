/// @file
/// @brief generic RTC funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#include "drivers/rtc.h"

#include <stdio.h>

int rtc_time2str(char *buf, size_t len, sys_tick_t t)
{
    int ssz = 0;

    unsigned long s = rtc_ticks2sec(t);
    unsigned long m = s / 60;
    unsigned long h = m / 60;
    unsigned long d = h / 24;

    h %= 24;
    m %= 60;
    s %= 60;

    if (d) {
        ssz = snprintf(buf, len, "%lu day(s), ", d);
    }
    if (ssz >= 0 && (size_t)ssz < len) {
        ssz += snprintf(buf + ssz, len - (size_t)ssz, "%lu:%s%lu:%s%lu",
                        h, (m < 10 ? "0" : ""), m, (s < 10 ? "0" : ""), s);
    }

    return ssz;
}

#ifndef __llvm__
__attribute__((optimize("O2", "no-unroll-loops")))
#endif
unsigned long rtc_dyn_hart_clock_rate(void)
{
    unsigned long c0 = arch_cycle();
    sys_tick_t t = rtc_now();
    const sys_tick_t ticks = PLF_MS2TICKS(125);

#ifdef __llvm__
    #pragma clang loop unroll(disable)
#endif
    do {
        // reduce the number of rtc_now() calls
        for (int i = 0; i < 64; ++i) {
            cpu_relax();
            cpu_relax();
        }
    } while ((rtc_now() - t) < ticks);

    return (arch_cycle() - c0) * 8;
}
