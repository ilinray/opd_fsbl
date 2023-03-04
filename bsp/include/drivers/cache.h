/// @file
/// @brief Cache CSR's defs and inlines
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2019. All rights reserved.

#ifndef SCR_BSP_CACHE_H
#define SCR_BSP_CACHE_H

#ifdef INSN_NOT_SUPPORTED
#include "asm-magic.h"
#endif // INSN_NOT_SUPPORTED
#include "csr.h"

// cache control CSRs
#define SCR_CSR_CACHE_GLBL 0xbd4
// cache info CSRs
#define SCR_CSR_CACHE_DSCR_L1 0xfc3

// global cache's control bits
#define CACHE_GLBL_L1I_EN (1 << 0)
#define CACHE_GLBL_L1D_EN (1 << 1)
#define CACHE_GLBL_L1I_INV (1 << 2)
#define CACHE_GLBL_L1D_INV (1 << 3)
#define CACHE_GLBL_ENABLE (CACHE_GLBL_L1I_EN | CACHE_GLBL_L1D_EN)
#define CACHE_GLBL_DISABLE 0
#define CACHE_GLBL_INV (CACHE_GLBL_L1I_INV | CACHE_GLBL_L1D_INV)

#ifdef INSN_NOT_SUPPORTED
#define ENC_CLINV(regn) (0x10800073 | (((regn) & 0x1f) << 15))
// instruction: clflush <regn> (cache line flush & invalidate)
#define ENC_CLFLUSH(regn) (0x10900073 | (((regn) & 0x1f) << 15))
#endif // INSN_NOT_SUPPORTED

#ifndef __ASSEMBLER__

#ifdef __GNUC__

#include "arch.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline bool cache_l1_available(void)
{
    return read_csr(SCR_CSR_CACHE_DSCR_L1) != 0;
}

// setup global cache policy
static inline void cache_l1_ctrl(unsigned long ctrl_val)
{
    if (cache_l1_available()) {
        write_csr(SCR_CSR_CACHE_GLBL, ctrl_val);
        ifence();
    }
}

// setup global cache policy
static inline bool cache_l1_enabled(void)
{
    if (!cache_l1_available())
        return false;
    return (read_csr(SCR_CSR_CACHE_GLBL) & (CACHE_GLBL_L1I_EN | CACHE_GLBL_L1D_EN)) != 0;
}

static inline void cache_l1_enable(void)
{
    cache_l1_ctrl(CACHE_GLBL_ENABLE);
}

static inline void cache_l1_disable(void)
{
    if (cache_l1_available()) {
        write_csr(SCR_CSR_CACHE_GLBL, CACHE_GLBL_DISABLE | CACHE_GLBL_INV);
        ifence();
        // wait until invalidation complete
        while (read_csr(SCR_CSR_CACHE_GLBL) & (CACHE_GLBL_INV))
            fence();
    }
}

static inline void plf_l1cache_init(void)
{
#ifdef PLF_CACHE_CFG
    // setup global cache policy
    cache_l1_ctrl(PLF_CACHE_CFG);
#endif // PLF_CACHE_CFG
}

int cache_l1i_info(char *buf, size_t len);
int cache_l1d_info(char *buf, size_t len);

static inline void cache_invalidate(void *vaddr, long size)
{
    fence();

#if PLF_CACHELINE_SIZE
    if (PLF_CACHELINE_SIZE && size) {
        register uintptr_t a0 = (uintptr_t)vaddr;
        do {
#ifdef INSN_NOT_SUPPORTED
            asm volatile(".equ code_clinv, "_TOSTR(ENC_CLINV(10))";"
                         "mv a0, %0; .word code_clinv;" ::"r"(a0) : "a0");
#else
            asm volatile(".insn i SYSTEM, 0, zero, %[rs1], 0b000100001000"
                         :: [rs1]"r"(a0) : "memory");
#endif // INSN_NOT_SUPPORTED
            a0 += PLF_CACHELINE_SIZE;
            size -= PLF_CACHELINE_SIZE;
        } while (size > 0);
    }
#else
    (void)vaddr;
    (void)size;
#endif // PLF_CACHELINE_SIZE
}

static inline void cache_flush(void *vaddr, long size)
{
#if PLF_CACHELINE_SIZE
    if (PLF_CACHELINE_SIZE && size) {
        register uintptr_t a0 = (uintptr_t)vaddr;
        /* Flush the cache for the requested range */
        do {
#ifdef INSN_NOT_SUPPORTED
            asm volatile(".equ code_clflush, "_TOSTR(ENC_CLFLUSH(10))";"
                         "mv a0, %0; .word code_clflush;" ::"r"(a0) : "a0");
#else
            asm volatile(".insn i SYSTEM, 0, zero, %[rs1], 0b000100001001"
                         :: [rs1]"r"(a0) : "memory");
#endif // INSN_NOT_SUPPORTED
            a0 += PLF_CACHELINE_SIZE;
            size -= PLF_CACHELINE_SIZE;
        } while (size > 0);
    }
#else
    (void)vaddr;
    (void)size;
#endif // PLF_CACHELINE_SIZE

    fence();
}

/* L2$ routines  */
#if PLF_L2CTL_BASE
void plf_l2cache_enable(void);
void plf_l2cache_disable(void);
bool plf_l2cache_is_enabled(void);
void plf_l2cache_init(void);
#else /* PLF_L2CTL_BASE */
#define plf_l2cache_enable() do {} while (0)
#define plf_l2cache_disable() do {} while (0)
#define plf_l2cache_init() do {} while (0)
#define plf_l2cache_is_enabled() (false)
#endif /* PLF_L2CTL_BASE */
int cache_l2_info(char *buf, size_t len);

#ifdef __cplusplus
}
#endif
#endif // __GNUC__
#else // !__ASSEMBLER__

.altmacro

// assembler macros

#ifndef INSN_NOT_SUPPORTED
// cache line flush
.macro clflush reg
    .insn i SYSTEM, 0, zero, \reg, 0b000100001001
.endm

// cache line invalidate
.macro clinval reg
    .insn i SYSTEM, 0, zero, \reg, 0b000100001000
.endm
#endif // INSN_NOT_SUPPORTED

// reset L1 and disable caching
.macro cache_reset_nc
#ifdef PLF_CACHE_CFG
    // setup caches: disabled, flush&invalidate
    LOCAL cache_inv_wait, cache_inv_skip
    csrr t0, SCR_CSR_CACHE_DSCR_L1
    beqz t0, cache_inv_skip
    csr_write_imm SCR_CSR_CACHE_GLBL, %(CACHE_GLBL_DISABLE | CACHE_GLBL_INV)
    fence.i
    // wait until invalidation complete
cache_inv_wait:
    csrr t0, SCR_CSR_CACHE_GLBL
    andi t0, t0, CACHE_GLBL_INV
    bnez t0, cache_inv_wait
    fence.i
cache_inv_skip:
#endif // PLF_CACHE_CFG
.endm

.macro cache_flush addr, size
#if PLF_CACHELINE_SIZE
    LOCAL cache_flush_loop
cache_flush_loop:
    clflush \addr
    addi \size, \size, -PLF_CACHELINE_SIZE
    addi \addr, \addr, PLF_CACHELINE_SIZE
    bgez \size, cache_flush_loop
#endif // PLF_CACHELINE_SIZE
.endm // cache_l1l2_flush

#endif // !__ASSEMBLER__
#endif // SCR_BSP_CACHE_H
