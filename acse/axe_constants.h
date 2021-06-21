/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_constants.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#ifndef _AXE_CONSTANTS_H
#define _AXE_CONSTANTS_H

/* registers */
#define REG_INVALID -1
#define REG_0 0

/* MACE opcodes */
#define ADD 0
#define SUB 1
#define ANDL 2
#define ORL 3
#define EORL 4
#define ANDB 5
#define ORB 6
#define EORB 7
#define MUL 8
#define DIV 9
#define SHL 10
#define SHR 11
#define ROTL 12
#define ROTR 13
#define NEG 14
#define SPCL 15
#define ADDI 16
#define SUBI 17
#define ANDLI 18
#define ORLI 19
#define EORLI 20
#define ANDBI 21
#define ORBI 22
#define EORBI 23
#define MULI 24
#define DIVI 25
#define SHLI 26
#define SHRI 27
#define ROTLI 28
#define ROTRI 29
#define NOTL 30
#define NOTB 31
#define NOP 32
#define MOVA 33
#define JSR 34
#define RET 35
#define HALT 36
#define SEQ 37
#define SGE 38
#define SGT 39
#define SLE 40
#define SLT 41
#define SNE 42
#define BT 43
#define BF 44
#define BHI 45
#define BLS 46
#define BCC 47
#define BCS 48
#define BNE 49
#define BEQ 50
#define BVC 51
#define BVS 52
#define BPL 53
#define BMI 54
#define BGE 55
#define BLT 56
#define BGT 57
#define BLE 58
#define LOAD 59
#define STORE 60
#define AXE_READ 61
#define AXE_WRITE 62
#define INVALID_OPCODE -1

#endif
