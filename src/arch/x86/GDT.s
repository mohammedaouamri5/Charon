/*
 * void gdt_flush(struct gdt_pointer *gdtr)
 *
 * Load the GDT and reload the segment registers. This file is assembled by
 * GNU as, so use its Intel-syntax mode instead of NASM directives/comments.
 */
.intel_syntax noprefix

.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    mov eax, [esp + 4]      /* pointer to gdtr */
    lgdt [eax]

    mov ax, 0x10            /* kernel data selector */
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.done          /* far jump reloads CS */
.done:
    ret

.size gdt_flush, . - gdt_flush
