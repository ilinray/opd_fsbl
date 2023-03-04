/// @file
/// @brief Cache routines
/// Syntacore SCR infra
///
/// @copyright (C) Syntacore 2015-2019. All rights reserved.

#include "drivers/cache.h"
#include <stdint.h>
#include <stdio.h>

#ifdef PLF_CACHE_CFG
static int cacheinfo2str(char *buf, size_t len, unsigned long info)
{
    unsigned banks = (unsigned)(1 + ((info >> 16) & 0xf));
    unsigned ways = 1U << (info & 0x7);
    unsigned linesz = 1U << ((info >> 4) & 0xf);
    unsigned lines = 1U << ((info >> 8) & 0x1f);
    unsigned size_kb = (lines * linesz * ways * banks) / 1024;

    return snprintf(buf, len,
                    "%uK, %u-way, %u-byte line",
                    size_kb, ways, linesz
        );
}
#endif /* PLF_CACHE_CFG */

int cache_l1i_info(char *buf, size_t len)
{
    int sz0 = 0;
#ifdef PLF_CACHE_CFG
    if (cache_l1_available()) {
        unsigned long cstate = read_csr(SCR_CSR_CACHE_GLBL);
        unsigned long cinfo = read_csr(SCR_CSR_CACHE_DSCR_L1) & 0xffff;

        if (cinfo) {
            len -= 1;
            sz0 = snprintf(buf, len, "[%04lx] ", cinfo);

            if (sz0 >= 0 && (size_t)sz0 < len) {
                int sz1 = cacheinfo2str(buf + sz0, len - (size_t)sz0, cinfo);
                if (sz1 >= 0) {
                    sz0 += sz1;
                    if (sz0 >= 0 && (size_t)sz0 < len) {
                        sz1 = snprintf(buf + sz0, len - (size_t)sz0, ", %sabled",
                                       (cstate & CACHE_GLBL_L1I_EN) ? "en" : "dis");
                        if (sz1 >= 0)
                            sz0 += sz1;
                    }
                }
            }
        }
    }
#endif /* PLF_CACHE_CFG */

    if (len > 0 && sz0 >= 0)
        buf[sz0] = '\0';

    return sz0;
}

int cache_l1d_info(char *buf, size_t len)
{
    int sz0 = 0;
#ifdef PLF_CACHE_CFG
    if (cache_l1_available()) {
        unsigned long cstate = read_csr(SCR_CSR_CACHE_GLBL);
        unsigned long cinfo = (read_csr(SCR_CSR_CACHE_DSCR_L1) >> 16) & 0xffff;

        if (cinfo) {
            len -= 1;
            sz0 = snprintf(buf, len, "[%04lx] ", cinfo);

            if (sz0 >= 0 && (size_t)sz0 < len) {
                int sz1 = cacheinfo2str(buf + sz0, len - (size_t)sz0, cinfo);
                if (sz1 >= 0) {
                    sz0 += sz1;
                    if (sz0 >= 0 && (size_t)sz0 < len) {
                        sz1 = snprintf(buf + sz0, len - (size_t)sz0, ", %sabled",
                                       (cstate & CACHE_GLBL_L1D_EN) ? "en" : "dis");
                        if (sz1 >= 0)
                            sz0 += sz1;
                    }
                }
            }
        }
    }
#endif /* PLF_CACHE_CFG */

    if (len > 0 && sz0 >= 0)
        buf[sz0] = '\0';

    return sz0;
}

#if PLF_L2CTL_BASE

//-----------------
// L2$ registers

#define L2_CSR_VER_IDX   0
#define L2_CSR_DESCR_IDX 1
#define L2_CSR_EN_IDX    4
#define L2_CSR_FLUSH_IDX 5
#define L2_CSR_INV_IDX   6

#define L2_CSR_VER_OFFS   (L2_CSR_VER_IDX * 4)
#define L2_CSR_DESCR_OFFS (L2_CSR_DESCR_IDX * 4)
#define L2_CSR_EN_OFFS    (L2_CSR_EN_IDX * 4)
#define L2_CSR_FLUSH_OFFS (L2_CSR_FLUSH_IDX * 4)
#define L2_CSR_INV_OFFS   (L2_CSR_INV_IDX * 4)

#define L2_CSR_VER   (PLF_L2CTL_BASE + L2_CSR_VER_OFFS)
#define L2_CSR_DESCR (PLF_L2CTL_BASE + L2_CSR_DESCR_OFFS)
#define L2_CSR_EN    (PLF_L2CTL_BASE + L2_CSR_FLUSH_OFFS)
#define L2_CSR_FLUSH (PLF_L2CTL_BASE + L2_CSR_FLUSH_OFFS)
#define L2_CSR_INV   (PLF_L2CTL_BASE + L2_CSR_INV_OFFS)

//-----------------
// L2$ routines

void plf_l2cache_enable(void)
{
    // init L2$: disable, confirm  state, invalidate, confirm, enable, confirm
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;
    uint32_t cbmask;

    if (!l2ctl[L2_CSR_VER_IDX])
        return; // cache not exists or not supported

    cbmask = l2ctl[L2_CSR_DESCR_IDX];

    cbmask = (1U << (((cbmask >> 16) & 0xf) + 1)) - 1;

    // disable L2$
    l2ctl[L2_CSR_EN_IDX] = 0;
    // confirm state
    while (l2ctl[L2_CSR_EN_IDX]);
    // invalidate
    l2ctl[L2_CSR_INV_IDX] = cbmask;
    // confirm state
    while (l2ctl[L2_CSR_INV_IDX]);
    // enable
    l2ctl[L2_CSR_EN_IDX] = cbmask;
    // confirm state
    while (l2ctl[L2_CSR_EN_IDX] != cbmask);
}

void plf_l2cache_disable(void)
{
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

    if (!l2ctl[L2_CSR_VER_IDX])
        return; // cache not exists or not supported

    // sync (flush) current code
    // FIXME: cache flush before disabing

    // disable
    l2ctl[L2_CSR_EN_IDX] = 0;
    // confirm state
    while (l2ctl[L2_CSR_EN_IDX]);
    // flush overall
    l2ctl[L2_CSR_FLUSH_IDX] = ~0U;
    // confirm state
    while (l2ctl[L2_CSR_FLUSH_IDX]);
    // invalidate
    l2ctl[L2_CSR_INV_IDX] = ~0U;
    // confirm state
    while (l2ctl[L2_CSR_INV_IDX]);
}

bool plf_l2cache_is_enabled(void)
{
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

    return l2ctl[L2_CSR_VER_IDX] && l2ctl[L2_CSR_EN_IDX];
}

void plf_l2cache_init(void)
{
    plf_l2cache_enable();
}

int cache_l2_info(char *buf, size_t len)
{
    volatile uint32_t *l2ctl = (uint32_t*)PLF_L2CTL_BASE;

    unsigned l2dscr = l2ctl[L2_CSR_DESCR_IDX];

    if (!l2dscr)
        return 0;

    unsigned banks = 1 + ((l2dscr >> 16) & 0xf);
    unsigned ways = 1U << (l2dscr & 0x7);
    unsigned linesz = 1U << ((l2dscr >> 4) & 0xf);
    unsigned lines = 1U << ((l2dscr >> 8) & 0x1f);
    unsigned cores = 1 + ((l2dscr >> 28) & 0xf);
    /* unsigned type = (l2dscr >> 13) & 0x7; */
    unsigned size_kb = (lines * linesz * ways * banks) / 1024;

    /* static const char *cache_types[8] = { */
    /*     [0] = "RO", */
    /*     [1] = "Write-through, no allocate", */
    /*     [7] = "Write-back, allocate, L1 inclusive" */
    /* }; */

    int ssz = snprintf(buf, len,
                       "[%08x %08x] %uK, %u-way, %u-byte line, %s",
                       (unsigned)(l2ctl[L2_CSR_VER_IDX]), l2dscr,
                       size_kb, ways, linesz,
                       (cores > 1 ? "shared" : "dedicated")
        );

    if (cores > 1 && (size_t)ssz < len)
        ssz += snprintf(buf + ssz, len - (size_t)ssz, " (%u cores)", cores);

    if ((size_t)ssz < len)
        ssz += snprintf(buf + ssz, len - (size_t)ssz, ", status: %x",
                        (unsigned)l2ctl[L2_CSR_EN_IDX]);
    return ssz;
}

#else  /* PLF_L2CTL_BASE */

int cache_l2_info(char *buf, size_t len)
{
    if (len > 0)
        buf[0] = 0;
    return 0;
}

#endif /* PLF_L2CTL_BASE */
