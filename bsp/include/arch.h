/// @file
/// @brief Architecture specific defs and inlines
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2022. All rights reserved.

#ifndef SCR_BSP_ARCH_H
#define SCR_BSP_ARCH_H

#if __riscv_xlen == 32
#define EXPAND32ADDR(x) (x)
#else
#define EXPAND32ADDR(x) (((((x) / 0x80000000) & 1) * 0xffffffff00000000) + (x))
#endif

#if __riscv_xlen == 32
#define TEST_ADDR      (0xf0080000)
#define TEST_ADDR_BASE ((uint32_t*)(TEST_ADDR))
#else
#define TEST_ADDR      (0xffffffffff008000)
#define TEST_ADDR_BASE ((uint64_t*)(TEST_ADDR))
#endif

#include "plf.h"
#include "stringify.h"

#ifndef PLF_HAVE_FPU
#define PLF_HAVE_FPU (PLF_CORE_VARIANT_SCR > 3)
#endif // PLF_HAVE_FPU

#ifndef PLF_HAVE_MPU
#define PLF_HAVE_MPU (PLF_CORE_VARIANT_SCR > 1)
#endif // PLF_HAVE_MPU

#ifndef PLF_HAVE_MMU
#define PLF_HAVE_MMU (PLF_CORE_VARIANT_SCR >= 5)
#endif // PLF_HAVE_MMU

#ifndef PLF_CPU_CLK
#define PLF_CPU_CLK PLF_SYS_CLK
#endif

// platform info
#ifndef PLF_CPU_NAME
#if __riscv_xlen == 32
#define PLF_CPU_NAME _TOSTR(PLF_CORE_VARIANT)
#else
#define PLF_CPU_ISA  "/RV" _TOSTR(__riscv_xlen)
#define PLF_CPU_NAME _TOSTR(PLF_CORE_VARIANT) PLF_CPU_ISA
#endif
#endif
#ifndef PLF_IMPL_STR
#define PLF_IMPL_STR "custom"
#endif

// cache support
#ifndef PLF_CACHELINE_SIZE
#define PLF_CACHELINE_SIZE 0
#endif

// default stack reserved space
#ifndef PLF_STACK_SIZE
#define PLF_STACK_SIZE 2048
#endif

// separate trap stack size (0 if not used)
#ifndef PLF_TRAP_STACK
#define PLF_TRAP_STACK 0
#endif // PLF_TRAP_STACK

#if PLF_SMP_SUPPORT
// minimal stack for SMP slaves
#ifndef PLF_HLS_MIN_STACK_SIZE
#define PLF_HLS_MIN_STACK_SIZE (2032 + PLF_TRAP_STACK)
#endif

#ifndef PLF_SMP_HART_NUM
#define PLF_SMP_HART_NUM (__riscv_xlen / 8)
#endif

#ifndef PLF_SMP_HARTID_BASE
#define PLF_SMP_HARTID_BASE 0
#endif

#ifndef PLF_SMP_MASTER_HARTID
#define PLF_SMP_MASTER_HARTID PLF_SMP_HARTID_BASE
#endif

#if PLF_SMP_MASTER_HARTID >= (PLF_SMP_HART_NUM + PLF_SMP_HARTID_BASE)
#error PLF_SMP_MASTER_HARTID >= (PLF_SMP_HART_NUM + PLF_SMP_HARTID_BASE)
#endif

#ifndef PLF_SMP_SLAVE_ENTRY
#define PLF_SMP_SLAVE_ENTRY smp_slave_entry
#endif

#endif // PLF_SMP_SUPPORT

#ifndef PLF_ATOMIC_SUPPORTED
#ifdef __riscv_atomic
#define PLF_ATOMIC_SUPPORTED 1
#else // __riscv_atomic
#define PLF_ATOMIC_SUPPORTED 0
#endif // __riscv_atomic
#endif // !PLF_ATOMIC_SUPPORTED

// default console headers
#ifndef PLF_DEFAULT_CONSOLE
#if defined(PLF_UART0_16550)
#define PLF_DEFAULT_CONSOLE "drivers/uart_16550.h"
#elif defined(PLF_SIM_CONSOLE)
#define PLF_DEFAULT_CONSOLE "drivers/uart_sim.h"
#elif defined(PLF_UART0_SIFIVE)
#define PLF_DEFAULT_CONSOLE "drivers/uart_sifive.h"
#elif defined(PLF_HTIF_CONSOLE)
#define PLF_DEFAULT_CONSOLE "drivers/htif.h"
#elif defined(PLF_VIRTIO_CONSOLE)
#define PLF_DEFAULT_CONSOLE "drivers/virtio.h"
#endif // search default console
#endif // PLF_DEFAULT_CONSOLE

// trap handler smart save/restore
#ifndef PLF_SAVE_RESTORE_REGS331_SUB
#define PLF_SAVE_RESTORE_REGS331_SUB 0
#endif // PLF_SAVE_RESTORE_REGS331_SUB

// trap causes
// interrupts
#define TRAP_CAUSE_INTERRUPT_FLAG (1UL << (__riscv_xlen - 1))
#define TRAP_CAUSE_INT_SSOFT      (1)
#define TRAP_CAUSE_INT_MSOFT      (3)
#define TRAP_CAUSE_INT_STIME      (5)
#define TRAP_CAUSE_INT_MTIME      (7)
#define TRAP_CAUSE_INT_SEXT       (9)
#define TRAP_CAUSE_INT_MEXT       (11)
// exceptions
#define TRAP_CAUSE_EXC_FETCH_ALIGN  (0)
#define TRAP_CAUSE_EXC_FETCH_ACCESS (1)
#define TRAP_CAUSE_EXC_ILLEGAL      (2)
#define TRAP_CAUSE_EXC_BREAKPOINT   (3)
#define TRAP_CAUSE_EXC_LOAD_ALIGN   (4)
#define TRAP_CAUSE_EXC_LOAD_ACCESS  (5)
#define TRAP_CAUSE_EXC_STORE_ALIGN  (6)
#define TRAP_CAUSE_EXC_STORE_ACCESS (7)
#define TRAP_CAUSE_EXC_UECALL       (8)
#define TRAP_CAUSE_EXC_SECALL       (9)
#define TRAP_CAUSE_EXC_RESERVED1    (10)
#define TRAP_CAUSE_EXC_MECALL       (11)
#define TRAP_CAUSE_EXC_FETCH_PAGE   (12)
#define TRAP_CAUSE_EXC_LOAD_PAGE    (13)
#define TRAP_CAUSE_EXC_RESERVED2    (14)
#define TRAP_CAUSE_EXC_STORE_PAGE   (15)
#define TRAP_CAUSE_EXC_G_FETCH_PAGE (20)
#define TRAP_CAUSE_EXC_G_LOAD_PAGE  (21)
#define TRAP_CAUSE_EXC_G_VIRT_INST  (22)
#define TRAP_CAUSE_EXC_G_STORE_PAGE (23)

// mie/mip bits
#define MIE_MSOFTWARE (1 << TRAP_CAUSE_INT_MSOFT)
#define MIE_MTIMER    (1 << TRAP_CAUSE_INT_MTIME)
#define MIE_MEXTERNAL (1 << TRAP_CAUSE_INT_MEXT)
// mstatus bits
#define MSTATUS_SIE   (1UL << 1)
#define MSTATUS_MIE   (1UL << 3)
#define MSTATUS_SPIE  (1UL << 5)
#define MSTATUS_UBE   (1UL << 6)
#define MSTATUS_MPIE  (1UL << 7)
#define MSTATUS_SPP   (1UL << 8)
#define MSTATUS_VS    (3UL << 9)
#define MSTATUS_MPP   (3UL << 11)
#define MSTATUS_FS    (3UL << 13)
#define MSTATUS_XS    (3UL << 15)
#define MSTATUS_MPRV  (1UL << 17)
#define MSTATUS_SUM   (1UL << 18)
#define MSTATUS_MXR   (1UL << 19)
#define MSTATUS_TVM   (1UL << 20)
#define MSTATUS_TW    (1UL << 21)
#define MSTATUS_UXL   (3UL << 32)
#define MSTATUS_SXL   (3UL << 34)
#define MSTATUS_SBE   (1UL << 36)
#define MSTATUS_MBE   (1UL << 37)
#define MSTATUS_SD    (1UL << 63)

#define MSTATUS_MPP_SHIFT (11)

#define PRV_U (0)
#define PRV_S (1)
#define PRV_M (3)

// sstatus bits
#define SSTATUS_SIE   (1UL << 1)
#define SSTATUS_SPIE  (1UL << 5)
#define SSTATUS_UBE   (1UL << 6)
#define SSTATUS_SPP   (1UL << 8)
#define SSTATUS_VS    (3UL << 9)
#define SSTATUS_FS    (3UL << 13)
#define SSTATUS_XS    (3UL << 15)
#define SSTATUS_SUM   (1UL << 18)
#define SSTATUS_MXR   (1UL << 19)
#define SSTATUS_UXL   (3UL << 32)
#define SSTATUS_SD    (1UL << 63)

#define SSTATUS_SPP_SHIFT (8)

// hstatus bits
#define HSTATUS_VSBE  (1UL << 5)
#define HSTATUS_GVA   (1UL << 6)
#define HSTATUS_SPV   (1UL << 7)
#define HSTATUS_SPVP  (1UL << 8)
#define HSTATUS_HU    (1UL << 9)
#define HSTATUS_VGEIN (63UL << 12)
#define HSTATUS_VTVM  (1UL << 20)
#define HSTATUS_VTW   (1UL << 21)
#define HSTATUS_VTSR  (1UL << 22)
#define HSTATUS_VSXL  (3UL << 32)

#ifndef __ASSEMBLER__

#include "csr.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline __attribute__((always_inline)) unsigned long arch_misa(void) { return read_csr(misa); } 

static inline __attribute__((always_inline)) unsigned long arch_cpuid(void) { return arch_misa(); }

static inline __attribute__((always_inline)) unsigned long arch_impid(void) { return read_csr(mimpid); }

static inline __attribute__((always_inline)) unsigned long arch_vendorid(void) { return read_csr(mvendorid); }

static inline __attribute__((always_inline)) unsigned long arch_archid(void) { return read_csr(marchid); }

static inline __attribute__((always_inline)) unsigned long arch_hartid(void) { return read_csr(mhartid); }

static inline __attribute__((always_inline)) unsigned long arch_mtval(void) { return read_csr(mtval); }

static inline __attribute__((always_inline)) uint64_t arch_cycle(void)
{
#if __riscv_xlen == 32
    return read_csr64(cycle);
#else  //  __riscv_xlen == 32
    return read_csr(cycle);
#endif // __riscv_xlen == 32
}

static inline __attribute__((always_inline)) uint64_t arch_instret(void)
{
#if __riscv_xlen == 32
    return read_csr64(instret);
#else  //  __riscv_xlen == 32
    return read_csr(instret);
#endif // __riscv_xlen == 32
}

static inline __attribute__((always_inline)) void ifence(void) { asm volatile("fence.i" ::: "memory"); }

static inline __attribute__((always_inline)) void fence(void) { asm volatile("fence" ::: "memory"); }

static inline void wfi(void) { asm volatile("wfi" ::: "memory"); }

void __attribute__((noreturn)) _hart_halt(void);
void __attribute__((noreturn)) _hart_halt1(int ret_code);

static inline void cpu_relax(void)
{
    long dummy;
    asm volatile("div %0, %0, zero" : "=r"(dummy)::"memory");
}

static inline void sfence_vma(void) { asm volatile("sfence.vma" ::: "memory"); }

static inline void sfence_vma_addr(uintptr_t addr) { asm volatile("sfence.vma %0, zero" ::"r"(addr) : "memory"); }

static inline void sfence_vma_asid(unsigned asid) { asm volatile("sfence.vma zero, %0" ::"r"(asid) : "memory"); }

static inline void sfence_vma_addr_asid(uintptr_t addr, unsigned asid)
{
    asm volatile("sfence.vma %0, %1" ::"r"(addr), "r"(asid) : "memory");
}

static inline void flush_tlb(void) { sfence_vma(); }

static inline uint32_t get_build_id(void)
{
#if PLF_BLD_ID_ADDR
    return *(volatile uint32_t*)PLF_BLD_ID_ADDR;
#else
    return 0;
#endif
}

static inline uint32_t get_system_id(void)
{
#ifdef PLF_SYS_ID_ADDR
    return *(volatile uint32_t*)PLF_SYS_ID_ADDR;
#else
    return 0;
#endif
}

static inline int supports_extension(char ext) { return (arch_misa() & (1UL << (ext - 'A'))) != 0; }

// generic platform initialization function
void plf_init_generic(void);

#ifdef __cplusplus
}
#endif
#endif // __ASSEMBLER__
#endif // SCR_BSP_ARCH_H
