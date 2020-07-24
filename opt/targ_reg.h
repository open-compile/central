//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_TARG_REG_H
#define OCC_TARG_REG_H
#include "basic.h"
#include "targ_basic.h"

#define ISA_REGISTER_FIRST (0)
#define ISA_REGISTER_MAX 15

#define DEFAULT_RCLASS_SIZE(rc)	\
	(REGISTER_bit_size(rc, REGISTER_CLASS_last_register(rc))/8)

typedef UINT8 mISA_REGISTER_CLASS;

#define FOR_ALL_ISA_REGISTER_CLASS(cl) \
	for (cl = ISA_REGISTER_CLASS_MIN; \
	     cl <= ISA_REGISTER_CLASS_MAX; \
	     cl = (ISA_REGISTER_CLASS)(cl + 1))

#define FOR_ALL_ISA_REGISTER_CLASS_IN_REVERSE(cl) \
	for (cl = ISA_REGISTER_CLASS_MAX; \
	     cl >= ISA_REGISTER_CLASS_MIN; \
	     cl = (ISA_REGISTER_CLASS)(cl - 1))

typedef struct {
  UINT8       isa_mask;
  UINT8       bit_size;
  UINT16      min_regnum;
  UINT16      max_regnum;
  BOOL        can_store;
  BOOL        multiple_save;
  const char *name;
  const char *reg_name[ISA_REGISTER_MAX+1];
} ISA_REGISTER_CLASS_INFO;

typedef enum {
  ISA_REGISTER_SUBCLASS_UNDEFINED,
  ISA_REGISTER_SUBCLASS_rax,
  ISA_REGISTER_SUBCLASS_rdx,
  ISA_REGISTER_SUBCLASS_rbx,
  ISA_REGISTER_SUBCLASS_rcx,
  ISA_REGISTER_SUBCLASS_rbp,
  ISA_REGISTER_SUBISA_REGISTER_SUBCLASS_INFOCLASS_rsp,
  ISA_REGISTER_SUBCLASS_rdi,
  ISA_REGISTER_SUBCLASS_r11,
  ISA_REGISTER_SUBCLASS_xmm0,
  ISA_REGISTER_SUBCLASS_m32_8bit_regs,
  ISA_REGISTER_SUBCLASS_MIN = ISA_REGISTER_SUBCLASS_rax,
  ISA_REGISTER_SUBCLASS_MAX = ISA_REGISTER_SUBCLASS_m32_8bit_regs,
  ISA_REGISTER_SUBCLASS_COUNT = ISA_REGISTER_SUBCLASS_MAX - ISA_REGISTER_SUBCLASS_MIN + 1
} ISA_REGISTER_SUBCLASS;

typedef UINT8 mISA_REGISTER_SUBCLASS;

#define FOR_ALL_ISA_REGISTER_SUBCLASS(sc) \
	for (sc = ISA_REGISTER_SUBCLASS_MIN; \
	     sc <= ISA_REGISTER_SUBCLASS_MAX; \
	     sc = (ISA_REGISTER_SUBCLASS)(sc + 1))

typedef struct {
  const char *name;
  mISA_REGISTER_CLASS rclass;
  UINT16 count;
  UINT16 members[ISA_REGISTER_MAX+1];
  const char *reg_name[ISA_REGISTER_MAX+1];
} ISA_REGISTER_SUBCLASS_INFO;

inline const ISA_REGISTER_CLASS_INFO *ISA_REGISTER_CLASS_Info(
  ISA_REGISTER_CLASS rc
)
{
  extern const ISA_REGISTER_CLASS_INFO ISA_REGISTER_CLASS_info[];
  extern UINT8 ISA_REGISTER_CLASS_info_index[];
  INT index = ISA_REGISTER_CLASS_info_index[(INT)rc];
  return &ISA_REGISTER_CLASS_info[index];
}

inline INT ISA_REGISTER_CLASS_INFO_First_Reg(
  const ISA_REGISTER_CLASS_INFO *info
)
{
  return info->min_regnum;
}

inline INT ISA_REGISTER_CLASS_INFO_Last_Reg(
  const ISA_REGISTER_CLASS_INFO *info
)
{
  return info->max_regnum;
}

inline INT ISA_REGISTER_CLASS_INFO_Bit_Size(
  const ISA_REGISTER_CLASS_INFO *info
)
{
  return info->bit_size;
}

inline BOOL ISA_REGISTER_CLASS_INFO_Can_Store(
  const ISA_REGISTER_CLASS_INFO *info
)
{
  return info->can_store;
}

inline BOOL ISA_REGISTER_CLASS_INFO_Multiple_Save(
  const ISA_REGISTER_CLASS_INFO *info
)
{
  return info->multiple_save;
}

inline const char *ISA_REGISTER_CLASS_INFO_Name(
  const ISA_REGISTER_CLASS_INFO *info
)
{
  return info->name;
}

inline const char *ISA_REGISTER_CLASS_INFO_Reg_Name(
  const ISA_REGISTER_CLASS_INFO *info,
  INT reg_index
)
{
  return info->reg_name[reg_index];
}

inline const ISA_REGISTER_SUBCLASS_INFO *ISA_REGISTER_SUBCLASS_Info(
  ISA_REGISTER_SUBCLASS sc
)
{
  extern const ISA_REGISTER_SUBCLASS_INFO ISA_REGISTER_SUBCLASS_info[];
  return &ISA_REGISTER_SUBCLASS_info[sc];
}

inline const char *ISA_REGISTER_SUBCLASS_INFO_Name(
  const ISA_REGISTER_SUBCLASS_INFO *info
)
{
  return info->name;
}

inline ISA_REGISTER_CLASS ISA_REGISTER_SUBCLASS_INFO_Class(
  const ISA_REGISTER_SUBCLASS_INFO *info
)
{
  return (ISA_REGISTER_CLASS)info->rclass;
}

inline INT ISA_REGISTER_SUBCLASS_INFO_Count(
  const ISA_REGISTER_SUBCLASS_INFO *info
)
{
  return info->count;
}

inline UINT ISA_REGISTER_SUBCLASS_INFO_Member(
  const ISA_REGISTER_SUBCLASS_INFO *info,
  INT n
)
{
  return info->members[n];
}

inline const char *ISA_REGISTER_SUBCLASS_INFO_Reg_Name(
  const ISA_REGISTER_SUBCLASS_INFO *info,
  INT n
)
{
  return info->reg_name[n];
}

extern void ISA_REGISTER_Initialize(void);


typedef enum {
  ISA_SUBSET_x86_64,
  ISA_SUBSET_UNDEFINED,
  ISA_SUBSET_MIN=ISA_SUBSET_x86_64,
  ISA_SUBSET_MAX=ISA_SUBSET_x86_64
} ISA_SUBSET;
extern ISA_SUBSET ISA_SUBSET_Value;

extern const char* ISA_SUBSET_Name( ISA_SUBSET subset );
extern INT32 ISA_SUBSET_Member( ISA_SUBSET subset,
                              UINT8 opcode );

#endif //OCC_TARG_REG_H
