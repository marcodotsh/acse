        .data

test1:  .word 1, 2, 3
test2:  .word 4
test3:  .space 20

        .text

_start: bgt x1, x0, next
        ble x1, x0, next
        bgtu x1, x0, next
        bleu x1, x0, next

        j next

        lb x1, test1
        lh x1, test1
        lw x1, test1
        lbu x1, test1
        lhu x1, test1
        sb x1, test1, x2
        sh x1, test1, x2
        sw x1, test1, x2

        li x1, 0
        li x2, -1
        li x1, 1
        li x1, 0x7FF
        li x1, 0x800
        li x1, 0x801
        li x1, 0x7FFFFFFF
        li x1, -0x800
        li x1, -0x801
        li x1, -0x7FFFFFFF
        li x1, -0x80000000

        la x1, next
        la x2, _start

        add   x1, x2, x3
        sub   x1, x2, x3
        xor   x31, x30, x29
        or    x1, x2, x3
        and   x1, x2, x3
        sll   x1, x2, x3
        srl   x1, x2, x3
        sra   x1, x2, x3
        slt   x1, x2, x3
        sltu  x1, x2, x3

        mul   x1, x2, x3
        mulh  x1, x2, x3
        mulhsu x1, x2, x3
        mulhu x1, x2, x3
        div   x1, x2, x3
        divu  x1, x2, x3
        rem   x1, x2, x3
        remu  x1, x2, x3

        addi  x31, x31, 0
        xori  x31, x31, -0x800
        ori   x31, x31, 0x7ff
        andi  x0, x1, 444
        slli  x0, x1, 6
        srli  x0, x1, 31
        srai  x0, x1, 6
        slti  x1, x2, 10
        sltiu x1, x2, 20

        lui x1, %hi(test1)
        addi x1, x1, %lo(test1)
otherinst:
        auipc x1, %pcrel_hi(_start)
        addi x1, x1, %pcrel_lo(otherinst) /* BLEEEEARGH */

        jal x0, next
        jalr x1, -5(x2)
next:
        lb x1, 5(x2)
        lw x1, 5(x2)
        lh x1, 5(x2)
        lbu x1, 5(x2)
        lhu x1, 5(x2)

        sb x1, 5(x2)
        sw x1, 5(x2)
        sh x1, 5(x2)

        beq x1, x2, next
        bne x1, x2, next
        blt x1, x2, next
        bge x1, x2, next
        bltu x1, x2, next
        bgeu x31, x30, next

        nop
        ecall
        ebreak

