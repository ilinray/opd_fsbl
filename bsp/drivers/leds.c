/// @file
/// @brief LEDs funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#include "drivers/leds.h"

#include "drivers/rtc.h"

#ifdef PLF_PINLED_ADDR
void leds_dimming(pinled_port_mem v, unsigned long interval_ms, unsigned duty)
{
    duty = 1U << ((duty & 7) + 2);

    for (unsigned long i = 0; i < interval_ms; ++i) {
        leds_set(v);
        rtc_delay_us(duty);
        leds_set(0);
        rtc_delay_us(999 - duty);
    }
}
#endif /* PLF_PINLED_ADDR */

#ifdef PLF_HEXLED_ADDR

static const unsigned HEX_DIGITS_TABLE[16] = {
    HEXLED_0,
    HEXLED_1,
    HEXLED_2,
    HEXLED_3,
    HEXLED_4,
    HEXLED_5,
    HEXLED_6,
    HEXLED_7,
    HEXLED_8,
    HEXLED_9,
    HEXLED_A,
    HEXLED_B,
    HEXLED_C,
    HEXLED_D,
    HEXLED_E,
    HEXLED_F
};

#if PLF_HEXLED_PORT_WIDTH == 4
typedef uint32_t hexled_port_mem;
#elif PLF_HEXLED_PORT_WIDTH == 2
typedef uint16_t hexled_port_mem;
#else
typedef uint8_t hexled_port_mem;
#endif

struct hex_seg_map {
    uintptr_t addr;
    unsigned shift;
#ifdef PLF_HEXLED_WO
    unsigned refn;
    hexled_port_mem cache;
#endif // PLF_HEXLED_WO
};

static
#ifndef PLF_HEXLED_WO
const
#endif // PLF_HEXLED_WO
struct hex_seg_map HEX_MAP[] = {
    PLF_HEXLED_ADDR_MAP
};

void leds_hex(unsigned n, unsigned v)
{
    if (n < sizeof(HEX_MAP) / sizeof(*HEX_MAP)) {
        volatile hexled_port_mem *p = (volatile hexled_port_mem*)(HEX_MAP[n].addr);
        unsigned shift = HEX_MAP[n].shift;
        hexled_port_mem mask = ~(hexled_port_mem)(HEXLED_ALL << shift);
#ifdef PLF_HEXLED_WO
        unsigned ref = HEX_MAP[n].refn;
       HEX_MAP[ref].cache = (HEX_MAP[ref].cache & mask) | ((v ^ PLF_HEXLED_INV) << shift);
       *p = HEX_MAP[ref].cache;
#else // PLF_HEXLED_WO
       *p = (*p & mask) | ((v ^ PLF_HEXLED_INV) << shift);
#endif // PLF_HEXLED_WO
    }
}

void leds_hex_digit(unsigned n, unsigned v)
{
    leds_hex(n, HEX_DIGITS_TABLE[v & 0xf]);
}

void leds_hex_all(unsigned v)
{
    for (unsigned i = 0; i < sizeof(HEX_MAP) / sizeof(*HEX_MAP); ++i)
        leds_hex(i, v);
}
#endif // PLF_HEXLED_ADDR

#if defined(PLF_HEXLED_ADDR) || defined(PLF_PINLED_ADDR)
void leds_init(void)
{
#ifdef PLF_HEXLED_ADDR
    for (unsigned i = 0; i < sizeof(HEX_MAP) / sizeof(*HEX_MAP); ++i) {
#ifdef PLF_HEXLED_WO
        // init refn
        HEX_MAP[i].refn = i;
        for (unsigned j = 0; j < i; ++j) {
            if (HEX_MAP[j].addr == HEX_MAP[i].addr) {
                HEX_MAP[i].refn = HEX_MAP[j].refn;
                break;
            }
        }
#endif // PLF_HEXLED_WO
        leds_hex(i, HEXLED_ALL);
    }
#endif // PLF_HEXLED_ADDR
#ifdef PLF_PINLED_ADDR
    leds_set((pinled_port_mem)~0UL);
#endif // PLF_PINLED_ADDR
    rtc_delay_us(63 * 4096); // ~258ms
#ifdef PLF_HEXLED_ADDR
    leds_hex_all(HEXLED_EMPTY);
#endif // PLF_HEXLED_ADDR
#ifdef PLF_PINLED_ADDR
    leds_set(0UL);
#endif // PLF_PINLED_ADDR
}
#endif // PLF_HEXLED_ADDR || PLF_PINLED_ADDR
