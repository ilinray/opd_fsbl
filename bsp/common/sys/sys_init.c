/// @file
/// @brief platform init funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2019. All rights reserved.

#include "drivers/cache.h"
#include "drivers/console.h"
#include "drivers/leds.h"
#include "drivers/mpu.h"
#include "drivers/rtc.h"
#include "memasm.h"
#include "perf.h"

#include <string.h>
#include <sys/errno.h>
#include <unistd.h> /* sbrk() */

extern char _tdata_start[];
extern char _tdata_end[];
extern char _tbss_start[];
extern char _tbss_end[];

// init hart local storage
static void plf_init_tls(void* tls)
{
    // copy tls data
    memcpy(tls, _tdata_start, (size_t)(_tdata_end - _tdata_start));
    // init tls bss
    memset((char*)tls + (_tbss_start - _tdata_start), 0, (size_t)(_tbss_end - _tbss_start));
#if PLF_SMP_NON_COHERENT
    cache_flush(tls, _tbss_end - _tdata_start);
#endif // PLF_SMP_NON_COHERENT
}

#if PLF_SMP_SUPPORT
volatile int plf_smp_sync_var = 0;

void plf_smp_slave_init(void)
{
    plf_l1cache_init();
    mpu_init();

    do
    {
#if PLF_SMP_NON_COHERENT
        cache_invalidate((void*)&plf_smp_sync_var, sizeof(plf_smp_sync_var));
#else  // PLF_SMP_NON_COHERENT
        fence();
#endif // PLF_SMP_NON_COHERENT
    } while (!plf_smp_sync_var);
}

// allocate stack and TLS, init TLS
// return init value of SP/TP
void* plf_alloc_thread_generic(void)
{
    extern char __TLS_SIZE__[];

    unsigned long tls_size = (unsigned long)__TLS_SIZE__;
    unsigned long mem_size = tls_size + 16 + PLF_HLS_MIN_STACK_SIZE;

    void* hls_base = sbrk((ptrdiff_t)mem_size);

    if (hls_base == (void*)-ENOMEM)
        return hls_base;

    void* base = (char*)hls_base + mem_size - tls_size;
    // align by 16
    base = (void*)((uintptr_t)base & (uintptr_t)-0x10L);

    return base;
}
void* plf_alloc_thread(void) __attribute__((weak, alias("plf_alloc_thread_generic")));

void smp_slave_entry_default(void) { _hart_halt(); }
void PLF_SMP_SLAVE_ENTRY(void) __attribute__((weak, alias("smp_slave_entry_default")));

volatile void* hart_start_table[PLF_SMP_HART_NUM] = {(void*)~0};

#endif // PLF_SMP_SUPPORT

static void plf_smp_init(void)
{
#if PLF_SMP_SUPPORT
    for (size_t i = 0; i < sizeof(hart_start_table) / sizeof(*hart_start_table); ++i)
    {
        if (i != PLF_SMP_MASTER_HARTID - PLF_SMP_HARTID_BASE)
        {
            void* hls = plf_alloc_thread();
            if (hls == (void*)-ENOMEM)
                break;
            plf_init_tls(hls);
            hart_start_table[i] = hls;
        }
    }

    cache_flush(hart_start_table, sizeof(hart_start_table));
#endif // PLF_SMP_SUPPORT
}

extern void plf_init_relocate(void) __attribute__((weak));

extern char __bss_start[], __bss_end[];

void __init plf_init_noreloc(void)
{
    // do nothing
}

void __init plf_init_generic(void)
{
    // do relocations
    plf_init_relocate();
    // init TLS
    register void* tp asm("tp");
    plf_init_tls(tp);
    // init BSS
    memset(__bss_start, 0, (size_t)(__bss_end - __bss_start));
    // enable/init caches
    plf_l2cache_init();
    plf_l1cache_init();
    // enable/init MPU
    mpu_init();
    // init peripheries
    con_init();
    rtc_init();
#ifdef BSP_USE_LEDS
    leds_init();
#endif

    // init smp
    plf_smp_init();

#ifdef NANOBENCH
    perf_cfg_br_inst();
    perf_cfg_br_miss();
    perf_cfg_l1d_hit();
    perf_cfg_l1d_miss();

    perf_init_hpm();
#endif
}

void plf_init(void) __attribute__((weak, alias("plf_init_generic")));
