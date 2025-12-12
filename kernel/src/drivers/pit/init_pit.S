.intel_syntax noprefix
.global init_pit

.section .data
PIT_reload_value:   .long 0
IRQ0_frequency:     .long 0

.section .text

init_pit:
    push rax
    push rbx
    push rcx
    push rdx

    # Bounds check
    mov eax, 0x10000       # slowest reload
    cmp ebx, 18
    jbe gotReloadValue
    mov eax, 1             # fastest reload
    cmp ebx, 1193181
    jae gotReloadValue

    mov eax, 3579545
    xor edx, edx
    div ebx
    cmp edx, 3579545 / 2
    jb l1
    inc eax
l1:
    mov [PIT_reload_value], eax

    mov eax, 3579545
    xor edx, edx
    mov ebx, [PIT_reload_value]
    div ebx
    cmp edx, 3579545 / 2
    jb l3
    inc eax
l3:
    mov [IRQ0_frequency], eax

gotReloadValue:
    # Program PIT channel 0
    pushfq
    cli
    mov al, 0b00110100      # channel 0, lo/hi byte, rate generator
    out 0x43, al

    mov eax, [PIT_reload_value]
    out 0x40, al            # low byte
    mov al, ah
    out 0x40, al            # high byte
    popfq

    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret
