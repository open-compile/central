//
// Created by xc5 on 2020/7/15.
//

#ifndef OCC_DATA_LAYOUT_H
#define OCC_DATA_LAYOUT_H

#include "symtab.h"

/* direction for the stack frame to grow */
typedef enum { STACK_INCREMENT, STACK_DECREMENT } STACK_DIR;

/* Choice of stack model to use: */
typedef enum {
  SMODEL_UNDEF = 0,	/* Not yet defined */
  SMODEL_SMALL = 1,	/* Small frame, no frame pointer */
  SMODEL_LARGE = 2,	/* Large frame, use frame pointer */
  SMODEL_DYNAMIC = 3	/* Dynamic allocation, use frame pointer */
} STACK_MODEL;

class DATA_LAYOUT {
private:
  ST_IDX  SP_sym                  = 0;
  ST_IDX  FP_sym                  = 0;
  ST_IDX  Local_spill_sym         = 0;
  INT32   PU_real_size            = 0;
  STACK_MODEL Current_stack_model = SMODEL_UNDEF;
public:
  BOOL        ST_on_stack(ST_IDX sym);
  BOOL        ST_pu_defined(ST_IDX sym);
  ST_IDX      Get_st_ref_base (ST_IDX sym);
  UINT32      Stack_alignment();
  UINT32      Get_pu_arg_area_size(PU_IDX pu_idx);
  void        Set_pu_arg_area_size(PU_IDX pu_idx, UINT32 size);
  INT         Stack_offset_adjustment_for_pu ();
  ST_IDX      Find_special_return_address_symbol ();
  BOOL        Stack_frame_has_calls () {
    return true;
  }
  STACK_DIR   Stack_direction(void) {
    return STACK_DECREMENT;
  }
  STACK_MODEL Stack_model(void)  {
    return SMODEL_DYNAMIC;
  }
  void        Initialize_frame();
  UINT32      Calculate_stack_frame_size();
  void        Allocate_object(ST_IDX obj_sym);            // register for sym bound variable
  void        Allocate_temp_to_stack(IRNODE_IDX irnode);  // register spilled variable
  void        Allocate_file_statics (void);
  UINT32      Get_ST_formal_preg_num(ST_IDX sym);
  INLINE BOOL ST_has_formal_preg_num (const ST_IDX st) {
    return (ST_sclass(st) == SYMC_FORMAL &&
            Get_ST_formal_preg_num(st) != 0);
  }
};

#endif //OCC_DATA_LAYOUT_H
