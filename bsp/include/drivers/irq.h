/// @file
/// @brief Syntacore external interrupt primitives
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#ifndef SCR_BSP_IRQ_H
#define SCR_BSP_IRQ_H

#include "arch.h"

#if PLF_PLIC_BASE
#include "drivers/plic.h"

#define IRQ_TYPE_LEVEL_HIGH         PLIC_SRC_MODE_LEVEL_HIGH
#define IRQ_TYPE_LEVEL_LOW          PLIC_SRC_MODE_LEVEL_LOW
#define IRQ_TYPE_EDGE_RISING        PLIC_SRC_MODE_EDGE_RISING
#define IRQ_TYPE_EDGE_FALLING       PLIC_SRC_MODE_EDGE_FALLING

#define IRQ_NONE (0)

//----------------------
// generic IRQ routines
//----------------------

#ifdef __cplusplus
extern "C" {
#endif

static inline void irq_init(void)
{
    plic_init();
}

static inline int irq_setup(int line, int mode)
{
    return plic_irq_setup(line, mode);
}

static inline void irq_enable(int irq)
{
    plic_irq_enable(irq);
}

static inline void irq_disable(int irq)
{
    plic_irq_disable(irq);
}

static inline int irq_soi(void)
{
    return plic_soi();
}

static inline void irq_eoi(int line)
{
    plic_eoi(line);
}

#ifdef __cplusplus
}
#endif
#else  // PLF_PLIC_BASE
#include "drivers/ipic.h"

#define IRQ_TYPE_LEVEL_HIGH   (IPIC_IRQ_LEVEL)
#define IRQ_TYPE_LEVEL_LOW    (IPIC_IRQ_LEVEL | IPIC_IRQ_INV)
#define IRQ_TYPE_EDGE_RISING  (IPIC_IRQ_EDGE)
#define IRQ_TYPE_EDGE_FALLING (IPIC_IRQ_EDGE | IPIC_IRQ_INV)

#define IRQ_NONE (IPIC_IRQ_VEC_VOID)

//----------------------
// generic IRQ routines
//----------------------
#ifdef __cplusplus
extern "C" {
#endif

static inline void irq_init(void)
{
    ipic_init();
}

static inline int irq_setup(int line, int mode)
{
    return ipic_irq_setup(-1, line, mode, IPIC_IRQ_PRIV_MMODE);
}

static inline void irq_enable(int irq)
{
    ipic_irq_enable(irq);
}

static inline void irq_disable(int irq)
{
    ipic_irq_disable(irq);
}

static inline int irq_soi(void)
{
    return ipic_soi();
}

static inline void irq_eoi(int line)
{
    (void)line;
    ipic_eoi();
}

#ifdef __cplusplus
}
#endif
#endif // PLF_PLIC_BASE
#endif // SCR_BSP_IRQ_H
