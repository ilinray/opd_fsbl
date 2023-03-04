/// @file
/// Syntacore SCR* framework
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.
///
/// @brief baremetal C/C++ startup

#include "arch.h"
#include "memasm.h"

#include <stdio.h>
#include <stdlib.h>

// implement lite libc startup/exit code
// if C++ exceptions aren't used
// and default behavior is not forced by macro

#if __cpp_exceptions
#define IMPL_BSP_DEFAULT_LIBC_INIT_EXIT 1
#elif BSP_DEFAULT_LIBC_INIT_EXIT
#define IMPL_BSP_DEFAULT_LIBC_INIT_EXIT 1
#endif // __cpp_exceptions

typedef void (*ptr_func)();

extern "C" {
extern ptr_func __preinit_array_start[];
extern ptr_func __preinit_array_end[];

extern ptr_func __init_array_start[];
extern ptr_func __init_array_end[];

extern ptr_func __fini_array_start[];
extern ptr_func __fini_array_end[];

void __libc_init_array(void);
void __libc_fini_array(void);

void _cstart(void);
void _exit(int ret_code);
void exit(int ret_code);
}

#ifdef QEMU_AUTOEXIT
static void qemu_exit(int ret_code);
#endif // QEMU_AUTOEXIT

void __init _cstart(void)
{
#if IMPL_BSP_DEFAULT_LIBC_INIT_EXIT
    atexit(__libc_fini_array);
    __libc_init_array();
#else  // IMPL_BSP_DEFAULT_LIBC_INIT_EXIT
    // Call constructors for static objects
    for (ptr_func* fp = __preinit_array_start; fp < __preinit_array_end; ++fp) (*fp)();

    for (ptr_func* fp = __init_array_start; fp < __init_array_end; ++fp) (*fp)();
#endif // IMPL_BSP_DEFAULT_LIBC_INIT_EXIT
}

#if IMPL_BSP_DEFAULT_LIBC_INIT_EXIT
void _exit(int ret_code)
{
#ifdef QEMU_AUTOEXIT
    qemu_exit(ret_code);
#endif // QEMU_AUTOEXIT
    _hart_halt1(ret_code);
}
#else // IMPL_BSP_DEFAULT_LIBC_INIT_EXIT
void exit(int ret_code)
{
    // Call destructors for static objects
    ptr_func* fp = __fini_array_end;

    while (fp > __fini_array_start) { (*(--fp))(); }
#ifdef QEMU_AUTOEXIT
    qemu_exit(ret_code);
#endif // QEMU_AUTOEXIT
    _hart_halt1(ret_code);
}
#endif // IMPL_BSP_DEFAULT_LIBC_INIT_EXIT

#ifdef QEMU_AUTOEXIT
void qemu_exit(int ret_code)
{
    printf("RET_CODE = %d\n", ret_code);
    printf("TEST_ADDR_BASE = %p\n", TEST_ADDR_BASE);
#if __riscv_xlen == 32
    *TEST_ADDR_BASE = (ret_code) ? (0x3333 | ((uint32_t)ret_code << 16)) : 0x5555;
    printf("VALUE_TEST_ADDR_BASE = 0x%x\n", *TEST_ADDR_BASE);
#else
    *TEST_ADDR_BASE = (ret_code) ? (0x3333 | ((uint64_t)ret_code << 16)) : 0x5555;
    printf("VALUE_TEST_ADDR_BASE = 0x%lx\n", *TEST_ADDR_BASE);
#endif // __riscv_xlen == 32
}
#endif // QEMU_AUTOEXIT
