#include "uart.h"
#include "platform.h"

void uart_init()
{
	uart_io.ier = 0;
	uart_io.mcr = 0;
	uart_io.lcr |= UART_8BIT_LCR;
	uart_div_acc(UART_DIV_ENABLE);
	uart_io.dll = UART_DIV_RATE_L;
	uart_io.dlm = UART_DIV_RATE_H;
	uart_div_acc(UART_DIV_DISABLE);
	uart_io.fcr = BIT(0);
}
