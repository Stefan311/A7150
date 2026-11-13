
.globl _inb, _outb

_inb:
    mov     bx, sp
    mov     dx, #2(bx)
    in
    subb    ah, ah
    ret

_outb:
    mov     bx, sp
    mov     dx, #2(bx)
    mov     ax, #4(bx)
    out
    ret
