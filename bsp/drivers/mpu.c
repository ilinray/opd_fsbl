/// @file
/// @brief MMU init function
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2018. All rights reserved.

#include "drivers/mpu.h"

#ifdef PLF_MEM_MAP
const scr_mpu_region_info mem_regions[] = {
    PLF_MEM_MAP,
    {0, 0, 0, 0}
};
#endif // PLF_MEM_MAP

#if PLF_HAVE_MPU
void mpu_init(void)
{
#ifdef PLF_MEM_MAP
    scr_mpu_setup(mem_regions);
#endif // PLF_MEM_MAP
}
#endif // PLF_HAVE_MPU
