/// @file
/// @brief console i/o routines
/// Syntacore SCR infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.


#include "arch.h"
#include "drivers/console.h"

#ifdef PLF_DEFAULT_CONSOLE
#include PLF_DEFAULT_CONSOLE
#else // stubs
#define plf_con_init() do {} while (0)
#define plf_con_put(data) ((int)(data))
#define plf_con_tx_flush() do {} while (0)
#define plf_con_getch_nowait() (-1)
#endif

void con_init(void)
{
    plf_con_init();
}

int con_putc(int c)
{
    if (c == '\n') {
        plf_con_put('\r');
    }
    plf_con_put(c);
    return c;
}

int con_getc_nowait(void)
{
    return plf_con_getch_nowait();
}

// flush console i/o
void con_flush(void)
{
    plf_con_tx_flush();

    while (con_getc_nowait() >= 0);
}

int con_getc(void)
{
    int c;

    do {
        c = con_getc_nowait();
    } while (c < 0);

    return c;
}

#ifdef putchar
#undef putchar
#endif

// We do not want to alias putchar if glibc printf is used
// because of buffering issues
#if (PRINTF_LEVEL > 0)
int putchar(int ch) __attribute__((weak, alias("con_putc")));
#endif

void con_puts(const char *s)
{
    while (*s)
        con_putc(*s++);
}

void con_puthex64(uint64_t val)
{
    con_puthex32((uint32_t)(val >> 32));
    con_puthex32((uint32_t)val);
}

void con_puthex32(uint32_t val)
{
    con_puthex16(val >> 16);
    con_puthex16(val);
}

void con_puthex16(unsigned val)
{
    con_puthex8(val >> 8);
    con_puthex8(val);
}

void con_puthex8(unsigned val)
{
    con_puthex4(val >> 4);
    con_puthex4(val);
}

void con_puthex4(unsigned val)
{
    int c = val & 0xf;
    con_putc(c + (c > 9 ? ('A' - 10) : '0'));
}

void con_putdec(unsigned long v)
{
    unsigned long res0 = 0;
    unsigned long i0 = 0;

    if (!v) {
        con_putc('0');
        return;
    }

    for (; i0 < sizeof(unsigned long) * 2 && v; ++i0) {
        res0 = (res0 << 4) | v % 10;
        v /= 10;
    }

    if (v) {
        unsigned long res1 = 0;
        unsigned long i1 = 0;
        for (; i1 < sizeof(unsigned long) * 2 && v; ++i1) {
            res1 = (res1 << 4) | v % 10;
            v /= 10;
        }
        while (i1--) {
            con_putc((int)(res1 & 0xf) + '0');
            res1 >>= 4;
        }
    }
    while (i0--) {
        con_putc((int)(res0 & 0xf) + '0');
        res0 >>= 4;
    }
}

unsigned long con_read_hex(void)
{
    unsigned len = 0;
    unsigned long res = 0;
    while (1) {
        int c = con_getc();
        if ((c >= '0' && c <= '9') || ((c & ~0x20) >= 'A' && (c & ~0x20) <= 'F')) {
            if (len >= sizeof(unsigned long) * 2)
                continue;
            con_putc(c);
            if (c > '9')
                c = (c & ~0x20) - 'A' + 10;
            else
                c = c - '0';
            ++len;
            res = (res << 4) + (c & 0xf);
        } else if (c == 0x7f || c == 0x8) {
            if (!len)
                continue;
            --len;
            res >>= 4;
            con_putc(0x8);
            con_putc(' ');
            con_putc(0x8);
        } else if (c == '\r') {
            con_putc('\n');
            break;
        }
    }

    return res;
}

unsigned long con_read_dec(char *echo_str)
{
    unsigned len = 0;
#if __riscv_xlen > 32
    const unsigned max_len = 20;
#else
    const unsigned max_len = 10;
#endif
    unsigned long res = 0;

    if (echo_str) {
        for (const char *p = echo_str;
             len <= max_len && *p && *p >= '0' && *p <= '9';
             ++p, ++len) {
            char c = *p;
            res = res * 10 + (unsigned long)c - '0';
            con_putc(c);
        }
    }

    while (1) {
        int c = con_getc();
        if (c >= '0' && c <= '9') {
            if (len > max_len)
                continue;
            con_putc(c);
            if (echo_str)
                echo_str[len] = (char)c;
            ++len;
            res = res * 10 + (unsigned long)c - '0';
        } else if (c == 0x7f || c == 0x8) {
            if (!len)
                continue;
            --len;
            res /= 10;
            con_putc(0x8);
            con_putc(' ');
            con_putc(0x8);
        } else if (c == '\r') {
            con_putc('\n');
            break;
        }
    }
    if (echo_str)
        echo_str[len] = '\0';

    return res;
}

unsigned con_read_str(char *buf, unsigned size)
{
    unsigned len = 0;

    while (1) {
        int c = con_getc();
        if (c > 0x20 && c < 0x7f) {
            if (len + 1 >= size)
                continue;
            con_putc(c);
            buf[len++] = (char)c;
        } else if (c == 0x7f || c == 0x8) {
            if (!len)
                continue;
            --len;
            con_putc(0x8);
            con_putc(' ');
            con_putc(0x8);
        } else if (c == '\r') {
            if (size)
                buf[len] = 0;
            con_putc('\n');
            break;
        }
    }

    return len;
}
