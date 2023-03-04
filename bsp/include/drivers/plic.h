/// @file
/// @brief PLIC defs and inline funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2020. All rights reserved.

#ifndef SCR_BSP_PLIC_H
#define SCR_BSP_PLIC_H

#include "arch.h"

#ifndef PLF_PLIC_OFFSET_SRC_PRIO
#define PLF_PLIC_OFFSET_SRC_PRIO (0x0)
#endif
#ifndef PLF_PLIC_OFFSET_SRC_IP
#define PLF_PLIC_OFFSET_SRC_IP   (0x1000)
#endif
#ifndef PLF_PLIC_OFFSET_TGT_IE
#define PLF_PLIC_OFFSET_TGT_IE   (0x2000)
#endif
#ifndef PLF_PLIC_OFFSET_SRC_MODE
#define PLF_PLIC_OFFSET_SRC_MODE (0x1f0000)
#endif
#ifndef PLF_PLIC_OFFSET_TGT_CFG
#define PLF_PLIC_OFFSET_TGT_CFG  (0x200000)
#endif

#define PLIC_SRC_MODE_OFF           0
#define PLIC_SRC_MODE_LEVEL_HIGH    1
#define PLIC_SRC_MODE_LEVEL_LOW     2
#define PLIC_SRC_MODE_EDGE_RISING   3
#define PLIC_SRC_MODE_EDGE_FALLING  4
#define PLIC_SRC_MODE_EDGE_BOTH     5
#define PLIC_SRC_MODE_MAX           PLIC_SRC_MODE_EDGE_BOTH

// source line priority (0 - line disabled, 1 - min, 7 - max)
#define PLIC_SRC_PRIO_OFF 0
#define PLIC_SRC_PRIO_MIN 1
#define PLIC_SRC_PRIO_MAX 7
#define PLIC_SRC_PRIO_DFL 3

// target priority threshold (0 - permits all, 7 - disable all)
#define PLIC_TGT_PRIO_MIN 0
#define PLIC_TGT_PRIO_MAX 7
#define PLIC_TGT_PRIO_DFL PLIC_TGT_PRIO_MIN

#ifndef PLF_PLIC_LINE_NUM
#define PLF_PLIC_LINE_NUM 1023
#endif

#ifndef PLF_PLIC_TARGET_NUM
// typical targets mapping:
// M-mode only (MCU) cores: 1 target line per HART
// S-mode (with MMU) cores: 2 target lines per HART
#if PLF_SMP_SUPPORT
#if PLF_HAVE_MMU
#define PLF_PLIC_TARGET_NUM (PLF_SMP_HART_NUM * 2) // HARTs with 2 target lines (M-mode + S-mode)
#else
#define PLF_PLIC_TARGET_NUM (PLF_SMP_HART_NUM) // HARTs with M-mode targets lines
#endif // PLF_HAVE_MMU
#else
#define PLF_PLIC_TARGET_NUM (1) // single HART with 1 target line (M-mode)
#endif // PLF_SMP_SUPPORT
#endif // PLF_PLIC_TARGET_NUM

#define PLF_PLIC_BANK_NUM (((PLF_PLIC_LINE_NUM) + 31) / 32)

/* #define IRQ_PRIV_MMODE      0 */
/* #define IRQ_PRIV_SMODE      1 */

#if PLF_PLIC_BASE

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdarg.h>

#define PLIC_REGS_OFFSET(offs) ((volatile uint32_t*)((PLF_PLIC_BASE) + (offs)))

#define PLIC_SRC_PRIO (PLIC_REGS_OFFSET(PLF_PLIC_OFFSET_SRC_PRIO))
#define PLIC_SRC_IP (PLIC_REGS_OFFSET(PLF_PLIC_OFFSET_SRC_IP))
#define PLIC_SRC_MODE (PLIC_REGS_OFFSET(PLF_PLIC_OFFSET_SRC_MODE))
#define PLIC_TGT_IE (PLIC_REGS_OFFSET(PLF_PLIC_OFFSET_TGT_IE))
#define PLIC_TGT_CFG (PLIC_REGS_OFFSET(PLF_PLIC_OFFSET_TGT_CFG))

// current target mapping
#ifndef PLF_PLIC_CONTEXT
// typical HART to target mapping:
// M-mode only (MCU) cores: 1 target line per HART
// S-mode (with MMU) cores: 2 target lines per HART
#if PLF_SMP_SUPPORT
#if PLF_HAVE_MMU
#define PLF_PLIC_CONTEXT (arch_hartid() * 2) // HARTs with 2 target lines (M-mode + S-mode)
#else
#define PLF_PLIC_CONTEXT (arch_hartid()) // HARTs with M-mode targets lines
#endif // PLF_HAVE_MMU
#else
#define PLF_PLIC_CONTEXT (0) // single HART with 1 target line (M-mode)
#endif // PLF_SMP_SUPPORT
#endif // !PLF_PLIC_CONTEXT

//----------------------
// PLIC interface routines
//----------------------

#ifdef __cplusplus
extern "C" {
#endif

static inline void plic_line_config(int line, int prio, int mode)
{
    PLIC_SRC_MODE[line] = mode;
    PLIC_SRC_PRIO[line] = prio;
}

// setup target interrupt threshold
// PLIC_TGT_PRITH_MIN - permits all
// PLIC_TGT_PRITH_MAX - disable all
static inline void plic_target_setup(int tgt, int prioth)
{
    PLIC_TGT_CFG[tgt * 1024] = prioth;
}

static inline void plic_target_line_enable(int tgt, int line)
{
    volatile uint32_t *bits = &PLIC_TGT_IE[tgt * 32 + (line / 32)];
    *bits |= (1 << (line % 32));
}

static inline void plic_target_line_disable(int tgt, int line)
{
    volatile uint32_t *bits = &PLIC_TGT_IE[tgt * 32 + (line / 32)];
    *bits &= ~(1 << (line % 32));
}

static inline int plic_target_claim(int tgt)
{
    return (int)PLIC_TGT_CFG[tgt * 1024 + 1];
}

static inline void plic_target_complete(int tgt, int id)
{
    PLIC_TGT_CFG[tgt * 1024 + 1] = (uint32_t)id;
}

static inline void plic_init(void)
{
    // init targets
    for (int tgt = 0; tgt < PLF_PLIC_TARGET_NUM; ++tgt) {
        // disable target
        plic_target_setup(tgt, PLIC_TGT_PRIO_MAX);
        // clear IE bits
        for (int i = 0; i < PLF_PLIC_BANK_NUM; ++i) {
            PLIC_TGT_IE[tgt * 32 + i] = 0;
        }
    }
    // init lines
    for (int i = 0; i < PLF_PLIC_LINE_NUM; i++) {
        // disable line
        plic_line_config(i, PLIC_SRC_PRIO_OFF, PLIC_SRC_MODE_OFF);
    }
}

//----------------------
// PLIC current context
// M-mode IRQ control
//----------------------

static inline int plic_irq_setup(int line, int mode)
{
    plic_target_line_disable((int)(PLF_PLIC_CONTEXT), line);
    plic_line_config(line, PLIC_SRC_PRIO_DFL, mode);
    plic_target_setup((int)(PLF_PLIC_CONTEXT), PLIC_TGT_PRIO_DFL);

    return line;
}

static inline void plic_irq_enable(int irq)
{
    plic_target_line_enable((int)(PLF_PLIC_CONTEXT), irq);
}

static inline void plic_irq_disable(int irq)
{
    plic_target_line_disable((int)(PLF_PLIC_CONTEXT), irq);
}

int plic_soi(void)
{
    return plic_target_claim((int)(PLF_PLIC_CONTEXT));
}

void plic_eoi(int irq)
{
    plic_target_complete((int)(PLF_PLIC_CONTEXT), irq);
}

#ifdef __cplusplus
}
#endif
#endif // !__ASSEMBLER__
#endif // PLF_PLIC_BASE
#endif // SCR_BSP_PLIC_H
