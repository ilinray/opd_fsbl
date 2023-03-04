/** @file
*  @brief Bare metal linker script
*
*  Syntacore SCR* infra
*
*  @copyright (C) Syntacore 2015-2022. All rights reserved.
*/

MEMORY {
  RAM (rwx) : ORIGIN = 0x0, LENGTH = 128M
}

REGION_ALIAS("REGION_TEXT",   RAM);
REGION_ALIAS("REGION_RODATA", RAM);
REGION_ALIAS("REGION_DATA",   RAM);
REGION_ALIAS("REGION_BSS",    RAM);
REGION_ALIAS("REGION_STACK",  RAM);

INCLUDE "bsp0.lds"
