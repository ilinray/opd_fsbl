#include "uart.h"
#include "bl_stdlib.h"

void gen_interrupt();

int main()
{
	uart_init();
	char buffer[256];
	bl_sprintf(buffer, "%s with %s and digits(%d) and chars(%c) and hexadecimal(%x)", "Working", "strings", -55, 'b', 412);
	bl_puts(buffer);

	char res;
	while (1)
	{
		res = uart_getc();
		buffer[0] = res;

		if (res != CIN_EMPTY)
			bl_putc(res);
		if (res == '\r')
			bl_crlf();
		if (res == 'a')
			gen_interrupt();

	}
	return 0;
}

