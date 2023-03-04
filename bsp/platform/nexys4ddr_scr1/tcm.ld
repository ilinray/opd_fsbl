/** @file
*  @brief Bare metal linker script
*
*  Syntacore SCR* infra
*
*  @copyright (C) Syntacore 2015-2022. All rights reserved.
*/

MEMORY {
  TCM (rwx) : ORIGIN = 0xF0000000, LENGTH = 64K
}

REGION_ALIAS("REGION_TEXT",   TCM);
REGION_ALIAS("REGION_RODATA", TCM);
REGION_ALIAS("REGION_DATA",   TCM);
REGION_ALIAS("REGION_BSS",    TCM);
REGION_ALIAS("REGION_STACK",  TCM);

INCLUDE "bsp0.lds"
