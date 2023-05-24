#include "platform.h"
#include "bl_stdlib.h"

void bl_empty_handler(int);
void ecall_int_handle(void);


void bl_init()
{
	for (int i = 0; i < 16; ++i)
		int_table[i] = (U32)bl_empty_handler;
	for (int i = 0; i < 32; ++i)
		exc_table[i] = (U32)bl_empty_handler;
	bl_puts("[INIT] Inited trap vector table");
	exc_table[11] = (U32)ecall_int_handle;
}
