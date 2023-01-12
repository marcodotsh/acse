# 1 "sh.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "sh.S"
# See LICENSE for license details.

#*****************************************************************************
# sh.S
#-----------------------------------------------------------------------------

# Test sh instruction.


# 1 "riscv_test.h" 1
# 11 "sh.S" 2
# 1 "test_macros.h" 1






#-----------------------------------------------------------------------
# Helper macros
#-----------------------------------------------------------------------
# 18 "test_macros.h"
# We use a macro hack to simpify code generation for various numbers
# of bubble cycles.
# 34 "test_macros.h"
#-----------------------------------------------------------------------
# RV64UI MACROS
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Tests for instructions with immediate operand
#-----------------------------------------------------------------------
# 90 "test_macros.h"
#-----------------------------------------------------------------------
# Tests for vector config instructions
#-----------------------------------------------------------------------
# 118 "test_macros.h"
#-----------------------------------------------------------------------
# Tests for an instruction with register operands
#-----------------------------------------------------------------------
# 146 "test_macros.h"
#-----------------------------------------------------------------------
# Tests for an instruction with register-register operands
#-----------------------------------------------------------------------
# 240 "test_macros.h"
#-----------------------------------------------------------------------
# Test memory instructions
#-----------------------------------------------------------------------
# 317 "test_macros.h"
#-----------------------------------------------------------------------
# Test branch instructions
#-----------------------------------------------------------------------
# 402 "test_macros.h"
#-----------------------------------------------------------------------
# Test jump instructions
#-----------------------------------------------------------------------
# 431 "test_macros.h"
#-----------------------------------------------------------------------
# RV64UF MACROS
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Tests floating-point instructions
#-----------------------------------------------------------------------
# 567 "test_macros.h"
#-----------------------------------------------------------------------
# Pass and fail code (assumes test num is in x28)
#-----------------------------------------------------------------------
# 579 "test_macros.h"
#-----------------------------------------------------------------------
# Test data section
#-----------------------------------------------------------------------
# 12 "sh.S" 2


.text; .global _start; .global sh_ret; _start: lui s0,%hi(test_name); addi s0,s0,%lo(test_name); name_print_loop: lb a0,0(s0); beqz a0,prname_done; li a7,11; ecall; addi s0,s0,1; j name_print_loop; test_name: .ascii "sh"; .byte '.','.',0x00; .balign 4, 0; prname_done:

  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

  test_2: la x1, tdat; li x2, 0x000000aa; sh x2, 0(x1); lh x3, 0(x1);; li x29, 0x000000aa; li x28, 2; bne x3, x29, fail;;
  test_3: la x1, tdat; li x2, 0xffffaa00; sh x2, 2(x1); lh x3, 2(x1);; li x29, 0xffffaa00; li x28, 3; bne x3, x29, fail;;
  test_4: la x1, tdat; li x2, 0xbeef0aa0; sh x2, 4(x1); lw x3, 4(x1);; li x29, 0xbeef0aa0; li x28, 4; bne x3, x29, fail;;
  test_5: la x1, tdat; li x2, 0xffffa00a; sh x2, 6(x1); lh x3, 6(x1);; li x29, 0xffffa00a; li x28, 5; bne x3, x29, fail;;

  # Test with negative offset

  test_6: la x1, tdat8; li x2, 0x000000aa; sh x2, -6(x1); lh x3, -6(x1);; li x29, 0x000000aa; li x28, 6; bne x3, x29, fail;;
  test_7: la x1, tdat8; li x2, 0xffffaa00; sh x2, -4(x1); lh x3, -4(x1);; li x29, 0xffffaa00; li x28, 7; bne x3, x29, fail;;
  test_8: la x1, tdat8; li x2, 0x00000aa0; sh x2, -2(x1); lh x3, -2(x1);; li x29, 0x00000aa0; li x28, 8; bne x3, x29, fail;;
  test_9: la x1, tdat8; li x2, 0xffffa00a; sh x2, 0(x1); lh x3, 0(x1);; li x29, 0xffffa00a; li x28, 9; bne x3, x29, fail;;

  # Test with a negative base

  test_10: la x1, tdat9; li x2, 0x12345678; addi x4, x1, -32; sh x2, 32(x4); lh x3, 0(x1);; li x29, 0x5678; li x28, 10; bne x3, x29, fail;







  # Test with unaligned base

  test_11: la x1, tdat9; li x2, 0x00003098; addi x1, x1, -5; sh x2, 7(x1); la x4, tdat10; lh x3, 0(x4);; li x29, 0x3098; li x28, 11; bne x3, x29, fail;
# 53 "sh.S"
  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_12: li x28, 12; li x4, 0; 1: li x1, 0xffffccdd; la x2, tdat; sh x1, 0(x2); lh x3, 0(x2); li x29, 0xffffccdd; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_13: li x28, 13; li x4, 0; 1: li x1, 0xffffbccd; la x2, tdat; nop; sh x1, 2(x2); lh x3, 2(x2); li x29, 0xffffbccd; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_14: li x28, 14; li x4, 0; 1: li x1, 0xffffbbcc; la x2, tdat; nop; nop; sh x1, 4(x2); lh x3, 4(x2); li x29, 0xffffbbcc; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_15: li x28, 15; li x4, 0; 1: li x1, 0xffffabbc; nop; la x2, tdat; sh x1, 6(x2); lh x3, 6(x2); li x29, 0xffffabbc; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_16: li x28, 16; li x4, 0; 1: li x1, 0xffffaabb; nop; la x2, tdat; nop; sh x1, 8(x2); lh x3, 8(x2); li x29, 0xffffaabb; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_17: li x28, 17; li x4, 0; 1: li x1, 0xffffdaab; nop; nop; la x2, tdat; sh x1, 10(x2); lh x3, 10(x2); li x29, 0xffffdaab; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;

  test_18: li x28, 18; li x4, 0; 1: la x2, tdat; li x1, 0x2233; sh x1, 0(x2); lh x3, 0(x2); li x29, 0x2233; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_19: li x28, 19; li x4, 0; 1: la x2, tdat; li x1, 0x1223; nop; sh x1, 2(x2); lh x3, 2(x2); li x29, 0x1223; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_20: li x28, 20; li x4, 0; 1: la x2, tdat; li x1, 0x1122; nop; nop; sh x1, 4(x2); lh x3, 4(x2); li x29, 0x1122; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_21: li x28, 21; li x4, 0; 1: la x2, tdat; nop; li x1, 0x0112; sh x1, 6(x2); lh x3, 6(x2); li x29, 0x0112; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_22: li x28, 22; li x4, 0; 1: la x2, tdat; nop; li x1, 0x0011; nop; sh x1, 8(x2); lh x3, 8(x2); li x29, 0x0011; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;
  test_23: li x28, 23; li x4, 0; 1: la x2, tdat; nop; nop; li x1, 0x3001; sh x1, 10(x2); lh x3, 10(x2); li x29, 0x3001; bne x3, x29, fail; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b;

  li a0, 0xbeef
  la a1, tdat
  sh a0, 6(a1)

  bne x0, x28, pass; fail: j fail_print; fail_string: .ascii "FAIL\n\0"; .balign 4, 0; fail_print: la s0,fail_string; fail_print_loop: lb a0,0(s0); beqz a0,fail_print_exit; li a7,11; ecall; addi s0,s0,1; j fail_print_loop; fail_print_exit: li a7,93; li a0,1; ecall;; pass: j pass_print; pass_string: .ascii "PASS!\n\0"; .balign 4, 0; pass_print: la s0,pass_string; pass_print_loop: lb a0,0(s0); beqz a0,pass_print_exit; li a7,11; ecall; addi s0,s0,1; j pass_print_loop; pass_print_exit: jal zero,sh_ret;

sh_ret: li a7,93; li a0,0; ecall;

  .data
.balign 4;

 

tdat:
tdat1: .half 0xbeef
tdat2: .half 0xbeef
tdat3: .half 0xbeef
tdat4: .half 0xbeef
tdat5: .half 0xbeef
tdat6: .half 0xbeef
tdat7: .half 0xbeef
tdat8: .half 0xbeef
tdat9: .half 0xbeef
tdat10: .half 0xbeef


