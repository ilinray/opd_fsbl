#!/usr/bin/sh

riscv64-unknown-elf-gcc -g -march=rv32im -mabi=ilp32 -fno-common -mstrict-align -mno-relax -std=c99 -Wa,-mno-arch-attr -c -o bl_init.o bl_init.c
riscv64-unknown-elf-gcc -g -march=rv32im -mabi=ilp32 -fno-common -mstrict-align -mno-relax -std=c99 -Wa,-mno-arch-attr -c -o bl_sprintf.o bl_sprintf.c
riscv64-unknown-elf-gcc -g -march=rv32im -mabi=ilp32 -fno-common -mstrict-align -mno-relax -std=c99 -Wa,-mno-arch-attr -c -o main.o main.c
riscv64-unknown-elf-gcc -g -march=rv32im -mabi=ilp32 -fno-common -mstrict-align -mno-relax -std=c99 -Wa,-mno-arch-attr -c -o puts.o puts.c
riscv64-unknown-elf-gcc -g -march=rv32im -mabi=ilp32 -fno-common -mstrict-align -mno-relax -std=c99 -Wa,-mno-arch-attr -c -o uart.o uart.c
riscv64-unknown-elf-gcc -g -march=rv32im -mabi=ilp32 -fno-common -mstrict-align -mno-relax -std=c99 -Wa,-mno-arch-attr -c -o bl_empty_handle.o bl_empty_handle.c
riscv64-unknown-elf-gcc -march=rv32im_zicsr -mabi=ilp32 -msmall-data-limit=8 -mno-relax -mstrict-align -mno-save-restore -Og -fmessage-length=0 -fno-common -Werror -Wall -Wextra  -g3 -x assembler -Wa,-mno-arch-attr -c -o bl_startup.o bl_startup.s
riscv64-unknown-elf-gcc -nostdlib -march=rv32im -mabi=ilp32 -T script.ld -o rvbl.elf bl_startup.o bl_empty_handle.o bl_init.o bl_sprintf.o main.o puts.o uart.o
objcopy --remove-section .comment --remove-section .riscv.attributes rvbl.elf stripped.rvbl.elf
cp stripped.rvbl.elf unstripped.rvbl.elf
strip stripped.rvbl.elf
objcopy -O binary stripped.rvbl.elf rvbl.bin -S

dd if=/dev/zero bs=64K count=1 of=rvbl.img
dd if=rvbl.bin of=rvbl.img
dd if=start_hook.bin of=rvbl.img obs=256 seek=255
