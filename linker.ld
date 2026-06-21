ENTRY(start)

SECTIONS
{
    /* Kernel startet bei 1MB */
    . = 1M;

    _kernel_start = .;
    kernel_start = .;

    /* Jede Sektion wird nun sauber auf 4KB (BLOCK(4K)) ausgerichtet */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON) /* WICHTIG: Füge COMMON hinzu, sonst fliegen manche Variablen raus! */
        *(.bss)
    }

    /* FIX: Erzwinge eine saubere Page-Grenze für das Kernel-Ende */
    . = ALIGN(4096);
    _kernel_end = .;
    kernel_end = .;
}