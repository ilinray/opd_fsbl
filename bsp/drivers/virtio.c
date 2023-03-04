/// @file
/// @brief implementation of virtio console interface
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2020. All rights reserved.

#include "arch.h"
#include "drivers/virtio.h"

#ifdef PLF_VIRTIO_BASE

#include <string.h>

enum {
    VQ_RX_BUF_SIZE = 16,
    VQ_RX_SIZE = 4,
    VQ_TX_SIZE = 2,
};

/* Magic value ("virt" string) - Read Only */
#define VIRTIO_MMIO_MAGIC_VALUE     0x000
/* Virtio device version - Read Only */
#define VIRTIO_MMIO_VERSION         0x004
/* Virtio device ID - Read Only */
#define VIRTIO_MMIO_DEVICE_ID       0x008
/* Virtio vendor ID - Read Only */
#define VIRTIO_MMIO_VENDOR_ID       0x00c
/* Bitmask of the features supported by the device (host)
 * (32 bits per set) - Read Only */
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010
/* Device (host) features set selector - Write Only */
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL 0x014
/* Bitmask of features activated by the driver (guest)
 * (32 bits per set) - Write Only */
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020
/* Activated features set selector - Write Only */
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL 0x024
/* Queue selector - Write Only */
#define VIRTIO_MMIO_QUEUE_SEL       0x030
/* Maximum size of the currently selected queue - Read Only */
#define VIRTIO_MMIO_QUEUE_NUM_MAX   0x034
/* Queue size for the currently selected queue - Write Only */
#define VIRTIO_MMIO_QUEUE_NUM       0x038
/* Ready bit for the currently selected queue - Read Write */
#define VIRTIO_MMIO_QUEUE_READY     0x044
/* Queue notifier - Write Only */
#define VIRTIO_MMIO_QUEUE_NOTIFY    0x050
/* Interrupt status - Read Only */
#define VIRTIO_MMIO_INTERRUPT_STATUS    0x060
/* Interrupt acknowledge - Write Only */
#define VIRTIO_MMIO_INTERRUPT_ACK   0x064
/* Device status register - Read Write */
#define VIRTIO_MMIO_STATUS      0x070
/* Selected queue's Descriptor Table address, 64 bits in two halves */
#define VIRTIO_MMIO_QUEUE_DESC_LOW  0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH 0x084
/* Selected queue's Available Ring address, 64 bits in two halves */
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW 0x090
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH    0x094
/* Selected queue's Used Ring address, 64 bits in two halves */
#define VIRTIO_MMIO_QUEUE_USED_LOW  0x0a0
#define VIRTIO_MMIO_QUEUE_USED_HIGH 0x0a4
/* Configuration atomicity value */
#define VIRTIO_MMIO_CONFIG_GENERATION   0x0fc
/* The config space is defined by each driver as
 * the per-driver configuration space - Read Write */
#define VIRTIO_MMIO_CONFIG      0x100

#define VIRTIO_MMIO_INT_VRING       (1 << 0)
#define VIRTIO_MMIO_INT_CONFIG      (1 << 1)

/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT       1
/* This marks a buffer as write-only (otherwise read-only). */
#define VIRTQ_DESC_F_WRITE      2
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT   4

/* Virtqueue descriptors: 16 bytes.
 * These can chain together via "next". */
struct virtq_desc {
    /* Address (guest-physical). */
    uint64_t addr;
    /* Length. */
    uint32_t len;
    /* The flags as indicated above. */
    uint16_t flags;
    /* We chain unused descriptors via this, too */
    uint16_t next;
};

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    //    uint16_t ring[];
    /* Only if VIRTIO_F_EVENT_IDX: le16 used_event; */
};

/* le32 is used here for ids for padding reasons. */
struct virtq_used_elem {
    /* Index of start of used descriptor chain. */
    uint32_t idx;
    /* Total length of the descriptor chain which was written to. */
    uint32_t len;
};

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    //    struct virtq_used_elem ring[];
    /* Only if VIRTIO_F_EVENT_IDX: le16 avail_event; */
};

#if PLF_VIRTIO_CONSOLE_BASE

void virtio_write32(unsigned offs, uint32_t val)
{
    *(volatile uint32_t*)(PLF_VIRTIO_CONSOLE_BASE + offs) = val;
}

uint32_t virtio_read32(unsigned offs)
{
    return *(volatile uint32_t*)(PLF_VIRTIO_CONSOLE_BASE + offs);
}

void virtio_write_addr(unsigned offs, const void* val)
{
    *(volatile uint32_t*)(PLF_VIRTIO_CONSOLE_BASE + offs) = (uint32_t)(uintptr_t)val;
#if __riscv_xlen > 32
    *(volatile uint32_t*)((PLF_VIRTIO_CONSOLE_BASE + 4) + offs) = (uint32_t)((uintptr_t)val >> 32);
#endif // __riscv_xlen > 32
}

uint8_t vq_buf[VQ_RX_SIZE * VQ_RX_BUF_SIZE];

volatile struct virtq_desc vq_desc[VQ_RX_SIZE] __attribute__((aligned(16)));

uint8_t vq_avail_buf[sizeof(struct virtq_avail) + VQ_RX_SIZE * sizeof(uint16_t)];
volatile struct virtq_avail *vq_avail = (struct virtq_avail*)vq_avail_buf;
volatile uint16_t *vq_avail_ring = (uint16_t*)(vq_avail_buf + sizeof(struct virtq_avail));

uint8_t vq_used_buf[sizeof(struct virtq_used) + VQ_RX_SIZE * sizeof(struct virtq_used_elem)];
volatile struct virtq_used *vq_used = (struct virtq_used*)vq_used_buf;
volatile struct virtq_used_elem *vq_used_ring = (struct virtq_used_elem *)(vq_used_buf + sizeof(struct virtq_used));

unsigned last_used = 0;
unsigned rx_len = 0;
unsigned rx_offs = 0;
uint8_t rx_buf[VQ_RX_BUF_SIZE];

volatile struct virtq_desc vq_tx_desc[VQ_TX_SIZE] __attribute__((aligned(16)));

uint8_t vq_tx_avail_buf[sizeof(struct virtq_avail) + VQ_TX_SIZE * sizeof(uint16_t)];
volatile struct virtq_avail *vq_tx_avail = (struct virtq_avail*)vq_tx_avail_buf;
volatile uint16_t *vq_tx_avail_ring = (uint16_t*)(vq_tx_avail_buf + sizeof(struct virtq_avail));

uint8_t vq_tx_used_buf[sizeof(struct virtq_used) + VQ_TX_SIZE * sizeof(struct virtq_used_elem)];
volatile struct virtq_used *vq_tx_used = (struct virtq_used*)vq_tx_used_buf;
volatile struct virtq_used_elem *vq_tx_used_ring = (struct virtq_used_elem *)(vq_tx_used_buf + sizeof(struct virtq_used));

uint8_t tx_buf[VQ_TX_SIZE];
unsigned last_tx_used = 0;

void virtio_console_init(void)
{
    // reset device
    virtio_write32(VIRTIO_MMIO_STATUS, 0);
    fence();
    virtio_write32(VIRTIO_MMIO_STATUS, 2 | 1); // status: ACK | DRIVER

    // init RX queue
    rx_offs = rx_len = 0;
    last_used = 0;
    virtio_write32(VIRTIO_MMIO_QUEUE_SEL, 0);
    virtio_write32(VIRTIO_MMIO_QUEUE_NUM, VQ_RX_SIZE);

    virtio_write_addr(VIRTIO_MMIO_QUEUE_DESC_LOW, (void*)vq_desc);
    virtio_write_addr(VIRTIO_MMIO_QUEUE_AVAIL_LOW, (void*)vq_avail_buf);
    virtio_write_addr(VIRTIO_MMIO_QUEUE_USED_LOW, (void*)vq_used_buf);

    for (int i = 0; i < VQ_RX_SIZE; ++i) {
        vq_desc[i].addr = (uint64_t)(uintptr_t)(vq_buf + VQ_RX_SIZE * i);
        vq_desc[i].len = VQ_RX_BUF_SIZE;
        vq_desc[i].flags = VIRTQ_DESC_F_WRITE;
        vq_desc[i].next = 0;
        vq_avail_ring[i] = i;
    }
    vq_avail->idx = VQ_RX_SIZE;
    fence();
    virtio_write32(VIRTIO_MMIO_QUEUE_READY, 1);

    // init TX queue
    last_tx_used = 0;
    virtio_write32(VIRTIO_MMIO_QUEUE_SEL, 1);
    virtio_write32(VIRTIO_MMIO_QUEUE_NUM, VQ_TX_SIZE);

    virtio_write_addr(VIRTIO_MMIO_QUEUE_DESC_LOW, (void*)vq_tx_desc);
    virtio_write_addr(VIRTIO_MMIO_QUEUE_AVAIL_LOW, (void*)vq_tx_avail_buf);
    virtio_write_addr(VIRTIO_MMIO_QUEUE_USED_LOW, (void*)vq_tx_used_buf);

    for (int i = 0; i < VQ_TX_SIZE; ++i) {
        vq_tx_desc[i].addr = (uint64_t)(uintptr_t)(tx_buf + i);
        vq_tx_desc[i].len = 1;
        vq_tx_desc[i].flags = 0;
        vq_tx_desc[i].next = 0;
        vq_tx_avail_ring[i] = i;
    }
    vq_tx_avail->idx = 0;
    fence();
    virtio_write32(VIRTIO_MMIO_QUEUE_READY, 1);
    fence();
    virtio_write32(VIRTIO_MMIO_STATUS, 4 | 2 | 1); // driver OK
    fence();
}

void virtio_console_shutdown(void)
{
    virtio_write32(VIRTIO_MMIO_STATUS, 0); // stop
    fence();
}

int virtio_console_getchar(void)
{
    if (last_used != vq_used->idx) {
        // process incoming data
        volatile struct virtq_used_elem *uitem = vq_used_ring + (last_used % VQ_RX_SIZE);
        if (VQ_RX_BUF_SIZE >= uitem->len + rx_len) {
            unsigned desc_idx = uitem->idx;
            volatile struct virtq_desc *desc = &vq_desc[desc_idx % VQ_RX_SIZE];
            memcpy(rx_buf + rx_len, (void*)(uintptr_t)desc->addr, uitem->len);
            rx_len += uitem->len;
            ++last_used;
            // return desc to avail queue

            // get next dscr
            int aidx = vq_avail->idx % VQ_RX_SIZE;
            // init dscr
            vq_avail_ring[aidx] = desc_idx;
            fence();
            vq_avail->idx += 1;
        }
    }

    if (rx_offs < rx_len) {
        // return chached incoming data
        int ch = rx_buf[rx_offs];
        ++rx_offs;
        if (rx_offs == rx_len)
            rx_offs = rx_len = 0;
        return ch;
    }

    return -1;
}

void virtio_console_tx_flush(void)
{
    while (last_tx_used != vq_tx_used->idx) {
        // grab transimtted descr
        volatile struct virtq_used_elem *uitem = vq_tx_used_ring + (last_tx_used % VQ_TX_SIZE);
        unsigned desc_idx = uitem->idx;
        // get next dscr
        int aidx = vq_tx_avail->idx % VQ_TX_SIZE;
        // push back dscr
        vq_tx_avail_ring[aidx] = desc_idx;
        ++last_tx_used;
        fence();
    }
}

void virtio_console_putchar(int ch)
{
    if (last_tx_used != vq_tx_used->idx) {
        // grab transimtted descr
        volatile struct virtq_used_elem *uitem = vq_tx_used_ring + (last_tx_used % VQ_TX_SIZE);
        unsigned desc_idx = uitem->idx;
        // get next dscr
        int aidx = vq_tx_avail->idx % VQ_TX_SIZE;
        // push back dscr
        vq_tx_avail_ring[aidx] = desc_idx;
        ++last_tx_used;
        fence();
    }

    unsigned txidx = vq_tx_avail->idx;
    unsigned desc_idx = vq_tx_avail_ring[txidx % VQ_TX_SIZE];
    tx_buf[desc_idx] = ch;
    fence();
    vq_tx_avail->idx = txidx + 1;

    virtio_write32(VIRTIO_MMIO_QUEUE_NOTIFY, 1);
}
#endif // PLF_VIRTIO_CONSOLE_BASE
#endif // PLF_VIRTIO_BASE
