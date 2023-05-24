
# define REGSIZE 4

    .globl main
    .globl trap_entry
    .globl bl_exit
    .globl gen_interrupt
    .globl bl_start
    .globl bl_init
    .globl int_table
    .globl exc_table
    .globl ecall_int_handle
    .globl bl_empty_handler
	.option norvc
	.option nopic

	.macro save_reg reg, at
		sw \reg, \at*4(sp)
	.endm
	.macro load_reg reg, at
		lw \reg, \at*4(sp)
	.endm

    .text
    .align 6

bl_start:
    csrw  mstatus, zero
    csrw mie, zero
    csrw mip, zero

    la t1, bl_trap_entry
    csrw mtvec, t1

	la gp, __global_pointer$

    la      a1, __bss_start
    la      a2, __bss_end
    j       4f
3:  sw      zero, 0(a1)
    add     a1, a1, 4
4:  bne     a1, a2, 3b
    la      sp, __c_stack_top

	jal		bl_init
 	j       main
bl_trap_entry:
    addi sp, sp, -124

save_reg x1, 1
save_reg x2, 2
save_reg x3, 3
save_reg x4, 4
save_reg x5, 5
save_reg x6, 6
save_reg x7, 7
save_reg x8, 8
save_reg x9, 9
save_reg x10, 10
save_reg x11, 11
save_reg x12, 12
save_reg x13, 13
save_reg x14, 14
save_reg x15, 15
save_reg x16, 16
save_reg x17, 17
save_reg x18, 18
save_reg x19, 19
save_reg x20, 20
save_reg x21, 21
save_reg x22, 22
save_reg x23, 23
save_reg x24, 24
save_reg x25, 25
save_reg x26, 26
save_reg x27, 27
save_reg x28, 28
save_reg x29, 29
save_reg x30, 30
save_reg x31, 31

    csrr a0, mcause
    mv t1, sp

    la sp, __handler_stack_top
    addi sp, sp, -4
    save_reg t1, 1

    jal bl_handle_trap

    load_reg t1, 1

	mv sp, t1

load_reg x1, 1
load_reg x2, 2
load_reg x3, 3
load_reg x4, 4
load_reg x5, 5
load_reg x6, 6
load_reg x7, 7
load_reg x8, 8
load_reg x9, 9
load_reg x10, 10
load_reg x11, 11
load_reg x12, 12
load_reg x13, 13
load_reg x14, 14
load_reg x15, 15
load_reg x16, 16
load_reg x17, 17
load_reg x18, 18
load_reg x19, 19
load_reg x20, 20
load_reg x21, 21
load_reg x22, 22
load_reg x23, 23
load_reg x24, 24
load_reg x25, 25
load_reg x26, 26
load_reg x27, 27
load_reg x28, 28
load_reg x29, 29
load_reg x30, 30
load_reg x31, 31

    addi sp, sp, 124
    mret
bl_handle_trap:
	addi sp, sp, -4
	save_reg ra, 1

	bltz a0, ht_int
	la t1, exc_table
	j ht_sum
ht_int:
	la t1, int_table
ht_sum:
	mv t2, a0
	slli t2, t2, 2
	add t1, t1, t2
	lw t1, 0(t1)

	jalr t1
	load_reg ra, 1
	addi sp, sp, 4
	ret

gen_interrupt:
	ecall
	ret

ecall_int_handle:
	addi sp, sp, -4
	save_reg ra, 1
	jal bl_empty_handler
	csrr t0, mepc
	addi t0, t0, 4
	csrw mepc, t0
	load_reg ra, 1
	addi sp, sp, 4
	ret

bl_exit:
1:  wfi
    j 1b
