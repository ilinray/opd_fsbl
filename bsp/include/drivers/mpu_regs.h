/// @file
/// @brief Syntacore MPU registers
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2018. All rights reserved.

#ifndef SCR_BSP_MPU_REGS_H
#define SCR_BSP_MPU_REGS_H

#include "arch.h"

// platform-specific MPU CSRs
#ifndef SCR_CSR_MPU_BASE
#define SCR_CSR_MPU_BASE 0xbc4
#endif

// MPU CSRs
#define SCR_CSR_MPU_SEL  (SCR_CSR_MPU_BASE + 0)
#define SCR_CSR_MPU_CTRL (SCR_CSR_MPU_BASE + 1)
#define SCR_CSR_MPU_ADDR (SCR_CSR_MPU_BASE + 2)
#define SCR_CSR_MPU_MASK (SCR_CSR_MPU_BASE + 3)

// MPU ctrl bits
#define SCR_MPU_ACCESS_R (1 << 0)
#define SCR_MPU_ACCESS_W (1 << 1)
#define SCR_MPU_ACCESS_X (1 << 2)
// MPU ctrl mode positions
#define SCR_MPU_MODE_M 0
#define SCR_MPU_MODE_U 1
#define SCR_MPU_MODE_S 2

#define SCR_MPU_MODE_ACCESS(mode, access_bits) ((access_bits) << ((mode) * 3 + 1))

#define _SCR_MPU_CTRL_MR SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_M, SCR_MPU_ACCESS_R)
#define _SCR_MPU_CTRL_MW SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_M, SCR_MPU_ACCESS_W)
#define _SCR_MPU_CTRL_MX SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_M, SCR_MPU_ACCESS_X)
#define _SCR_MPU_CTRL_UR SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_U, SCR_MPU_ACCESS_R)
#define _SCR_MPU_CTRL_UW SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_U, SCR_MPU_ACCESS_W)
#define _SCR_MPU_CTRL_UX SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_U, SCR_MPU_ACCESS_X)
#define _SCR_MPU_CTRL_SR SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_S, SCR_MPU_ACCESS_R)
#define _SCR_MPU_CTRL_SW SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_S, SCR_MPU_ACCESS_W)
#define _SCR_MPU_CTRL_SX SCR_MPU_MODE_ACCESS(SCR_MPU_MODE_S, SCR_MPU_ACCESS_X)
// memory access modes
#define _SCR_MPU_CTRL_MT_WEAKLY   (0 << 16)
#define _SCR_MPU_CTRL_MT_STRONG   (1 << 16)
#define _SCR_MPU_CTRL_MT_COHERENT (2 << 16)
#define _SCR_MPU_CTRL_MT_CFG      (3 << 16)
// MPU region lock bit
#define _SCR_MPU_CTRL_LOCK        (1 << 31)
// MPU region valid bit
#define _SCR_MPU_CTRL_VALID       (1 << 0)
// MPU complex access
#define _SCR_MPU_CTRL_MA          (_SCR_MPU_CTRL_MR | _SCR_MPU_CTRL_MW | _SCR_MPU_CTRL_MX)
#define _SCR_MPU_CTRL_SA          (_SCR_MPU_CTRL_SR | _SCR_MPU_CTRL_SW | _SCR_MPU_CTRL_SX)
#define _SCR_MPU_CTRL_UA          (_SCR_MPU_CTRL_UR | _SCR_MPU_CTRL_UW | _SCR_MPU_CTRL_UX)
#define _SCR_MPU_CTRL_ALL         (_SCR_MPU_CTRL_MA | _SCR_MPU_CTRL_SA | _SCR_MPU_CTRL_UA)
#define _SCR_MPU_CTRL_AR          (_SCR_MPU_CTRL_MR | _SCR_MPU_CTRL_SR | _SCR_MPU_CTRL_UR)
#define _SCR_MPU_CTRL_AW          (_SCR_MPU_CTRL_MW | _SCR_MPU_CTRL_SW | _SCR_MPU_CTRL_UW)
#define _SCR_MPU_CTRL_AX          (_SCR_MPU_CTRL_MX | _SCR_MPU_CTRL_SX | _SCR_MPU_CTRL_UX)

// MPU address/mask conversion macros (with sign extension)
#define SCR_MPU_MK_ADDR32(addr) \
    ((((addr) / 4) | ((addr) & 0x80000000) | ((addr) & 0x80000000) / 2) & 0xffffffff)
#define SCR_MPU_MK_MASK32(size) ((~((size) - 1) / 4) & 0xffffffff)

#define SCR_MPU_MK_ADDR64(addr) ((addr) / 4)
#define SCR_MPU_MK_MASK64(size) (~((size) - 1) / 4)

#if __riscv_xlen == 32
#define SCR_MPU_MK_ADDR(addr) SCR_MPU_MK_ADDR32(addr)
#define SCR_MPU_MK_MASK(size) SCR_MPU_MK_MASK32(size)
#elif __riscv_xlen == 64
#define SCR_MPU_MK_ADDR(addr) SCR_MPU_MK_ADDR64(addr)
#define SCR_MPU_MK_MASK(size) SCR_MPU_MK_MASK64(size)
#else
#error MPU addr/mask conversion is not implemented
#endif

#ifndef __ASSEMBLER__

#define SCR_MPU_CTRL_VALID       _SCR_MPU_CTRL_VALID
#define SCR_MPU_CTRL_MR          _SCR_MPU_CTRL_MR
#define SCR_MPU_CTRL_MW          _SCR_MPU_CTRL_MW
#define SCR_MPU_CTRL_MX          _SCR_MPU_CTRL_MX
#define SCR_MPU_CTRL_UR          _SCR_MPU_CTRL_UR
#define SCR_MPU_CTRL_UW          _SCR_MPU_CTRL_UW
#define SCR_MPU_CTRL_UX          _SCR_MPU_CTRL_UX
#define SCR_MPU_CTRL_SR          _SCR_MPU_CTRL_SR
#define SCR_MPU_CTRL_SW          _SCR_MPU_CTRL_SW
#define SCR_MPU_CTRL_SX          _SCR_MPU_CTRL_SX
#define SCR_MPU_CTRL_MT_WEAKLY   _SCR_MPU_CTRL_MT_WEAKLY
#define SCR_MPU_CTRL_MT_STRONG   _SCR_MPU_CTRL_MT_STRONG
/* #define SCR_MPU_CTRL_MT_COHERENT _SCR_MPU_CTRL_MT_COHERENT */
#define SCR_MPU_CTRL_MT_CFG      _SCR_MPU_CTRL_MT_CFG
#define SCR_MPU_CTRL_LOCK        _SCR_MPU_CTRL_LOCK

#define SCR_MPU_CTRL_MA  _SCR_MPU_CTRL_MA
#define SCR_MPU_CTRL_SA  _SCR_MPU_CTRL_SA
#define SCR_MPU_CTRL_UA  _SCR_MPU_CTRL_UA
#define SCR_MPU_CTRL_ALL _SCR_MPU_CTRL_ALL
#define SCR_MPU_CTRL_AR  _SCR_MPU_CTRL_AR
#define SCR_MPU_CTRL_AW  _SCR_MPU_CTRL_AW
#define SCR_MPU_CTRL_AX  _SCR_MPU_CTRL_AX

#else //!__ASSEMBLER__

// constants
.set SCR_MPU_CTRL_VALID,       _SCR_MPU_CTRL_VALID
.set SCR_MPU_CTRL_MR,          _SCR_MPU_CTRL_MR
.set SCR_MPU_CTRL_MW,          _SCR_MPU_CTRL_MW
.set SCR_MPU_CTRL_MX,          _SCR_MPU_CTRL_MX
.set SCR_MPU_CTRL_UR,          _SCR_MPU_CTRL_UR
.set SCR_MPU_CTRL_UW,          _SCR_MPU_CTRL_UW
.set SCR_MPU_CTRL_UX,          _SCR_MPU_CTRL_UX
.set SCR_MPU_CTRL_SR,          _SCR_MPU_CTRL_SR
.set SCR_MPU_CTRL_SW,          _SCR_MPU_CTRL_SW
.set SCR_MPU_CTRL_SX,          _SCR_MPU_CTRL_SX
.set SCR_MPU_CTRL_MT_WEAKLY,   _SCR_MPU_CTRL_MT_WEAKLY
.set SCR_MPU_CTRL_MT_STRONG,   _SCR_MPU_CTRL_MT_STRONG
/* .set SCR_MPU_CTRL_MT_COHERENT, _SCR_MPU_CTRL_MT_COHERENT */
.set SCR_MPU_CTRL_MT_CFG,      _SCR_MPU_CTRL_MT_CFG
.set SCR_MPU_CTRL_LOCK,        _SCR_MPU_CTRL_LOCK

.set SCR_MPU_CTRL_MA,  _SCR_MPU_CTRL_MA
.set SCR_MPU_CTRL_SA,  _SCR_MPU_CTRL_SA
.set SCR_MPU_CTRL_UA,  _SCR_MPU_CTRL_UA
.set SCR_MPU_CTRL_ALL, _SCR_MPU_CTRL_ALL
.set SCR_MPU_CTRL_AR,  _SCR_MPU_CTRL_AR
.set SCR_MPU_CTRL_AW,  _SCR_MPU_CTRL_AW
.set SCR_MPU_CTRL_AX,  _SCR_MPU_CTRL_AX

#endif // __ASSEMBLER__
#endif // SCR_BSP_MPU_REGS_H
