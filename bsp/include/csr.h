/// @file
/// @brief Architecture specific CSR's defs and inlines
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2019. All rights reserved.

#ifndef SCR_BSP_CSR_H
#define SCR_BSP_CSR_H

#ifndef __ASSEMBLER__

#include "stringify.h"

#define swap_csr(reg, val)                      \
({                                              \
    unsigned long __tmp = (unsigned long)(val); \
    asm volatile ("csrrw %0," _TOSTR(reg) ",%1" \
                  : "=r" (__tmp) : "rK" (__tmp) \
                  : "memory");                  \
    __tmp;                                      \
})

#define read_csr(reg)                           \
({                                              \
    unsigned long __tmp;                        \
    asm volatile ("csrr %0," _TOSTR(reg)        \
                  : "=r" (__tmp) :              \
                  : "memory");                  \
    __tmp;                                      \
})

#define write_csr(reg, val)                     \
({                                              \
    unsigned long __tmp = (unsigned long)(val); \
    asm volatile ("csrw " _TOSTR(reg) ",%0"     \
                  : : "rK" (__tmp)              \
                  : "memory");                  \
})

#define set_csr(reg, val)                       \
({                                              \
    unsigned long __tmp = (unsigned long)(val); \
    asm volatile ("csrrs %0," _TOSTR(reg) ",%1" \
                  : "=r" (__tmp) : "rK" (__tmp) \
                  : "memory");                  \
    __tmp;                                      \
})

#define clear_csr(reg, val)                     \
({                                              \
    unsigned long __tmp = (unsigned long)(val); \
    asm volatile ("csrrc %0," _TOSTR(reg) ",%1" \
                  : "=r" (__tmp) : "rK" (__tmp) \
                  : "memory");                  \
    __tmp;                                      \
})

#if __riscv_xlen == 32
#define read_csr64(reg)                                                               \
({                                                                                    \
    uint32_t lo, hi, tmp;                                                             \
                                                                                      \
    asm volatile("1:"                                                                 \
                 "csrr %[hi], " _TOSTR(_CONCAT(reg,h)) ";"                                \
                 "csrr %[lo], " _TOSTR(reg) ";"                                           \
                 "csrr %[tmp]," _TOSTR(_CONCAT(reg,h)) ";"                                \
                 "bne  %[hi], %[tmp], 1b"                                             \
                 : [ hi ] "=&r"(hi), [ lo ] "=&r"(lo), [ tmp ] "=&r"(tmp)::"memory"); \
                                                                                      \
    ((uint64_t)hi << 32) | lo;                                                        \
})
#endif // __riscv_xlen == 32

#else // !__ASSEMBLER__

// assembler macros

.altmacro

.macro csr_write_imm csr_reg, val
    .if \val == 0
    csrw \csr_reg, zero
    .elseif (\val / 32) == 0
    csrwi \csr_reg, \val
    .else
    li   t0, \val
    csrw \csr_reg, t0
    .endif
.endm

#endif // !__ASSEMBLER__
#endif // SCR_BSP_CSR_H
