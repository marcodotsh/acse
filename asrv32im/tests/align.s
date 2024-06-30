_start:
addi t0, zero, 100
.align 4
addi t0, zero, 101
.balign 8
addi t0, zero, 102
.balign 1
addi t0, zero, 102

.data
.byte 0x10
.align 4, 0xFE
.byte 0x11
.balign 8, 0xFD
.byte 0x12
