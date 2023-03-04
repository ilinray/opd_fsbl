/// @file
/// @brief Syntacore IPIC defs and inline funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#ifndef SCR_BSP_IPIC_H
#define SCR_BSP_IPIC_H

#include "arch.h"

// flags
#define IPIC_IRQ_PENDING           (1 << 0)
#define IPIC_IRQ_ENABLE            (1 << 1)
#define IPIC_IRQ_LEVEL             (0 << 2)
#define IPIC_IRQ_EDGE              (1 << 2)
#define IPIC_IRQ_INV               (1 << 3)
#define IPIC_IRQ_MODE_MASK         (3 << 2)
#define IPIC_IRQ_CLEAR_PENDING     IPIC_IRQ_PENDING

#define IPIC_IRQ_IN_SERVICE (1 << 4) // RO
#define IPIC_IRQ_PRIV_MASK  (3 << 8)
#define IPIC_IRQ_PRIV_MMODE (3 << 8)
#define IPIC_IRQ_PRIV_SMODE (1 << 8)
#define IPIC_IRQ_LN_OFFS    12

#ifndef PLF_IPIC_STATIC_LINE_MAPPING
#define PLF_IPIC_STATIC_LINE_MAPPING ((PLF_CORE_VARIANT_SCR) == 1)
#endif // PLF_IPIC_STATIC_LINE_MAPPING

#ifndef PLF_IPIC_IRQ_LN_NUM
#if (PLF_CORE_VARIANT_SCR) == 1
#define PLF_IPIC_IRQ_LN_NUM     16
#else
#define PLF_IPIC_IRQ_LN_NUM     32
#endif
#endif // PLF_IPIC_IRQ_LN_NUM

#ifndef PLF_IPIC_IRQ_VEC_NUM
#if PLF_IPIC_STATIC_LINE_MAPPING
#define PLF_IPIC_IRQ_VEC_NUM    PLF_IPIC_IRQ_LN_NUM
#else // PLF_IPIC_STATIC_LINE_MAPPING
#define PLF_IPIC_IRQ_VEC_NUM    32
#endif // PLF_IPIC_STATIC_LINE_MAPPING
#endif // PLF_IPIC_IRQ_VEC_NUM

#define IPIC_IRQ_LN_VOID    PLF_IPIC_IRQ_LN_NUM
#define IPIC_IRQ_VEC_VOID   PLF_IPIC_IRQ_VEC_NUM

// IPIC regs
// M-Mode
#ifndef PLF_IPIC_MBASE
#define PLF_IPIC_MBASE   0xbf0
#endif
#define IPIC_CISV   (PLF_IPIC_MBASE + 0)
#define IPIC_CICSR  (PLF_IPIC_MBASE + 1)
#define IPIC_IPR    (PLF_IPIC_MBASE + 2)
#define IPIC_ISVR   (PLF_IPIC_MBASE + 3)
#define IPIC_EOI    (PLF_IPIC_MBASE + 4)
#define IPIC_SOI    (PLF_IPIC_MBASE + 5)
#define IPIC_IDX    (PLF_IPIC_MBASE + 6)
#define IPIC_ICSR   (PLF_IPIC_MBASE + 7)
#define IPIC_IER    (PLF_IPIC_MBASE + 8)
#define IPIC_IMAP   (PLF_IPIC_MBASE + 9)
// S-Mode
#if PLF_HAVE_MMU
#ifndef PLF_IPIC_SBASE
#define PLF_IPIC_SBASE   0x9f0
#endif
#define IPIC_SCISV  (PLF_IPIC_SBASE + 0)
#define IPIC_SCICSR (PLF_IPIC_SBASE + 1)
#define IPIC_SIPR   (PLF_IPIC_SBASE + 2)
#define IPIC_SISVR  (PLF_IPIC_SBASE + 3)
#define IPIC_SEOI   (PLF_IPIC_SBASE + 4)
#define IPIC_SSOI   (PLF_IPIC_SBASE + 5)
#define IPIC_SIDX   (PLF_IPIC_SBASE + 6)
#define IPIC_SICSR  (PLF_IPIC_SBASE + 7)
#define IPIC_SIER   (PLF_IPIC_SBASE + 8)
#define IPIC_SIMAP  (PLF_IPIC_SBASE + 9)
#endif

#define MK_IRQ_CFG(line, mode, flags) ((mode) | (flags) | ((line) << IPIC_IRQ_LN_OFFS))

#ifndef __ASSEMBLER__

//----------------------
// IPIC interface routines
//----------------------

#include "csr.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline int ipic_irq_setup(int irq_vec, int line, int mode, int flags)
{
    if ((PLF_IPIC_STATIC_LINE_MAPPING) || irq_vec < 0)
        irq_vec = line;
    write_csr(IPIC_IDX, irq_vec);
    write_csr(IPIC_ICSR, MK_IRQ_CFG(line, mode, flags | IPIC_IRQ_CLEAR_PENDING));

    return irq_vec;
}

static inline void ipic_irq_reset(int irq_vec)
{
    ipic_irq_setup(irq_vec, IPIC_IRQ_LN_VOID, IPIC_IRQ_PRIV_MMODE, IPIC_IRQ_CLEAR_PENDING);
}

static inline void ipic_init(void)
{
    // configure IPIC
    for (int i = 0; i < PLF_IPIC_IRQ_VEC_NUM; ++i)
        ipic_irq_reset(i);
}

static inline unsigned long ipic_irq_vect_get(unsigned irq_vec)
{
    write_csr(IPIC_IDX, irq_vec);
    return read_csr(IPIC_ICSR);
}

static inline unsigned long ipic_irq_current_vector(void)
{
    return read_csr(IPIC_CISV);
}

static inline unsigned long ipic_irq_current_get(void)
{
    return read_csr(IPIC_CICSR);
}

static inline void ipic_irq_current_set(unsigned long val)
{
    write_csr(IPIC_CICSR, val);
}

static inline void ipic_irq_vect_set(unsigned irq_vec, unsigned long val)
{
    write_csr(IPIC_IDX, irq_vec);
    write_csr(IPIC_ICSR, val);
}

static inline unsigned long ipic_soi(void)
{
    write_csr(IPIC_SOI, 0);
    return ipic_irq_current_vector();
}

static inline void ipic_eoi(void)
{
    write_csr(IPIC_EOI, 0);
}

static inline void ipic_irq_enable(unsigned irq_vec)
{
    write_csr(IPIC_IDX, irq_vec);
    unsigned long state = (read_csr(IPIC_ICSR) & ~IPIC_IRQ_PENDING) | IPIC_IRQ_ENABLE;
    write_csr(IPIC_ICSR, state);
}

static inline void ipic_irq_disable(unsigned irq_vec)
{
    write_csr(IPIC_IDX, irq_vec);
    unsigned long state = read_csr(IPIC_ICSR) & ~(IPIC_IRQ_ENABLE | IPIC_IRQ_PENDING);
    write_csr(IPIC_ICSR, state);
}

static inline void ipic_irq_clear_pending(unsigned irq_vec)
{
    write_csr(IPIC_IDX, irq_vec);
    unsigned long state = read_csr(IPIC_ICSR) | IPIC_IRQ_CLEAR_PENDING;
    write_csr(IPIC_ICSR, state);
}

static inline unsigned long ipic_pending(void)
{
    return read_csr(IPIC_IPR);
}

static inline unsigned long ipic_inservice(void)
{
    return read_csr(IPIC_ISVR);
}

static inline unsigned long ipic_mmapping(void)
{
    return read_csr(IPIC_IMAP);
}

static inline unsigned long ipic_mier(void)
{
    return read_csr(IPIC_IER);
}

static inline void ipic_mier_set(unsigned long val)
{
    write_csr(IPIC_IER, val);
}

#if PLF_HAVE_MMU
static inline unsigned long ipic_smapping(void)
{
    return read_csr(IPIC_SIMAP);
}

static inline unsigned long ipic_sier(void)
{
    return read_csr(IPIC_SIER);
}

static inline void ipic_sier_set(unsigned long val)
{
    write_csr(IPIC_SIER, val);
}
#endif // PLF_HAVE_MMU

#ifdef __cplusplus
}
#endif
#endif // !__ASSEMBLER__
#endif // SCR_BSP_IPIC_H
