/// @file
/// Syntacore SCR* framework
///
/// @copyright (C) Syntacore 2015-2021. All rights reserved.
///
/// @brief ICCM defs and inline funcs

#ifndef SCR_BSP_ICCM_H
#define SCR_BSP_ICCM_H

#include "arch.h"

#ifndef SCR_ICCM_CSR_MBASE
#define SCR_ICCM_CSR_MBASE   0xbd8
#endif // SCR_ICCM_CSR_MBASE

// M mode CSRs
#define SCR_ICCM_CSR_MADDR   (SCR_ICCM_CSR_MBASE + 0)
#define SCR_ICCM_CSR_MSTATUS (SCR_ICCM_CSR_MBASE + 1)
#define SCR_ICCM_CSR_MRDATA  (SCR_ICCM_CSR_MBASE + 2)
#define SCR_ICCM_CSR_MWDATA  (SCR_ICCM_CSR_MBASE + 3)

#ifndef __ASSEMBLER__

#include "csr.h"
#include <stdbool.h>

//---------------------------
// ICCM registers access
//---------------------------

// set destination slot number
static inline void iccm_set_slot(unsigned dst)
{
    write_csr(SCR_ICCM_CSR_MADDR, dst);
}

// read ICCM status register
// bit 0: remote msgbox (by slot number) empty status
static inline unsigned long iccm_status(void)
{
    return read_csr(SCR_ICCM_CSR_MSTATUS);
}

// write ICCM WDATA register
// send message to destination (by addr)
static inline void iccm_write_data(long data)
{
    write_csr(SCR_ICCM_CSR_MWDATA, data);
}

// peek incoming msgbox status (MSB), source slot num (low bits) and data
// MSB = 1:
//   incoming msgbox is empty,
//   low slot_mask_bits = max slot (dst/src) number
// MSB = 0:
//   message value shifted by n_shift_bits,
//   low slot_mask_bits = source slot addr
// where slot_mask_bits = log2(max slot number)
static inline long iccm_peek_data(void)
{
    return (long)read_csr(SCR_ICCM_CSR_MRDATA);
}

// read incoming msgbox status (MSB), source slot num (low bits) and data
// incoming msgbox will be availible for new messages
static inline long iccm_get_data(void)
{
    return (long)swap_csr(SCR_ICCM_CSR_MRDATA, 1);
}

//---------------------------
// primitives
//---------------------------

static inline bool iccm_check_avail(unsigned dst)
{
    iccm_set_slot(dst);
    return iccm_status() == 0;
}

static inline bool iccm_send_nowait(unsigned dst, long val)
{
    bool avail = iccm_check_avail(dst);

    if (avail)
        iccm_write_data(val);

    return avail;
}

static inline int iccm_send(unsigned dst, long val)
{
    int t = 1000;

    iccm_set_slot(dst);

    while (t) {
        if (iccm_status() == 0) {
            iccm_write_data(val);
            break;
        }
        --t;
    }

    return t;
}

static inline void iccm_send_sync(unsigned dst, long val)
{
    iccm_set_slot(dst);

    while (iccm_status() != 0);
    iccm_write_data(val);
    while (iccm_status() != 0);
}

static inline long iccm_peek(void)
{
    return iccm_peek_data();
}

static inline long iccm_read_nowait(void)
{
    return iccm_get_data();
}

static inline long iccm_read(void)
{
    long msg;
    do {
        msg = iccm_read_nowait();
    } while (msg < 0);

    return msg;
}

// read all incoming data untill msgbox status != empty
// return incoming msgbox status (max slot number)
static inline long iccm_flush_read(void)
{
    for (int i = 32; i > 0; --i) {
        long data = iccm_read_nowait();
        if (data < 0)
            return data;
    }
    return 0; // timeout
}

// calculate slot_mask_bits from max slot number
// slot_mask_bits = log2(max slot number)
static inline unsigned iccm_slot_mask_bits(long msg)
{
    unsigned slot_mask_bits = 0;
    unsigned long max_slot_num = msg & 0xff;

    while (max_slot_num) {
        ++slot_mask_bits;
        max_slot_num >>= 1;
    }

    return slot_mask_bits;
}

#endif // __ASSEMBLER__
#endif // SCR_BSP_ICCM_H
