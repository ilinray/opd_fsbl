/// @file
/// @brief baremetal syscalls
///
/// Syntacore SCR* framework
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#include "drivers/console.h"
#include "drivers/rtc.h"

#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int close(int file) __attribute__((used));
int _close(int file) __attribute__((alias("close"), used));

int fstat(int file, struct stat* st) __attribute__((used));
int _fstat(int file, struct stat* st) __attribute__((alias("fstat"), used));

int isatty(int file) __attribute__((used));
int _isatty(int file) __attribute__((alias("isatty"), used));

int lseek(int file, int ptr, int dir) __attribute__((used));
int _lseek(int file, int ptr, int dir) __attribute__((alias("lseek"), used));

int open(const char* name, int flags, int mode) __attribute__((used));
int _open(const char* name, int flags, int mode) __attribute__((alias("open"), used));

#ifndef __llvm__
__attribute__((used,optimize("no-unroll-loops")))
#endif
int read(int file, char *ptr, int len);

int _read(int file, char* ptr, int len) __attribute__((alias("read"), used));

#ifndef __llvm__
__attribute__((used,optimize("no-unroll-loops")))
#endif
int write(int file, char *ptr, int len);

int _write(int file, char* ptr, int len) __attribute__((alias("write"), used));

void* sbrk(ptrdiff_t incr) __attribute__((used));
void* _sbrk(ptrdiff_t incr) __attribute__((alias("sbrk"), used));

clock_t times(struct tms* buf) __attribute__((used));
clock_t _times(struct tms* buf) __attribute__((alias("times"), used));

int gettimeofday(struct timeval* __restrict _p, void* __restrict _tz) __attribute__((used));
int _gettimeofday(struct timeval* __restrict _p, void* __restrict _tz) __attribute__((alias("gettimeofday"), used));

// ---------------------------
// implementation
// ---------------------------

int close(int file) { return -1; }

int fstat(int file, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int isatty(int file)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO)
        return 1;

    return 0;
}

int lseek(int file, int ptr, int dir) { return 0; }

int open(const char* name, int flags, int mode) { return -1; }

int read(int file, char* ptr, int len)
{
    int res = 0;
    int c;

#ifdef __llvm__
#pragma clang loop unroll(disable)
#endif
    while ((res < len) && ((c = con_getc_nowait()) >= 0)) ptr[res++] = (char)c;

    return res;
}

int write(int file, char* ptr, int len)
{
#ifdef __llvm__
#pragma clang loop unroll(disable)
#endif
    for (int i = 0; i < len; ++i) con_putc(*ptr++);

    return len;
}

struct sbrk_init_heap
{
    uintptr_t cur_end;
    uintptr_t sys_end;
};

void* sbrk(ptrdiff_t incr)
{
    /* Defined by the linker */
    extern char _heap_start[];
    extern char _heap_end[];
    extern char _heap_end_div[];
    extern char _mem_origin[];
    extern char _heap_end_in_stack[];

    void* memblk;

    static uintptr_t cur_heap_end = 0;
    static uintptr_t sys_heap_end = 0;

#if __riscv_xlen == 32
    if (!cur_heap_end)
    {
        uintptr_t effective_heap_end =
            (((uintptr_t)_heap_end - (uintptr_t)_mem_origin) * (uintptr_t)_heap_end_div + (uintptr_t)_mem_origin);
        cur_heap_end = (uintptr_t)_heap_start;
#if PLF_TRAP_STACK
        unsigned long stack_offset = (PLF_STACK_SIZE + PLF_TRAP_STACK + 16) * (uintptr_t)_heap_end_in_stack;
#else
        unsigned long stack_offset = PLF_STACK_SIZE * (uintptr_t)_heap_end_in_stack;
#endif
        sys_heap_end = effective_heap_end - stack_offset;
    }
#else // __riscv_xlen == 32

    if (!cur_heap_end)
    {
        struct sbrk_init_heap init = {.cur_end = (uintptr_t)_heap_start,
                                      .sys_end =
                                          ((uintptr_t)_heap_end - (uintptr_t)_mem_origin) * (uintptr_t)_heap_end_div +
                                          (uintptr_t)_mem_origin};
        cur_heap_end               = init.cur_end;
#if PLF_TRAP_STACK
        unsigned long stack_offset = (PLF_STACK_SIZE + PLF_TRAP_STACK + 16) * (uintptr_t)_heap_end_in_stack;
#else
        unsigned long stack_offset = PLF_STACK_SIZE * (uintptr_t)_heap_end_in_stack;
#endif
        sys_heap_end               = init.sys_end - stack_offset;
    }
#endif // __riscv_xlen == 32

    if (sys_heap_end - cur_heap_end < (size_t)incr)
    {
        /* Heap overflow */
        return (void*)(-ENOMEM);
    }

    memblk = (void*)cur_heap_end;
    cur_heap_end += (size_t)incr;

    return memblk;
}

clock_t times(struct tms* buf)
{
    /*
      tms_utime  : user clock ticks
      tms_stime  : system clock ticks
      tms_cutime : children's user clock ticks
      tms_cstime : children's system clock ticks
    */
    buf->tms_stime = buf->tms_cstime = buf->tms_cutime = 0;

    return buf->tms_utime = (clock_t)rtc_now();
}

#pragma GCC diagnostic ignored "-Wsign-conversion"

int gettimeofday(struct timeval* __restrict _p, void* __restrict _tz)
{
    if (_p)
    {
        sys_tick_t t      = rtc_now();
        unsigned long sec = rtc_ticks2sec(t);

        _p->tv_sec  = sec;
        _p->tv_usec = rtc_ticks2us(t - rtc_sec2ticks(sec));

        return 0;
    }

    return -1;
}
