/// @file
/// @brief 16550 registers
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2018. All rights reserved.

#ifndef SCR_BSP_16550_REGS_H
#define SCR_BSP_16550_REGS_H

#if PLF_UART0_MMIO == 8
#define UART_16550_REG_WIDTH 1
#elif PLF_UART0_MMIO == 32
#define UART_16550_REG_WIDTH 4
#else
#error Incorrect PLF UART MMIO width
#endif


// 16550 registers (offset)
#define UART_16550_RXD        (0 * UART_16550_REG_WIDTH)  // receive data
#define UART_16550_TXD        (0 * UART_16550_REG_WIDTH)  // transmit data
#define UART_16550_IER        (1 * UART_16550_REG_WIDTH)  // interrupt enable register
#define UART_16550_IIR        (2 * UART_16550_REG_WIDTH)  // interrupt identification register
#define UART_16550_FCR        (2 * UART_16550_REG_WIDTH)  // FIFO control register
#define UART_16550_LCR        (3 * UART_16550_REG_WIDTH)  // line control register (control)
#define UART_16550_MCR        (4 * UART_16550_REG_WIDTH)  // modem control register
#define UART_16550_LSR        (5 * UART_16550_REG_WIDTH)  // line status register (status)
#define UART_16550_MSR        (6 * UART_16550_REG_WIDTH)  // modem status register
#define UART_16550_SCRATCH    (7 * UART_16550_REG_WIDTH)  // scratch register
#define UART_16550_DIV_LO     (0 * UART_16550_REG_WIDTH)  // baud rate divisor register, low
#define UART_16550_DIV_HI     (1 * UART_16550_REG_WIDTH)  // baud rate divisor register, high

// 16550 FCR bits
#define UART_16550_FCR_RT_1   (0 << 6) // RX FIFO trigger level: 1 byte
#define UART_16550_FCR_RT_4   (1 << 6) // RX FIFO trigger level: 4 bytes
#define UART_16550_FCR_RT_8   (2 << 6) // RX FIFO trigger level: 8 bytes
#define UART_16550_FCR_RT_14  (3 << 6) // RX FIFO trigger level: 14 bytes
#define UART_16550_FCR_RMASK  (3 << 6) // RX FIFO trigger level mask bits
#define UART_16550_FCR_T_RST  (1 << 2) // reset TX FIFO
#define UART_16550_FCR_R_RST  (1 << 1) // reset RX FIFO
#define UART_16550_FCR_EN     (1 << 0) // FIFO enable
// FCR initial value: enabled
#define UART_16550_FCR_INIT   (UART_16550_FCR_RT_1 | UART_16550_FCR_EN)

// 16550 IER bits
#define UART_16550_IER_MODEM_STATUS_EN    (1 << 3) // Modem status interrupt enable
#define UART_16550_IER_RX_LINE_STATUS_EN  (1 << 2) // Receiver line status interrupt enable
#define UART_16550_IER_TX_REG_EMPTY_EN    (1 << 1) // Transmitter holding register empty interrupt enable
#define UART_16550_IER_RX_DATA_RDY_EN     (1 << 0) // Received data available interrupt enable
// IER initial value: disabled
#define UART_16550_IER_INIT   (0)

// 16550 IIR bits
#define UART_16550_IIR_NONE            (1 << 0) // None
#define UART_16550_IIR_RX_LINE_STATUS  (6 << 0) // Receiver line status interrupt
#define UART_16550_IIR_RX_DATA_RDY     (4 << 0) // Received data available
#define UART_16550_IIR_TX_REG_EMPTY    (2 << 0) // Transmitter holding register empty
#define UART_16550_IIR_MODEM_STATUS    (0 << 0) // Modem status interrupt

// 16550 line control register bits
#define UART_16550_LCR_DIVL   (1 << 7) // divisor latch access
#define UART_16550_LCR_SP     (1 << 5) // sticky parity
#define UART_16550_LCR_EPS    (1 << 4) // even parity select
#define UART_16550_LCR_PE     (1 << 3) // parity enable
#define UART_16550_LCR_SBN    (1 << 2) // number of stop bits (0 - 1, 1 - 1.5/2)
#define UART_16550_LCR_CL8    (3 << 0) // character length: 8
#define UART_16550_LCR_CL7    (2 << 0) // character length: 7
#define UART_16550_LCR_CL6    (1 << 0) // character length: 6
#define UART_16550_LCR_CL5    (0 << 0) // character length: 5
// LCR initial value: 8n1
#define UART_16550_LCR_INIT   (UART_16550_LCR_CL8)

// 16550 line status register bits
#define UART_16550_LSR_DR     (1 << 0) // rx not empty
#define UART_16550_LSR_ERR_OE (1 << 1) // overrun error
#define UART_16550_LSR_ERR_PE (1 << 2) // parity error
#define UART_16550_LSR_THRE   (1 << 5) // tx not full
#define UART_16550_LSR_TEMT   (1 << 6) // tx empty

#ifndef __ASSEMBLER__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if PLF_UART0_MMIO == 8
typedef uint8_t scr_uart_port_t;
#elif PLF_UART0_MMIO == 32
typedef uint32_t scr_uart_port_t;
#else
#error Incorrect PLF UART MMIO width
#endif

typedef struct _uart16550 {
    union {
        scr_uart_port_t data;
        scr_uart_port_t divlo;
    };
    union {
        scr_uart_port_t ier;
        scr_uart_port_t divhi;
    };
    union {
        scr_uart_port_t iir;
        scr_uart_port_t fcr;
    };
    scr_uart_port_t lcr;
    scr_uart_port_t mcr;
    scr_uart_port_t lsr;
    scr_uart_port_t msr;
    scr_uart_port_t scratch;
} volatile uart16550;

#ifdef __cplusplus
}
#endif
#endif // !__ASSEMBLER__
#endif // SCR_BSP_16550_REGS_H
