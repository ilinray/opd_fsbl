#ifndef BL_STDLIB
#define BL_STDLIB
int bl_sprintf(char *str, const char *format, ...);
void bl_puts(char *str);
void bl_putc(char c);
void bl_crlf();
#define CIN_EMPTY (I8)-1
#endif
