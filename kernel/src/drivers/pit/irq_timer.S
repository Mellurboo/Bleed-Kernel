.section .data
.extern timer_ticks

.intel_syntax noprefix
.global TimerIRQ
.section .text

TimerIRQ:
    push rax

    mov rax, [timer_ticks]
    test rax, rax
    jz TimerDone

    dec rax
    mov [timer_ticks], rax

TimerDone:
    pop rax
    iretq
