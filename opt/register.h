//
// Created by xc5 on 2020/7/16.
//

#ifndef OCC_REGISTER_H
#define OCC_REGISTER_H

#include "basic.h"

typedef UINT16   REGISTER;             // 16-bit registeer number
typedef UINT16   ISA_REGISTER_CLASS;   // 16-bit registeer number
/* define 32-bit structure to hold both the class and register number,
   and a union with an UINT32 so that we can efficiently compare
   register class-number pairs */
typedef union class_reg_pair {
  UINT32	class_n_reg;
  struct class_reg_struct {
    REGISTER 	        reg;
    ISA_REGISTER_CLASS rclass;
  } class_reg;
} CLASS_REG_PAIR;

#endif //OCC_REGISTER_H
