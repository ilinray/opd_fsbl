/// @file
/// @brief lock defs and inlines
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2020. All rights reserved.

#ifndef SCR_BSP_LOCK_H
#define SCR_BSP_LOCK_H

#include "arch.h"

#ifdef PLF_SMP_SUPPORT

#include "drivers/cache.h"

#ifdef __cplusplus
extern "C" {
#endif

#if PLF_ATOMIC_SUPPORTED

typedef struct arch_lock {
    volatile int lock;
} arch_lock_t;

static inline int arch_is_locked(arch_lock_t *lock)
{
#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)&(lock->lock), sizeof(lock->lock));
#endif // PLF_SMP_NON_COHERENT
    return lock->lock != 0;
}

static inline void arch_unlock_wait(arch_lock_t *lock)
{
    do {
        cpu_relax();
    } while (arch_is_locked(lock));
}

#define ARCH_LOCK_INIT(i) {(i)}

static inline void arch_unlock(arch_lock_t *lock)
{
    __asm__ __volatile__ (
        "amoswap.w.rl x0, x0, %0"
        : "=A" (lock->lock)
        :: "memory");
}

static inline int arch_trylock(arch_lock_t *lock)
{
    int tmp = 1, busy;

    __asm__ __volatile__ (
        "amoswap.w.aq %0, %2, %1"
        : "=r" (busy), "+A" (lock->lock)
        : "r" (tmp)
        : "memory");

    return !busy;
}

static inline void arch_lock(arch_lock_t *lock)
{
    while (1) {
        if (arch_is_locked(lock))
            continue;

        if (arch_trylock(lock))
            break;
    }
}

#else // PLF_ATOMIC_SUPPORTED

/* 8-bit-per-hart mask size in xlen words */
#define PLF_SMP_HART8_MASK_SIZE ((((PLF_SMP_HART_NUM) * 8) + (__riscv_xlen - 1)) / __riscv_xlen)
#define PLF_SMP_HART8_XLEN (__riscv_xlen / 8)

typedef struct arch_lock {
    volatile unsigned long flags[PLF_SMP_HART8_MASK_SIZE];
} arch_lock_t;

#define ARCH_LOCK_INIT(i) {{(0)}}

void arch_unlock(arch_lock_t *lock);
void arch_lock(arch_lock_t *lock);
#endif // PLF_ATOMIC_SUPPORTED

#else // PLF_SMP_SUPPORT

typedef struct arch_lock {
} arch_lock_t;

#define arch_is_locked(x) (0)
#define arch_unlock_wait(x) do { } while (0)

#define ARCH_LOCK_INIT(i) {}

static inline void arch_unlock(arch_lock_t *lock)
{
    (void)lock;
}

static inline int arch_trylock(arch_lock_t *lock)
{
    (void)lock;

    return 1;
}

static inline void arch_lock(arch_lock_t *lock)
{
    (void)lock;
}

#ifdef __cplusplus
}
#endif

#endif // PLF_SMP_SUPPORT

#endif // SCR_BSP_LOCK_H
