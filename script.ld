MEMORY
{
	RAM : ORIGIN = 0, LENGTH = 256k
	ROM : ORIGIN = 0xffff0000, LENGTH = 64K
}

ENTRY(bl_start)
low_addr_buffer = 4k;
__handler_stack_top = 256k - 4;
__handler_stack_size = 32k;
__c_stack_top = __handler_stack_top - __handler_stack_size;
__c_stack_size = 64k;
int_table = 0 + low_addr_buffer;
exc_table = int_table + low_addr_buffer;


SECTIONS
  {
    .text 0xffff0000 :
      {
        *(.text)
      } > ROM AT>ROM
    .rodata ALIGN(4) :
      {
        *(.rodata)
      } > ROM AT>ROM
    .data  ALIGN(4):
      {
		HIDDEN(_data_start = .);
        *(.data)
		HIDDEN(_data_end = .);
      } > ROM AT>ROM
      
	PROVIDE(__global_pointer$ = (_data_start + _data_end) / 2);
    .bss ALIGN(4):
      {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
      } > ROM  AT>ROM
  }
