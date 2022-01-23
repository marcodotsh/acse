/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_array.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <assert.h>
#include "axe_array.h"
#include "axe_gencode.h"
#include "axe_utils.h"
#include "axe_errors.h"
#include "axe_target_info.h"
#include "axe_expressions.h"

void genStoreArrayElement(t_program_infos *program, char *ID
            , t_axe_expression index, t_axe_expression data)
{
   int address;
   
   address = genLoadArrayAddress(program, ID, index);

   if (data.type == REGISTER)
   {
      /* load the value indirectly into `mova_register' */
      genSWInstruction(program, data.registerId, 0, address);
   }
   else
   {
      int imm_register;

      imm_register = genLoadImmediate(program, data.immediate);

      /* load the value indirectly into `load_register' */
      genSWInstruction(program, imm_register, 0, address);
   }
}

int genLoadArrayElement(t_program_infos *program
               , char *ID, t_axe_expression index)
{
   int load_register;
   int address;

   /* retrieve the address of the array slot */
   address = genLoadArrayAddress(program, ID, index);

   /* get a new register */
   load_register = getNewRegister(program);

   /* load the value into `load_register' */
   genLWInstruction(program, load_register, 0, address);

   /* return the register ID that holds the required data */
   return load_register;
}

int genLoadArrayAddress(t_program_infos *program
            , char *ID, t_axe_expression index)
{
   int mova_register;
   t_axe_label *label;

   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (ID == NULL)
      notifyError(AXE_VARIABLE_ID_UNSPECIFIED);
   
   /* retrieve the label associated with the given
   * identifier */
   label = getLabelFromVariableID(program, ID);
                     
   /* test if an error occurred */
   if (label == NULL)
      return REG_INVALID;

   /* get a new register */
   mova_register = getNewRegister(program);

   /* generate the MOVA instruction */
   genLAInstruction(program, mova_register, label);

   /* We are making the following assumption:
    * the type can only be an INTEGER_TYPE */
   int sizeofElem = 4 / TARGET_PTR_GRANULARITY;

   if (index.type == IMMEDIATE)
   {
      if (index.immediate != 0)
      {
         genADDIInstruction(program, mova_register
                     , mova_register, index.immediate * sizeofElem);
      }
   }
   else
   {
      assert(index.type == REGISTER);

      int idxReg = index.registerId;
      if (sizeofElem != 1) {
         idxReg = getNewRegister(program);
         genMULIInstruction(program, idxReg, index.registerId, sizeofElem);
      }
      
      genADDInstruction(program, mova_register, mova_register, idxReg);
   }

   /* return the identifier of the register that contains
    * the value of the array slot */
   return mova_register;
}
