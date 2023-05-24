#include "uart.h"
#include "bl_stdlib.h"
void bl_crlf()
{
	uart_putc(0x0D);
	uart_putc(0x0A);
}
void bl_puts(char* str)
{
	uart_puts(str);
	bl_crlf();
}

char bl_getc()
{
	return uart_getc();
}
void bl_putc(char c)
{
	uart_putc(c);
}
