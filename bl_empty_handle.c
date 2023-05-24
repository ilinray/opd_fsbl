#include "bl_stdlib.h"

void bl_empty_handler(int mcause)
{
	char buffer[50];
	bl_sprintf(buffer, "empty handler; mcause = 0x%x", mcause);
	bl_puts(buffer);
}
