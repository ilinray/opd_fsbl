/// @file
/// @brief platform code/data relocation func
/// Syntacore SCR* infra
///
/// @copyright (C) Syntacore 2015-2020. All rights reserved.

#include <sys/errno.h>
#include <string.h>

#include "memasm.h"

struct Plf_reloc_info {
    char *start;
    char *end;
    char *load;
};

extern char __relrodata_start[] __attribute__((weak));
extern char __relrodata_end[] __attribute__((weak));
extern char __relrodata_load_start[] __attribute__((weak));
extern char __reldata_start[] __attribute__((weak));
extern char __reldata_end[] __attribute__((weak));
extern char __reldata_load_start[] __attribute__((weak));

volatile const __initconst struct Plf_reloc_info ro_reloc = {
    __relrodata_start, __relrodata_end, __relrodata_load_start
};
volatile const __initconst struct Plf_reloc_info data_reloc = {
    __reldata_start, __reldata_end, __reldata_load_start
};

void plf_init_relocate(void) __attribute__((__section__(".init.text.plf_init_relocate")));

void plf_init_relocate(void)
{
    if (ro_reloc.start != ro_reloc.load) {
        memcpy(ro_reloc.start, ro_reloc.load, (size_t)(ro_reloc.end - ro_reloc.start));
    }
    if (data_reloc.start != data_reloc.load) {
        memcpy(data_reloc.start, data_reloc.load, (size_t)(data_reloc.end - data_reloc.start));
    }
}
