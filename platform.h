#ifndef PLATFORM_H
#define PLATFORM_H

#define BIT(N) (1 << N)

#define ct_assert(expr) _Static_assert(expr, "")


typedef unsigned char U8;
typedef unsigned short int U16;
typedef unsigned long int U32;
typedef char I8;
typedef short int I16;
typedef long int I32;

ct_assert(sizeof(U8 ) == 1);
ct_assert(sizeof(U16) == 2);
ct_assert(sizeof(U32) == 4);
ct_assert(sizeof(I8 ) == 1);
ct_assert(sizeof(I16) == 2);
ct_assert(sizeof(I32) == 4);

extern U32 int_table[];
extern U32 exc_table[];

#endif
