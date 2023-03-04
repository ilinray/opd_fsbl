/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2021. All rights reserved.
///
/// @brief SCR* infra startup code

#include "arch.h"
#include "memasm.h"
#include "drivers/mpu.h"
#include "drivers/cache.h"

/// /////////////////////////
/// startup code

    .globl _bsp_start, main, _hart_halt, _hart_halt1, plf_init, _cstart
    .weak abort, trap_entry, trap_handler, app_init

    .option norvc

    .section ".startup.entry","ax",@progbits

    // Entry point
_bsp_start:
    // reset mstatus
    csrw  mstatus, zero

#ifdef PLF_INIT_REGS
    init_regs_const 1, XREG_MAX_NUM, 0
#endif // PLF_INIT_REGS

    // setup gp
    load_addrword_abs gp, __global_pointer$
    // setup MTVEC
    load_addrword_abs t0, trap_entry
    csrw  mtvec, t0
    // setup MIE, MIP
    csrw  mie, zero
    csrw  mip, zero

#if PLF_CACHE_CFG
    // prepare disabling of caches: flush init sections
    load_addrword_pcrel t0, __TEXT_INIT_START__
    load_const_int32 t1, __TEXT_INIT_SIZE__
    cache_flush t0, t1
#endif // PLF_CACHE_CFG

    // preconfigure MPU (global NC, MMCFG, MMIO)
    mpu_reset_init
    // reset and disable L1$
    cache_reset_nc

#if PLF_HAVE_FPU
    // init FPU (if supported)
    csrr  a0, misa
    andi  a0, a0, (1 << ('F' - 'A')) | (1 << ('D' - 'A'))
    beqz  a0, 1f
    li    a0, (1 << 13) // mstatus.FS = 1 (initial)
    csrs  mstatus, a0
    csrw  fcsr, zero

#ifdef PLF_INIT_REGS
    zero_fregs 0, FREG_MAX_NUM
#endif // PLF_INIT_REGS

1:
#endif // PLF_HAVE_FPU
#if PLF_HAVE_VPU
    // init VPU
    // not neccessary since
    // "Allowing mstatus.VS to exist when misa.V is clear, enables vector emulation
    // and simplifies handling of mstatus.VS in systems with writable misa.V"
    csrr  a0, misa
    li    a1, (1 << ('V' - 'A'))
    and   a0, a0, a1
    beqz  a0, 1f
    // ----------------------------------------------------------------------------
    li    a0, (1 << 9) // mstatus.VS = 1 (initial)
    csrs  mstatus, a0
#endif // PLF_HAVE_VPU

1:
#if PLF_SMP_SUPPORT
#define slave_hart_start secondary_spin_wait
#else
#define slave_hart_start _hart_halt
#endif // PLF_SMP_SUPPORT

    // park/prep SMP slaves
    csrr  a0, mhartid
    // TODO: alt harts enumeration (slot#, lottery, etc)

#if PLF_SMP_MASTER_HARTID > 0
    li    t0, PLF_SMP_MASTER_HARTID
    bne   a0, t0, slave_hart_start
#elif PLF_SMP_MASTER_HARTID == 0
    bnez  a0, slave_hart_start
#endif // PLF_SMP_MASTER_HARTID > 0

    // init HART#0 sp, tp
    load_addrword_abs sp, __TLS0_BASE__
    mv    tp, sp
    csrw  mscratch, sp
#if PLF_TRAP_STACK
    // reserve trap stack space
    li    t0, (PLF_TRAP_STACK + 16)
    sub   sp, sp, t0
#endif // PLF_TRAP_STACK
    // platform init
    load_addrword t0, plf_init
    jalr  t0
    // C runtime init
    load_addrword t0, _cstart
    jalr  t0
    // application-specific initialization hook
    load_addrword t0, app_init
    jalr  t0
#if PLF_SMP_SUPPORT
    // kick slaves
    .globl plf_smp_sync_var
    li    t0, 1
    load_addrword t1, plf_smp_sync_var
    sw    t0, (t1)
#if PLF_SMP_NON_COHERENT
    clflush t1
#endif // PLF_SMP_NON_COHERENT
#endif // PLF_SMP_SUPPORT

    // start main
    load_addrword t0, main
    load_addrword ra, exit
    // main() args
    li    a0, 0
    li    a1, 0
    li    a2, 0
    jr    t0

trap_handler:
_hart_halt:
#ifdef PLF_HALT_HANDLER
    li    a0, -1
_hart_halt1:
    load_addrword t0, PLF_HALT_HANDLER
    jalr  t0
#else  // PLF_HALT_HANDLER
_hart_halt1:
#endif

    // disable interrupts
    csrci mstatus, 0xf
    csrw  mie, zero
1:  wfi
    j     1b

abort:
    j     _hart_halt

#if PLF_SMP_SUPPORT
    .globl hart_start_table, plf_smp_slave_init
secondary_spin_wait:
    // park unsupported harts
#if PLF_SMP_HARTID_BASE != 0
#if PLF_SMP_HARTID_BASE < 0x7ff
    addi  a0, a0, -(PLF_SMP_HARTID_BASE)
#else // PLF_SMP_HARTID_BASE < 0x7ff
    li    a1, PLF_SMP_HARTID_BASE
    sub   a0, a0, a1
#endif // PLF_SMP_HARTID_BASE < 0x7ff
#endif // PLF_SMP_HARTID_BASE != 0
    li    a1, PLF_SMP_HART_NUM
    bgeu  a0, a1, _hart_halt
    load_addrword tp, hart_start_table
    sll   a1, a0, XREG_LEN_LOG
    add   tp, tp, a1
    // wait master init
1:
#if PLF_SMP_NON_COHERENT
    clinval tp
#endif // PLF_SMP_NON_COHERENT
    LOAD_XREG sp, (tp)
    // wait for valid address value: addr != 0 and (addr & 3) == 0
    beqz  sp, 1b
    andi  a1, sp, 3
    bnez  a1, 1b
    // store invalid addr value to prevent infinity loops
    li    a1, -1
    SAVE_XREG a1, (tp)
#if PLF_SMP_NON_COHERENT
    clflush tp
#endif // PLF_SMP_NON_COHERENT
    mv    tp, sp
    csrw  mscratch, sp
#if PLF_TRAP_STACK
    // reserve trap stack space
    li    t0, PLF_TRAP_STACK
    sub   sp, sp, t0
#endif // PLF_TRAP_STACK
    jal   plf_smp_slave_init
    // start main
    li    a0, 0
    li    a1, 0
    load_addrword t0, PLF_SMP_SLAVE_ENTRY
    load_addrword ra, exit
    jr    t0
#endif // PLF_SMP_SUPPORT

    // stubs
app_init:
    ret

#if PLF_SAVE_RESTORE_REGS331_SUB
/// trap helpers: save/restore regs x3-x31
    .section ".text.crt.plf_save_regs331_sub","ax",@progbits
    .align 4
    .type plf_save_regs331_sub, @function
plf_save_regs331_sub:
    save_regs 3, XREG_MAX_NUM, sp // save x3 - x15/x31
    ret
    .size plf_save_regs331_sub, .-plf_save_regs331_sub

    .section ".text.crt.plf_restore_regs331_sub","ax",@progbits
    .align 4
    .type plf_restore_regs331_sub, @function
plf_restore_regs331_sub:
    load_regs 3, XREG_MAX_NUM, sp // restore x3 - x15/x31
    ret
    .size plf_restore_regs331_sub, .-plf_restore_regs331_sub
#endif // PLF_SAVE_RESTORE_REGS331_SUB

/// /////////////////////////
/// trap handler
    .section ".text.crt.trap_entry","ax",@progbits
    .align 6
    .type trap_entry, @function
trap_entry:
    // save context
    context_save
    // save mstatus priv stack
    csrr s0, mstatus
    // load trap handler args
    csrr a0, mcause
    csrr a1, mepc
    mv   a2, sp

    // setup gp
    load_addrword_abs gp, __global_pointer$
    // call trap handler
    load_addrword t0, trap_handler
    jalr t0

    // restore mstatus priv stack
    csrw mstatus, s0
    // restore context
    context_restore
    mret
    .size trap_entry, .-trap_entry

/// Vectored interrupt handlers

    .weak trap_int_msw_entry, trap_int_mtimer_entry, trap_int_mext_entry
    .globl trap_int_msw_handler, trap_int_mtimer_handler, trap_int_mext_handler

    .section ".text.crt.trap_int_msw_entry","ax",@progbits
    .align 2
    .type trap_int_msw_entry, @function
trap_int_msw_entry:
    // save context
    context_save
    // save mstatus priv stack
    csrr s0, mstatus
    // load trap handler args
    csrr a0, mcause
    csrr a1, mepc
    mv   a2, sp
    // setup gp
    load_addrword_abs gp, __global_pointer$
    // call m-mode timer interrupt handler
    load_addrword t0, trap_int_msw_handler
    jalr t0
    // restore mstatus priv stack
    csrw mstatus, s0
    // restore context
    context_restore
    mret
    .size trap_int_msw_entry, .-trap_int_msw_entry

    .section ".text.crt.trap_int_mtimer_entry","ax",@progbits
    .align 2
    .type trap_int_mtimer_entry, @function
trap_int_mtimer_entry:
    // save context
    context_save
    // save mstatus priv stack
    csrr s0, mstatus
    // load trap handler args
    csrr a0, mcause
    csrr a1, mepc
    mv   a2, sp
    // setup gp
    load_addrword_abs gp, __global_pointer$
    // call m-mode timer interrupt handler
    load_addrword t0, trap_int_mtimer_handler
    jalr t0
    // restore mstatus priv stack
    csrw mstatus, s0
    // restore context
    context_restore
    mret
    .size trap_int_mtimer_entry, .-trap_int_mtimer_entry

    .section ".text.crt.trap_int_mext_entry","ax",@progbits
    .align 2
    .type trap_int_mext_entry, @function
trap_int_mext_entry:
    // save context
    context_save
    // save mstatus priv stack
    csrr s0, mstatus
    // load trap handler args
    csrr a0, mcause
    csrr a1, mepc
    mv   a2, sp
    // setup gp
    load_addrword_abs gp, __global_pointer$
    // call m-mode external interrupt handler
    load_addrword t0, trap_int_mext_handler
    jalr t0
    // restore mstatus priv stack
    csrw mstatus, s0
    // restore context
    context_restore
    mret
    .size trap_int_mext_entry, .-trap_int_mext_entry
