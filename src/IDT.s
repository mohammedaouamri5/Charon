# IDT.s
.intel_syntax noprefix
.altmacro

.macro ISR_STUB n
isr\n:
  .if (\n == 8) || (\n == 10) || (\n == 11) || (\n == 12) || (\n == 13) || (\n == 14) || (\n == 17) || (\n == 21) || (\n == 30)
    push \n
  .else
    push 0
    push \n
  .endif
    jmp isrCommon

  .section .rodata
    .long isr\n             /* table entry, appended in order */
  .text
.endm

.section .rodata
.global isrStubTable
isrStubTable:               /* entries get appended right after this label */

.text
.set i, 0
.rept 256
    ISR_STUB %i
    .set i, i + 1
.endr

# isrCommon: pusha -> pass saved state -> call isrHandler in C++ -> popa -> drop vector + error code -> iret
# IF is already clear (0x8E interrupt gate), so no cli is needed here
.global isrCommon
.type isrCommon, @function

isrCommon:
    pusha
    push esp          /* argument: pointer to the saved state */
    call isrHandler
    add esp, 4
    popa
    add esp, 8        /* drop vector + error code */
    iret

.size isrCommon, . - isrCommon

# no segment reload needed, unlike lgdt
.global idtFlush
.type idtFlush, @function

idtFlush:
    mov eax, [esp + 4]
    lidt [eax]
    ret

.size idtFlush, . - idtFlush
