/// @file
/// @brief console i/o routines
/// Syntacore SCR infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#ifndef SCR_BSP_CONSOLE_H
#define SCR_BSP_CONSOLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// initialization
void con_init(void);
void con_flush(void);

// output primitives
int con_putc(int c);
void con_puthex4(unsigned val);
void con_puthex8(unsigned val);
void con_puthex16(unsigned val);
void con_puthex32(uint32_t val);
void con_puthex64(uint64_t val);
void con_putdec(unsigned long v);
void con_puts(const char *s);

// input primitives
int con_getc(void);
int con_getc_nowait(void);

unsigned long con_read_hex(void);
unsigned long con_read_dec(char *echo_str);
unsigned con_read_str(char *buf, unsigned size);

// inlines
static inline void con_puthex(unsigned long val)
{
#if __riscv_xlen > 64
    con_puthex64(val >> 64);
#endif
#if __riscv_xlen > 32
    con_puthex64(val);
#else
    con_puthex32(val);
#endif
}

#ifdef __cplusplus
}
#endif

#endif // SCR_BSP_CONSOLE_H
