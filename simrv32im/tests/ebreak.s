.data

pass_string:
        .ascii "PASS!\n\0";

.text

_start: ebreak
pass:   la s0,pass_string
1:      lb a0,0(s0)
        beqz a0,2f
        li a7,11
        ecall
        addi s0,s0,1
        j 1b
2:      li a7,93
        li a0,0
        ecall
