# 1 "ori.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "ori.S"
# See LICENSE for license details.

#*****************************************************************************
# ori.S
#-----------------------------------------------------------------------------

# Test ori instruction.


# 1 "riscv_test.h" 1
# 11 "ori.S" 2
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
# 12 "ori.S" 2


.text; .global _start; .global ori_ret; _start: lui s0,%hi(test_name); addi s0,s0,%lo(test_name); name_print_loop: lb a0,0(s0); beqz a0,prname_done; li a7,11; ecall; addi s0,s0,1; j name_print_loop; test_name: .ascii "ori"; .byte '.','.',0x00; .balign 4, 0; prname_done:

  #-------------------------------------------------------------
  # Logical tests
  #-------------------------------------------------------------

  test_2: li x1, 0xff00ff00; ori x3, x1, -0xf1;; li x29, 0xffffff0f; li x28, 2; bne x3, x29, fail;;
  test_3: li x1, 0x0ff00ff0; ori x3, x1, 0x0f0;; li x29, 0x0ff00ff0; li x28, 3; bne x3, x29, fail;;
  test_4: li x1, 0x00ff00ff; ori x3, x1, 0x70f;; li x29, 0x00ff07ff; li x28, 4; bne x3, x29, fail;;
  test_5: li x1, 0xf00ff00f; ori x3, x1, 0x0f0;; li x29, 0xf00ff0ff; li x28, 5; bne x3, x29, fail;;

  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_6: li x1, 0xff00ff00; ori x1, x1, 0x0f0;; li x29, 0xff00fff0; li x28, 6; bne x1, x29, fail;;

  #-------------------------------------------------------------
  # Bypassing tests
  #-------------------------------------------------------------

  test_7: li x4, 0; 1: li x1, 0x0ff00ff0; ori x3, x1, 0x0f0; addi x6, x3, 0; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b; li x29, 0x0ff00ff0; li x28, 7; bne x6, x29, fail;;
  test_8: li x4, 0; 1: li x1, 0x00ff00ff; ori x3, x1, 0x70f; nop; addi x6, x3, 0; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b; li x29, 0x00ff07ff; li x28, 8; bne x6, x29, fail;;
  test_9: li x4, 0; 1: li x1, 0xf00ff00f; ori x3, x1, 0x0f0; nop; nop; addi x6, x3, 0; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b; li x29, 0xf00ff0ff; li x28, 9; bne x6, x29, fail;;

  test_10: li x4, 0; 1: li x1, 0x0ff00ff0; ori x3, x1, 0x0f0; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b; li x29, 0x0ff00ff0; li x28, 10; bne x3, x29, fail;;
  test_11: li x4, 0; 1: li x1, 0x00ff00ff; nop; ori x3, x1, -0xf1; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b; li x29, 0xffffffff; li x28, 11; bne x3, x29, fail;;
  test_12: li x4, 0; 1: li x1, 0xf00ff00f; nop; nop; ori x3, x1, 0x0f0; addi x4, x4, 1; li x5, 2; bne x4, x5, 1b; li x29, 0xf00ff0ff; li x28, 12; bne x3, x29, fail;;

  test_13: ori x1, x0, 0x0f0;; li x29, 0x0f0; li x28, 13; bne x1, x29, fail;;
  test_14: li x1, 0x00ff00ff; ori x0, x1, 0x70f;; li x29, 0; li x28, 14; bne x0, x29, fail;;

  bne x0, x28, pass; fail: j fail_print; fail_string: .ascii "FAIL\n\0"; .balign 4, 0; fail_print: la s0,fail_string; fail_print_loop: lb a0,0(s0); beqz a0,fail_print_exit; li a7,11; ecall; addi s0,s0,1; j fail_print_loop; fail_print_exit: li a7,93; li a0,1; ecall;; pass: j pass_print; pass_string: .ascii "PASS!\n\0"; .balign 4, 0; pass_print: la s0,pass_string; pass_print_loop: lb a0,0(s0); beqz a0,pass_print_exit; li a7,11; ecall; addi s0,s0,1; j pass_print_loop; pass_print_exit: jal zero,ori_ret;

ori_ret: li a7,93; li a0,0; ecall;

  .data
.balign 4;

 


