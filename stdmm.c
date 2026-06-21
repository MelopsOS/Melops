#include <stddef.h>
#include <stdint.h>
#include "stdmm.h"
#include "stdio.h"
#include "multiboot.h"

#define BLOCK_SIZE 4096

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

uint32_t *pmm_stack = (uint32_t *)1;

int32_t pmm_stack_top = -1;
uint32_t pmm_max_blocks = 0;

void pmm_free_block(void *ptr)
{
    uint32_t addr = (uint32_t)ptr;

    if (addr % BLOCK_SIZE != 0)
        return;

    pmm_stack_top++;
    pmm_stack[pmm_stack_top] = addr;
}

void *pmm_alloc_block()
{
    if (pmm_stack_top < 0)
    {
        return NULL;
    }
    uint32_t addr = pmm_stack[pmm_stack_top];
    pmm_stack_top--;

    return (void *)addr;
}

void pmm_init(struct multiboot_info *mbm)
{
    uint32_t k_start = (uint32_t)&_kernel_start;
    uint32_t k_end = (uint32_t)&_kernel_end;

    uint64_t total_mem_bytes = ((uint64_t)mbm->mem_lower + (uint64_t)mbm->mem_upper) * 1024;
    pmm_max_blocks = (uint32_t)(total_mem_bytes / BLOCK_SIZE);

    uint32_t stack_start = k_end;
    if (stack_start % BLOCK_SIZE != 0)
    {
        stack_start = (stack_start + BLOCK_SIZE) & ~(BLOCK_SIZE - 1);
    }

    pmm_stack = (uint32_t *)stack_start;
    uint32_t stack_end = stack_start + (pmm_max_blocks * sizeof(uint32_t));

    if (mbm->flags & (1 << 6))
    {
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbm->mmap_addr;
        uint32_t mmap_end = mbm->mmap_addr + mbm->mmap_length;

        print("PMM: Versuche Memory Map zu lesen...\n", 'R');

        while ((uint32_t)mmap < mmap_end)
        {
            if (mmap->type == 1)
            {
                uint32_t start_addr = (uint32_t)mmap->addr_low;
                uint32_t end_addr = start_addr + (uint32_t)mmap->len_low;

                if (start_addr % BLOCK_SIZE != 0)
                {
                    start_addr = (start_addr + BLOCK_SIZE) & ~(BLOCK_SIZE - 1);
                }

                for (uint32_t addr = start_addr; addr < end_addr; addr += BLOCK_SIZE)
                {
                    if (addr >= k_start && addr < k_end)
                        continue;
                    if (addr >= stack_start && addr < stack_end)
                        continue;
                    if (addr < 0x100000)
                        continue;

                    pmm_free_block((void *)addr);
                }
            }
            mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    }

    if (pmm_stack_top < 0)
    {
        print("PMM: Map war leer. Aktiviere linearen Fallback (mem_upper)...\n", 'R');

        uint32_t fallback_end_addr = 0x100000 + (mbm->mem_upper * 1024);

        for (uint32_t addr = 0x100000; addr < fallback_end_addr; addr += BLOCK_SIZE)
        {

            if (addr >= k_start && addr < k_end)
                continue;

            if (addr >= stack_start && addr < stack_end)
                continue;

            pmm_free_block((void *)addr);
        }
    }

    if (pmm_stack_top < 0)
    {
        print("PMM PANIC: No RAM Detected\n", 'R');
        while (1)
            ;
    }

    print("PMM: PHYSICAL MEMORY MANAGER: OKAY\n", 'R');
}