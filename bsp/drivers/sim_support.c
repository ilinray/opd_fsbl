/// @file
/// @brief implementation of sim console
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2022. All rights reserved.

#include "arch.h"
#include "drivers/uart_sim.h"

#ifdef PLF_SC_SIM_EXIT
#include <stddef.h>

void sim_halt_handler(int ret_code)
{
    asm volatile(".global sc_exit; sc_exit: nop");
    *(volatile size_t*)PLF_SC_SIM_EXIT = ret_code;
    ifence();
}
#endif // PLF_SC_SIM_EXIT
