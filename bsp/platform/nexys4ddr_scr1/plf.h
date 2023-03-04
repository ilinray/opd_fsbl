/// @file
/// @brief platform specific configurations
/// Syntacore SCR* framework
///
/// @copyright (C) Syntacore 2015-2018. All rights reserved.

#ifndef PLATFORM_NEXYS4DDR_SCR1_H
#define PLATFORM_NEXYS4DDR_SCR1_H

#ifndef PLF_SYS_CLK
#define PLF_SYS_CLK 30000000
#endif

#define PLF_CORE_VARIANT_SCR 1
#define PLF_CORE_VARIANT_SCR1 1
#define PLF_CORE_VARIANT SCR1

#define PLF_IMPL_STR "Syntacore FPGA"

// memory configuration
//----------------------
#define PLF_MEM_BASE     (0)
#define PLF_MEM_SIZE     (128*1024*1024)
#define PLF_MEM_ATTR     0
#define PLF_MEM_NAME     "DDR"

#define PLF_TCM_BASE     (0xf0000000)
#define PLF_TCM_SIZE     (128*1024)
#define PLF_TCM_ATTR     0
#define PLF_TCM_NAME     "TCM"

#define PLF_MTIMER_BASE  (0xf0040000)
#define PLF_MTIMER_SIZE  (0x1000)
#define PLF_MTIMER_ATTR  0
#define PLF_MTIMER_NAME  "MTimer"

#define PLF_MMIO_BASE    (0xff000000)
#define PLF_MMIO_SIZE    (0x100000)
#define PLF_MMIO_ATTR    0
#define PLF_MMIO_NAME    "MMIO"

#define PLF_OCRAM_BASE   (0xffff0000)
#define PLF_OCRAM_SIZE   (64*1024)
#define PLF_OCRAM_ATTR   0
#define PLF_OCRAM_NAME   "On-Chip RAM"

// FPGA UART port
#define PLF_UART0_BASE   (PLF_MMIO_BASE + 0x10000)
#define PLF_UART0_16550
// FPGA build ID
#define PLF_BLD_ID_ADDR  (PLF_MMIO_BASE)

#define PLF_PINLED_ADDR         (PLF_MMIO_BASE + 0x1000)
#define PLF_HEXLED_ADDR         (PLF_MMIO_BASE + 0x2000)
#define PLF_DIP_ADDR            (PLF_MMIO_BASE + 0x8)
#define PLF_PINLED_PORT_WIDTH 4
#define PLF_HEXLED_PORT_WIDTH 4

#define PLF_HEXLED_ADDR_MAP                     \
    {(PLF_HEXLED_ADDR + 0x18), 24},             \
    {(PLF_HEXLED_ADDR + 0x18), 16},             \
    {(PLF_HEXLED_ADDR + 0x18), 8},              \
    {(PLF_HEXLED_ADDR + 0x18), 0},              \
    {(PLF_HEXLED_ADDR + 0x0), 24},              \
    {(PLF_HEXLED_ADDR + 0x0), 16},              \
    {(PLF_HEXLED_ADDR + 0x0), 8},               \
    {(PLF_HEXLED_ADDR + 0x0), 0}

#define PLF_HEXLED_INV 0xff
#define PLF_PINLED_INV 0

#define HEXLED_SEG_A 0x01
#define HEXLED_SEG_B 0x02
#define HEXLED_SEG_C 0x04
#define HEXLED_SEG_D 0x08
#define HEXLED_SEG_E 0x10
#define HEXLED_SEG_F 0x20
#define HEXLED_SEG_G 0x40
#define HEXLED_SEG_P 0x80

// external interrupt lines
#define PLF_INTLINE_UART0 0

#endif // PLATFORM_NEXYS4DDR_SCR1_H
