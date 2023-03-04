/// @file
/// @brief Mutual exclusion using Szymanski's algorithm
/// Syntacore SCRx infra
///
/// @copyright (C) Syntacore 2020. All rights reserved.

/***********
 Pseudo-code:
 Entry protocol
 flag[self] <- 1                      # Standing outside waiting room
 await(all flag[1..N] in {0, 1, 2})   # Wait for open door
 flag[self] <- 3                      # Standing in doorway
 if any flag[1..N] = 1:               # Another process is waiting to enter
     flag[self] <- 2                  # Waiting for other processes to enter
     await(any flag[1..N] = 4)        # Wait for a process to enter and close the door

 flag[self] <- 4                      # The door is closed
 await(all flag[1..self-1] in {0, 1}) # Wait for everyone of lower ID to finish exit protocol

 # Critical section
 #...

 # Exit protocol
 await(all flag[self+1..N] in {0, 1, 4}) # Ensure everyone in the waiting room has
                                         # realized that the door is supposed to be closed
 flag[self] <- 0 # Leave. Reopen door if nobody is still in the waiting room
***********/

#include "arch.h"

#if PLF_SMP_SUPPORT && !PLF_ATOMIC_SUPPORTED

#include "lock.h"
#include "drivers/cache.h" /* flush/invalidate */

#if PLF_SMP_HART8_XLEN < PLF_SMP_HART_NUM
#error multiword lock is not implemented yet
#endif

static unsigned long asp_read_flags(arch_lock_t *lock)
{
#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)&(lock->flags), sizeof(lock->flags));
#else
    fence();
#endif // PLF_SMP_NON_COHERENT
    return lock->flags[0];
}

static void asp_write_flag(arch_lock_t *lock, long pos, uint8_t val)
{
    volatile uint8_t *self = (volatile uint8_t*)&(lock->flags[0]) + pos;

#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)self, sizeof(*self));
#else
    fence();
#endif // PLF_SMP_NON_COHERENT
    *self = val;
#if PLF_SMP_NON_COHERENT
    cache_flush((void*)self, sizeof(*self));
#endif // PLF_SMP_NON_COHERENT
}

static uint8_t asp_read_flag(arch_lock_t *lock, long pos)
{
    volatile uint8_t *self = (volatile uint8_t*)&(lock->flags[0]) + pos;

#if PLF_SMP_NON_COHERENT
    cache_invalidate((void*)self, sizeof(*self));
#else
    fence();
#endif // PLF_SMP_NON_COHERENT

    return *self;
}

#if __riscv_xlen == 32
#define MK_ASP_MASK(v) (((unsigned long)(v) << 0) | ((unsigned long)(v) << 8) | ((unsigned long)(v) << 16) | ((unsigned long)(v) << 24))
#else
#define MK_ASP_MASK(v) (((unsigned long)(v) << 0) | ((unsigned long)(v) << 8) | ((unsigned long)(v) << 16) | ((unsigned long)(v) << 24) | ((unsigned long)(v) << 32) | ((unsigned long)(v) << 40) | ((unsigned long)(v) << 48) | ((unsigned long)(v) << 56))
#endif

enum ASP_state {
    ASP_ST0 = 1 << 0,
    ASP_ST1 = 1 << 1,
    ASP_ST2 = 1 << 2,
    ASP_ST3 = 1 << 3,
    ASP_ST4 = 1 << 4,
};

static void asp_wait_all_in(arch_lock_t *lock, unsigned long mask)
{
    while ((asp_read_flags(lock) & ~mask) != 0);
}

static void asp_wait_any_eq(arch_lock_t *lock, unsigned long mask)
{
    while ((asp_read_flags(lock) & mask) == 0);
}

void arch_lock(arch_lock_t *lock)
{
    long self_pos = (long)arch_hartid();
    /* long self_pos = HLS()->ipi_n; */
    long self_bits = self_pos << 3;
    unsigned long self_mask = 0xff << self_bits;
    unsigned long hi_mask = ~((1 << self_bits) - 1);

    if (asp_read_flag(lock, self_pos) == ASP_ST4)
        return; // ignore recursive call

    // Standing outside waiting room
    asp_write_flag(lock, self_pos, ASP_ST0);
    //  Wait for open door
    asp_wait_all_in(lock, MK_ASP_MASK(ASP_ST0 | ASP_ST1 | ASP_ST2));
    // Standing in doorway
    asp_write_flag(lock, self_pos, ASP_ST3);
    if (asp_read_flags(lock) & MK_ASP_MASK(ASP_ST1)) {
        // Another process is waiting to enter
        // Waiting for other processes to enter
        asp_write_flag(lock, self_pos, ASP_ST2);
        // Wait for a process to enter and close the door
        asp_wait_any_eq(lock, ASP_ST4);
    }
    // The door is closed
    asp_write_flag(lock, self_pos, ASP_ST4);
    // Wait for everyone of lower ID to finish exit protocol
    asp_wait_all_in(lock, MK_ASP_MASK(ASP_ST0 | ASP_ST1) | hi_mask | self_mask);
}

void arch_unlock(arch_lock_t *lock)
{
    long self_pos = (long)arch_hartid();
    /* long self_pos = HLS()->ipi_n; */
    long self_bits = self_pos << 3;
    unsigned long low_mask = ((1 << self_bits) - 1);

    // Ensure everyone in the waiting room has
    // realized that the door is supposed to be closed
    asp_wait_all_in(lock, MK_ASP_MASK(ASP_ST0 | ASP_ST1 | ASP_ST4) | low_mask);
    // Leave. Reopen door if nobody is still in the waiting room
    asp_write_flag(lock, self_pos, ASP_ST0);
}

#endif // PLF_SMP_SUPPORT && !PLF_ATOMIC_SUPPORTED
