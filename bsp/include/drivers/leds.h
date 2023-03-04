/// @file
/// @brief LEDs defs and inline funcs
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#ifndef SCR_BSP_LEDS_H
#define SCR_BSP_LEDS_H

#include "arch.h"

#ifdef PLF_HEXLED_ADDR
/*
 *  Hex led codes:
 *   A --
 *  F |  | B
 *   G --
 *  E |  | C
 *   D -- o P
*/

// HEX symbols
#define HEXLED_0 ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_D))
#define HEXLED_1 ((HEXLED_SEG_B) | (HEXLED_SEG_C))
#define HEXLED_2 ((HEXLED_SEG_E) | (HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_3 ((HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_4 ((HEXLED_SEG_F) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_G))
#define HEXLED_5 ((HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_6 ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_7 ((HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_C))
#define HEXLED_8 ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_9 ((HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_A ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_G))
#define HEXLED_B ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_C ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_D))
#define HEXLED_D ((HEXLED_SEG_E) | (HEXLED_SEG_B) | (HEXLED_SEG_C) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_E ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_D) | (HEXLED_SEG_G))
#define HEXLED_F ((HEXLED_SEG_E) | (HEXLED_SEG_F) | (HEXLED_SEG_A) | (HEXLED_SEG_G))
#define HEXLED_DP (HEXLED_SEG_P)
#define HEXLED_MINUS (HEXLED_SEG_G)
#define HEXLED_EMPTY (0)
#define HEXLED_ALL (HEXLED_8 | HEXLED_DP)

#endif // PLF_HEXLED_ADDR

#ifndef __ASSEMBLER__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef PLF_PINLED_ADDR

#if PLF_PINLED_PORT_WIDTH == 4
typedef uint32_t pinled_port_mem;
#elif PLF_PINLED_PORT_WIDTH == 2
typedef uint16_t pinled_port_mem;
#else
typedef uint8_t pinled_port_mem;
#endif

static inline void leds_set(pinled_port_mem v)
{
    *(volatile pinled_port_mem*)(PLF_PINLED_ADDR) = v ^ PLF_PINLED_INV;
}
/* 1kHz PWM LEDs dimming */
/* duty range: 0-7 */
void leds_dimming(pinled_port_mem v, unsigned long interval_ms, unsigned duty);
#else /* PLF_PINLED_ADDR */
#define leds_set(v) do {} while (0)
#define leds_dimming(v, interval_ms, duty) do {} while (0)
#endif /* PLF_PINLED_ADDR */

#ifdef PLF_HEXLED_ADDR
void leds_hex(unsigned n, unsigned v);
void leds_hex_digit(unsigned n, unsigned v);
void leds_hex_all(unsigned v);
#else // PLF_HEXLED_ADDR
#define leds_hex(n, v) do {} while (0)
#define leds_hex_digit(n, v) do {} while (0)
#define leds_hex_all(v) do {} while (0)
#endif // PLF_HEXLED_ADDR

#if defined(PLF_HEXLED_ADDR) || defined(PLF_PINLED_ADDR)
void leds_init(void);
#else // PLF_HEXLED_ADDR || PLF_PINLED_ADDR
#define leds_init() do {} while (0)
#endif // PLF_HEXLED_ADDR || PLF_PINLED_ADDR

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !__ASSEMBLER__
#endif // SCR_BSP_LEDS_H
