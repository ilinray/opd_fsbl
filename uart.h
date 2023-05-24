#ifndef UART_H_
#define UART_H_
#include "platform.h"

#define UART_BASE 0xff010000
#define UART_RX_EMPTY (U8)-1
#define UART_DIV_ENABLE 1
#define UART_DIV_DISABLE 0
#define UART_BAUD_RATE 115200
#define QEMU_CLK_HZ 30000000
#define UART_DIV_RATE (QEMU_CLK_HZ / UART_BAUD_RATE / 16)
#define U8MASK 0xff
#define UART_DIV_RATE_H ((UART_DIV_RATE >> 8) & U8MASK)
#define UART_DIV_RATE_L (UART_DIV_RATE & U8MASK)
#define UART_8BIT_LCR 0x3


typedef struct
{
	union {
		U32 thr;
		U32 dll;
	};
	union {
		U32 ier;
		U32 dlm;
	};
	union {
		U32 iir;
		U32 fcr;
	};
	U32 lcr;
	U32 mcr;
	U32 lsr;
	U32 msr;
	U32 scr;
} volatile uart_regs;

#define uart_tx_ready (uart_io.lsr & BIT(6))
#define uart_rx_ready (uart_io.lsr & BIT(0))

#define uart_io (*(uart_regs*)(UART_BASE))

void uart_init();

static inline void uart_div_acc(int acc)
{
	if (acc)
		uart_io.lcr |= BIT(7);
	else
		uart_io.lcr &= ~BIT(7);

}
static inline void uart_putc(U8 n) {
	while (!uart_tx_ready) {}
	uart_io.thr = n;
}
static inline void uart_puts(char* str)
{
	while (*str) uart_putc(*str++);
}

static inline U8 uart_getc()
{
	if (uart_rx_ready)
		return uart_io.thr;
	return UART_RX_EMPTY;
}


#endif /* UART_H_ */
