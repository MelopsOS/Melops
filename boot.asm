; --- MULTIBOOT HEADER ---
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

; --- STACK SETUP ---
section .bss
align 16
stack_bottom:
    resb 16384 
stack_top:

; --- CODE SECTION ---
section .text
global _start
extern kernel_main

_start:
  
    mov esp, stack_top

    
    call kernel_main

  
.hang:
    cli
    hlt
    jmp .hang


; --- GDT RELOADER ---
global reload_gdt
extern gdtr

reload_gdt:
    lgdt [gdtr]          
    mov ax, 0x10         
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush      
.flush:
    ret



global keyboard_isr
extern isr_handler

keyboard_isr:
    pusha                
    cld                  
    call isr_handler     
    popa                
    iretd                




extern exception_handler

%macro ISR_NOERRCODE 1
global isr_stub_%1
isr_stub_%1:
    push byte 0          
    push byte %1         
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr_stub_%1
isr_stub_%1:
    push byte %1         
    jmp isr_common_stub
%endmacro


ISR_NOERRCODE 0  ; Divide-by-zero
ISR_NOERRCODE 1  ; Debug
ISR_NOERRCODE 2  ; Non-maskable Interrupt
ISR_NOERRCODE 3  ; Breakpoint
ISR_NOERRCODE 4  ; Overflow
ISR_NOERRCODE 5  ; Bound Range Exceeded
ISR_NOERRCODE 6  ; Invalid Opcode
ISR_NOERRCODE 7  ; Device Not Available
ISR_ERRCODE   8  ; Double Fault 
ISR_NOERRCODE 9  ; Coprocessor Segment Overrun
ISR_ERRCODE   10 ; Invalid TSS 
ISR_ERRCODE   11 ; Segment Not Present 
ISR_ERRCODE   12 ; Stack-Segment Fault 
ISR_ERRCODE   13 ; General Protection Fault 
ISR_ERRCODE   14 ; Page Fault 


isr_common_stub:
    pusha                
    mov ax, ds           
    push eax

    mov ax, 0x10         
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp             
    call exception_handler
    add esp, 4           

    pop eax              
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa                 
    add esp, 8           
    iretd                
