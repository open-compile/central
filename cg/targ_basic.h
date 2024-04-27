//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_TARG_BASIC_H
#define OCC_TARG_BASIC_H

typedef signed char mINT8;
typedef signed short mINT16;
typedef signed int mINT32;
typedef signed long long mINT64;
typedef unsigned int UINT;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned char mUINT8;
typedef unsigned short mUINT16;
typedef unsigned int mUINT32;
typedef unsigned long long mUINT64;

typedef enum {
  ISA_REGISTER_CLASS_UNDEFINED,
  ISA_REGISTER_CLASS_integer,
  ISA_REGISTER_CLASS_float,
  ISA_REGISTER_CLASS_rflags,
  ISA_REGISTER_CLASS_rip,
  ISA_REGISTER_CLASS_x87,
  ISA_REGISTER_CLASS_x87_cw,
  ISA_REGISTER_CLASS_mmx,
  ISA_REGISTER_CLASS_mxcsr,
  ISA_REGISTER_CLASS_MIN = ISA_REGISTER_CLASS_integer,
  ISA_REGISTER_CLASS_MAX = ISA_REGISTER_CLASS_mxcsr,
  ISA_REGISTER_CLASS_COUNT = ISA_REGISTER_CLASS_MAX - ISA_REGISTER_CLASS_MIN + 1
} ISA_REGISTER_CLASS;

#endif //OCC_TARG_BASIC_H
