#include "stdmm.h"
//-------------------//
//  MELOPS-KERNEL-C  //
//-------------------//

#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "multiboot.h"
#include "TInterpreter.h"
//--- globales --
volatile int Line;
volatile int CommandLine = 0;
volatile int scancode_buffer = 0;

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void reload_gdt(void);
extern void keyboard_isr(void);

#define VGA 0xB8000
#define TextmodeWidth 80
#define TextmodeHeight 25

#define PIC1 0x20 // Master PIC
#define PIC2 0xA0 // Slave PIC
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20 // End-Of-Interrupt

#define PIC_READ_IRR 0x0a
#define PIC_READ_ISR 0x0b

#define KEYBOARD 0x60

//--- Interupt Command Words ---
#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define IDT_TA_INTERRUPT_GATE 0x8E

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW_SFNM 0x10

#define CASCADE_IQR 2

// --- GDT Macros ---
#define SEG_DESCTYPE(x) ((x) << 0x04)
#define SEG_PRES(x) ((x) << 0x07)
#define SEG_SAVL(x) ((x) << 0x0C)
#define SEG_LONG(x) ((x) << 0x0D)
#define SEG_SIZE(x) ((x) << 0x0E)
#define SEG_GRAN(x) ((x) << 0x0F)
#define SEG_PRIV(x) (((x) & 0x03) << 0x05)

#define SEG_DATA_RD 0x00
#define SEG_DATA_RDWR 0x02
#define SEG_CODE_EXRD 0x0A

#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                         SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
                         SEG_PRIV(0) | SEG_CODE_EXRD

#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                         SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
                         SEG_PRIV(0) | SEG_DATA_RDWR

#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                         SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
                         SEG_PRIV(3) | SEG_CODE_EXRD

#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                         SEG_LONG(0) | SEG_SIZE(1) | SEG_GRAN(1) | \
                         SEG_PRIV(3) | SEG_DATA_RDWR

#define GDT_OFFSET_KERNEL_CODE 0x08

//--- Inline Assembler ---
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile(" outb %b0, %w1" ::"a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

void PIC_sendEOI(uint8_t irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

// --- PIC Part ---
void PIC_remap(int offset1, int offset2)
{
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();
    outb(PIC1_DATA, 1 << CASCADE_IQR);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

// --- GDT Globals Storage ---
uint64_t gdt[5];
int gdt_index = 0;

struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr;

void create_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
{
    uint64_t descriptor;

    descriptor = limit & 0x000F0000;
    descriptor |= (flag << 8) & 0x00F0FF00;
    descriptor |= (base >> 16) & 0x000000FF;
    descriptor |= base & 0xFF000000;

    descriptor <<= 32;

    descriptor |= base << 16;
    descriptor |= limit & 0x0000FFFF;

    gdt[gdt_index++] = descriptor;
}

// --- Protected Mode Structures ---
typedef struct
{
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t isr_high;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

__attribute__((aligned(0x10))) static idt_entry_t idt[256];
static idtr_t idtr;

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags)
{
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs = GDT_OFFSET_KERNEL_CODE;
    descriptor->reserved = 0;
    descriptor->attributes = flags;
    descriptor->isr_high = ((uint32_t)isr >> 16) & 0xFFFF;
}

// --- VGA Textmode Driver ---

char colorScheme = 'w';

typedef struct
{
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void exception_handler(registers_t *regs)
{
    print(":/ CRITICAL CPU EXCEPTION DETECTED!\n", 'R');
    if (regs->int_no == 14)
    {
        print("Page Fault!\n", 'R');
    }
    while (1)
        ;
}

char kbd_scancode[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'u', '+', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'o', 'a', '>',
    0, '<', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ';', '.', '-', 0};
volatile uint8_t scancode;
void isr_handler()
{
    scancode = inb(KEYBOARD);

    if (!(scancode & 0x80))
    {
        if (scancode == 0x39)
        {
            cursor_pos++;
        }
        if (scancode == 0x0E)
        {
            if (cursor_pos > 0)
            {
                cursor_pos--;
                print(" ", 'w');
                cursor_pos = cursor_pos - 1;
                print(" ", 'w');
                cursor_pos--;
            }
        }
        if (kbd_scancode[scancode] == '\n')
        {
            scancode_buffer++;
            cursor_pos = ((cursor_pos / 80) + 1) * 80;

            if (Line < 24)
            {
                CommandLine = cursor_pos / 80 - 1;
                Line = Line + 1;
                char *LineString;
                IntParseString(Line, LineString);
                // print(LineVS,'R');
            }
            else
            {
                cursor_pos = 80;
                Line = 1;
                char *LineString;
                IntParseString(Line, LineString);
                // print(LineVS,'R');
            }
        }
        if (kbd_scancode[scancode] != 0 && scancode != 0x0E && scancode != 0x39 && kbd_scancode[scancode] != '\n')
        {
            char str[2] = {kbd_scancode[scancode], '\0'};
            print(str, 'W');
        }
    }
    PIC_sendEOI(1);
}
// --- Terminal ---

void Terminal()
{
    clearScreen();
    print("--- Melops/OS/ Terminal ---\n", 'Y');

    // Interpreter(0);

    while (1)
    {
        __asm__("hlt");

        if (scancode_buffer > 0)
        {
            Interpreter(CommandLine);

            scancode_buffer--;
        }
    }
}
// --- Kernel Entry ---
int kernel_main(uint32_t magic, struct multiboot_info *mbm)
{
    pmm_init(mbm);
    // GDT build
    create_descriptor(0, 0, 0);
    create_descriptor(0, 0xFFFFFFFF, (GDT_CODE_PL0));
    create_descriptor(0, 0xFFFFFFFF, (GDT_DATA_PL0));
    create_descriptor(0, 0xFFFFFFFF, (GDT_CODE_PL3));
    create_descriptor(0, 0xFFFFFFFF, (GDT_DATA_PL3));

    gdtr.limit = (sizeof(uint64_t) * 5) - 1;
    gdtr.base = (uint32_t)&gdt;

    reload_gdt();

    PIC_remap(0x20, 0x28);

    idtr.base = (uint32_t)&idt;
    idtr.limit = (sizeof(idt_entry_t) * 256) - 1;

    // CPU Exceptions
    idt_set_descriptor(0, (void *)isr_stub_0, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(1, (void *)isr_stub_1, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(2, (void *)isr_stub_2, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(3, (void *)isr_stub_3, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(4, (void *)isr_stub_4, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(5, (void *)isr_stub_5, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(6, (void *)isr_stub_6, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(7, (void *)isr_stub_7, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(8, (void *)isr_stub_8, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(9, (void *)isr_stub_9, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(10, (void *)isr_stub_10, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(11, (void *)isr_stub_11, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(12, (void *)isr_stub_12, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(13, (void *)isr_stub_13, IDT_TA_INTERRUPT_GATE);
    idt_set_descriptor(14, (void *)isr_stub_14, IDT_TA_INTERRUPT_GATE);

    idt_set_descriptor(33, (void *)keyboard_isr, IDT_TA_INTERRUPT_GATE);

    __asm__ volatile("lidt %0" : : "m"(idtr));

    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);

    __asm__ volatile("sti");

    clearScreen();
    print("/Melops V26.3/Startmenu/\n", 'B');
    print("PRESS /ENTER/ to acess the Terminal\n", 'R');

    while (1)
    {
        if (scancode_buffer == 1)
        {
            Line = 1;
            cursor_pos = 80;
            Terminal();
            scancode_buffer = 2;
        }
    }
}
