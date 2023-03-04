// -*- mode:asm -*-
/// @file
/// @brief assembler memory operations macros
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2021. All rights reserved.

#ifndef SCR_BSP_MEMASM_H
#define SCR_BSP_MEMASM_H

#if __riscv_xlen == 32
#define XREG_LEN 4
#define XREG_LEN_LOG 2
#define LOAD_XREG lw
#define SAVE_XREG sw
#elif __riscv_xlen == 64 // __riscv_xlen
#define XREG_LEN 8
#define XREG_LEN_LOG 3
#define LOAD_XREG ld
#define SAVE_XREG sd
#elif __riscv_xlen == 128 // __riscv_xlen
#define XREG_LEN 16
#define XREG_LEN_LOG 4
#define LOAD_XREG lq
#define SAVE_XREG sq
#else // __riscv_xlen
#error unsupported or unknown __riscv_xlen
#endif // __riscv_xlen

#ifdef __riscv_32e
#define XREG_MAX_NUM 15
#else // __riscv_32e
#define XREG_MAX_NUM 31
#endif // __riscv_32e

#if __riscv_flen
#define FREG_MAX_NUM 31
#else
#define FREG_MAX_NUM -1
#endif

#define TRAP_REGS_SPACE ((XREG_MAX_NUM + 1) * XREG_LEN)

#ifdef __ASSEMBLER__

.altmacro

.macro memop_reg_offs memop, reg, offs, mem_base=zero
    \memop \reg, \offs(\mem_base)
.endm

.macro save_reg_offs reg, offs, mem_base=zero
    memop_reg_offs SAVE_XREG, \reg, \offs*XREG_LEN, \mem_base
.endm

.macro load_reg_offs reg, offs, mem_base=zero
    memop_reg_offs LOAD_XREG, \reg, \offs*XREG_LEN, \mem_base
.endm

.macro save_reg_idx regn, save_mem_base=zero
    save_reg_offs x\regn, \regn, \save_mem_base
.endm

.macro load_reg_idx regn, load_mem_base=zero
    load_reg_offs x\regn, \regn, \load_mem_base
.endm

.macro save_regs reg_first, reg_last, save_mem_base=zero
    LOCAL regn
    regn = \reg_first
    .rept \reg_last - \reg_first + 1
    save_reg_idx %(regn), \save_mem_base
    regn = regn+1
    .endr
.endm

.macro load_regs reg_first, reg_last, load_mem_base=zero
    LOCAL regn
    regn = \reg_first
    .rept \reg_last - \reg_first + 1
    load_reg_idx %(regn), \load_mem_base
    regn = regn+1
    .endr
.endm

.macro init_xreg regn, const_val
    li   x\regn, \const_val
.endm

.macro init_regs_const reg_first, reg_last, const_val=0
    LOCAL regn
    regn = \reg_first
    .rept \reg_last - \reg_first + 1
        init_xreg %(regn), \const_val
        regn = regn+1
    .endr
.endm

.macro zero_freg regn
    fmv.s.x f\regn, x0
.endm

.macro zero_fregs reg_first, reg_last
    LOCAL regn
    .set regn, \reg_first
    .rept \reg_last - \reg_first + 1
        zero_freg %(regn)
        .set regn, regn+1
    .endr
.endm

.macro memcpy src_beg, src_end, dst, tmp_reg
    LOCAL memcpy_1, memcpy_2
    j     memcpy_2
    memcpy_1:
    LOAD_XREG \tmp_reg, (\src_beg)
    SAVE_XREG \tmp_reg, (\dst)
    add   \src_beg, \src_beg, XREG_LEN
    add   \dst, \dst, XREG_LEN
memcpy_2:
    bltu  \src_beg, \src_end, memcpy_1
.endm

.macro memset dst_beg, dst_end, val_reg
    LOCAL memset_1, memset_2
    j     memset_2
memset_1:
    SAVE_XREG \val_reg, (\dst_beg)
    add   \dst_beg, \dst_beg, XREG_LEN
memset_2:
    bltu  \dst_beg, \dst_end, memset_1
.endm

/// ##############################
/// context save/restore routines

.macro context_save
#if PLF_TRAP_STACK
    LOCAL _init_trap_stack, _use_trap_stack
    csrrw tp, mscratch, tp
    save_reg_offs t0, -1, tp // save original t0 (x5)
    // check trap stack is used
    bge   sp, tp, _init_trap_stack
    li    t0, PLF_TRAP_STACK
    sub   t0, tp, t0
    blt   sp, t0, _init_trap_stack
/*
    // check trap stack overflow
    addi  t0, t0, TRAP_REGS_SPACE
    blt   sp, tp, 3f         // trap stack overflow
*/
    // make new trap stack frame
    mv    t0, sp             // t0 = original sp
    addi  sp, sp, -TRAP_REGS_SPACE
    j     _use_trap_stack
/*
3:  j     plf_trap_stack_overflow
*/
_init_trap_stack:
    // sp not in trap stack area, init trap stack
    mv    t0, sp             // t0 = original sp
    addi  sp, tp, -(TRAP_REGS_SPACE + 0x10)
_use_trap_stack:
    andi  sp, sp, -0x10      // align callee SP by 16
    save_reg_idx 1, sp       // save ra (x1)
    save_reg_offs t0, 2, sp  // save original sp (x2)
    load_reg_offs t0, -1, tp // restore original t0 (x5)
    csrrw tp, mscratch, tp   // restore original tp (x4)
#else // PLF_TRAP_STACK
    // save context without trap stack
    save_reg_offs t0, -1, sp // save original t0 (x5)
    mv    t0, sp             // t0 = original sp
    addi  sp, sp, -TRAP_REGS_SPACE
    andi  sp, sp, -0x10      // align callee SP by 16
    save_reg_idx 1, sp       // save ra (x1)
    save_reg_offs t0, 2, sp  // save original sp (x2)
    load_reg_offs t0, -1, t0 // restore original t0 (x5)
#endif // PLF_TRAP_STACK
#if PLF_SAVE_RESTORE_REGS331_SUB
    jal   plf_save_regs331_sub
#else // PLF_SAVE_RESTORE_REGS331_SUB
    save_regs 3, XREG_MAX_NUM, sp // save x3 - x15/x31
#endif // PLF_SAVE_RESTORE_REGS331_SUB
    csrr tp, mscratch        // load tp
    csrr t1, mepc
    save_reg_offs t1, 0, sp  // save original pc
.endm // context_save

.macro context_restore
    load_reg_offs t1, 0, sp  // restore original pc
    csrw mepc, t1
#if PLF_SAVE_RESTORE_REGS331_SUB
    jal   plf_restore_regs331_sub
#else // PLF_SAVE_RESTORE_REGS331_SUB
    load_regs 3, XREG_MAX_NUM, sp // restore x3 - x15/x31
#endif // PLF_SAVE_RESTORE_REGS331_SUB
    load_reg_idx 1, sp       // restore ra (x1)
    load_reg_idx 2, sp       // restore original sp
.endm // context_restore

/// ##############################
/// addr load/read/write routines

.macro read_pcrel_addrword reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    LOAD_XREG \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // read_pcrel_addrword

.macro write_pcrel_addrword reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    SAVE_XREG \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // write_pcrel_addrword

// load 32 bit (sign extended) symbol
.macro read_pcrel_int32 reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    lw    \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // read_pcrel_int32

// save 32 bit (sign extended) symbol
.macro write_pcrel_int32 reg, sym
    LOCAL pcrel_addr
    .option push
    .option norelax
pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    sw    \reg, %pcrel_lo(pcrel_addr)(\reg)
    .option pop
.endm // write_pcrel_int32

// load 32 bit (sign extended) constant
.macro load_const_int32 reg, sym
    .option push
    .option norelax
    lui \reg, %hi(\sym)
    addi \reg, \reg, %lo(\sym)
    .option pop
.endm // load_const_int32

.macro load_addrword_abs reg, sym
#if __riscv_xlen == 32
    load_const_int32 \reg, \sym
#elif __riscv_xlen == 64
    LOCAL _addrword
    .subsection 8 // subsection for local constants
    .align 3
_addrword:
    .dword \sym
    .previous
     read_pcrel_addrword \reg, _addrword
#else // __riscv_xlen
#error load_addrword_abs is not implemented for xlen
#endif // __riscv_xlen
.endm // load_addrword_abs

.macro load_addrword_pcrel reg, sym
    LOCAL _pcrel_addr
    .option push
    .option norelax
_pcrel_addr:
    auipc \reg, %pcrel_hi(\sym)
    addi \reg, \reg, %pcrel_lo(_pcrel_addr)
    .option pop
.endm // load_addrword_pcrel

.macro load_addrword reg, sym
#if __riscv_xlen == 32
    load_addrword_abs \reg, \sym
#elif __riscv_xlen == 64
#if PLF_SPARSE_MEM
    load_addrword_abs \reg, \sym
#else // PLF_SPARSE_MEM
    load_addrword_pcrel \reg, \sym
#endif // PLF_SPARSE_MEM
#else // __riscv_xlen
#error load_addrword is not implemented for xlen
#endif // __riscv_xlen
.endm // load_addrword

    // sections
#define __INIT       ".init.text","ax",@progbits
#define __INITDATA   ".init.data","aw",@progbits
#define __INITRODATA ".init.rodata","a",@progbits

#else // __ASSEMBLER__

#define __init      __attribute__((__section__(".init.text")))
#define __initdata  __attribute__((__section__(".init.data")))
#define __initconst __attribute__((__section__(".init.rodata")))

#endif // __ASSEMBLER__
#endif // SCR_BSP_MEMASM_H
