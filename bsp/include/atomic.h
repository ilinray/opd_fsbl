/// @file
/// @brief Atomic primitives defs and inlines
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#ifndef SCR_BSP_ATOMIC_H
#define SCR_BSP_ATOMIC_H

#include "arch.h"
#include "drivers/cache.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct arch_atomic {
    int counter;
} arch_atomic_t;

#define ARCH_ATOMIC_INIT(i) {(i)}

static inline int atomic_read(const arch_atomic_t *v)
{
#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)&(v->counter), sizeof(v->counter));
#endif // PLF_SMP_NON_COHERENT
    return *((volatile int *)(&(v->counter)));
}

static inline void atomic_set(arch_atomic_t *v, int i)
{
    *((volatile int *)(&(v->counter))) = i;
#if PLF_SMP_NON_COHERENT
    cache_flush((void*)&(v->counter), sizeof(v->counter));
#endif // PLF_SMP_NON_COHERENT
}

#if PLF_ATOMIC_SUPPORTED

static inline int atomic_add(int i, arch_atomic_t *v)
{
    int out;

    __asm__ __volatile__ (
        "amoadd.w %1, %2, %0"
        : "+A" (v->counter), "=r" (out)
        : "r" (i) : "memory");
    return out;
}

#else

static inline int atomic_add(int i, arch_atomic_t *v)
{
    int out;

    out = atomic_read(v);
    atomic_set(v, out + i);

    return out;
}

#endif // PLF_ATOMIC_SUPPORTED

#ifdef __cplusplus
}
#endif
#endif // SCR_BSP_ATOMIC_H
