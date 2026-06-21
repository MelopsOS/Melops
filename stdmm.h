#ifndef STDMM_H
#define STDMM_H

#include "multiboot.h"

void pmm_init(struct multiboot_info *mbm);
void *pmm_alloc_block(void);
void pmm_free_block(void *ptr);

#endif