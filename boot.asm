;--- MULTIBOOT HEADER ---
MAGIC    equ 0x1BADB002         
; FLAGS: Bit 0 (Align), Bit 1 (MemInfo), und Bit 2 (Grafikmodus anfordern!)
FLAGS    equ 1 | 2 | 4          
CHECKSUM equ -(MAGIC + FLAGS)   

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    
    ; Diese Felder müssen ausgefüllt werden, wenn Bit 2 in FLAGS gesetzt ist:
    dd 0    ; Mode-Typ (0 = Linearer Framebuffer/Grafik, 1 = Text)
    dd 1024 ; Bevorzugte Breite (Width)
    dd 768  ; Bevorzugte Höhe (Height)
    dd 32   ; Bevorzugte Bits per Pixel (BPP)

; --- KERNEL STACK ---
section .bss
align 16
stack_bottom:
    resb 16384                  
stack_top:

; --- KERNEL ENTRY POINT ---
section .text
global _start
extern kernel_main              

_start:
    mov esp, stack_top

    ; WICHTIG: GRUB übergibt die Argumente in Registern:
    ; EAX = Magic Number (0x2BADB002 bei Multiboot 1)
    ; EBX = Adresse der Multiboot-Informationsstruktur
    ; Wir pushen sie auf den Stack, damit kernel_main(magic, addr) sie lesen kann.
    push ebx
    push eax

    call kernel_main

.halt:
    cli                         
    hlt                         
    jmp .halt                   
