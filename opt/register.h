//
// Created by xc5 on 2020/7/16.
//

#ifndef OCC_REGISTER_H
#define OCC_REGISTER_H

#include "basic.h"

const int ISA_REGISTER_MAX = 33;
const int ISA_REGISTER_CLASS_MAX = 33;

#define ISA_REGISTER_SUBCLASS_MAX ISA_REGISTER_CLASS_MAX
#define REGISTER_MAX ISA_REGISTER_MAX

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


/* Define the access macros
 */
#define Set_CLASS_REG_PAIR_class_n_reg(x,cnr)	((x).class_n_reg = (cnr))
#define     CLASS_REG_PAIR_class_n_reg(x)	((x).class_n_reg+0)
#define Set_CLASS_REG_PAIR_rclass(x,rc)		((x).class_reg.rclass = (rc))
#define     CLASS_REG_PAIR_rclass(x)		((ISA_REGISTER_CLASS)(x).class_reg.rclass)
#define Set_CLASS_REG_PAIR_reg(x,r)		((x).class_reg.reg = (r))
#define     CLASS_REG_PAIR_reg(x)		((x).class_reg.reg+0)

// Do not inline this function to avoid a g++ optimizer bug!
extern void Set_CLASS_REG_PAIR(CLASS_REG_PAIR& rp, ISA_REGISTER_CLASS rclass, REGISTER reg);

/* Create a CLASS_REG_PAIR from rclass/reg pair */
#define CREATE_CLASS_N_REG(rclass,reg)	((mUINT16)(((reg)<<8)|(rclass)))
#define CONSTRUCT_CLASS_REG_PAIR(cl_reg_pair,rclass,reg)	\
		(Set_CLASS_REG_PAIR_class_n_reg(cl_reg_pair, \
				CREATE_CLASS_N_REG(rclass,reg))


/**
 * Register set definition, is an array here.
 */

typedef struct {
  UINT64 v[(ISA_REGISTER_MAX/64)+1];
} REGISTER_SET;

extern const REGISTER_SET REGISTER_SET_EMPTY_SET;

/* These are private!!!!
 */
#define REGISTER_SET_WORD		UINT64
#define REGISTER_SET_ELEM(set, idx)	((set).v[(idx)])
#define REGISTER_SET_IDX(iv)		INT iv
#define FOR_REGISTER_SET(init, test, incr) for ((init); (test); (incr))
#define MAX_REGISTER_SET_IDX		(ISA_REGISTER_MAX/64)
#define REGISTER_SET_WORD_IDX(bit)	((bit) >> 6)
#define REGISTER_SET_BIT_IDX(bit)	((bit) & (64-1))


/****************************************************************************
 * Register info class
 ***************************************************************************/
typedef struct {
  UINT16           reg_machine_id[REGISTER_MAX + 1];
  UINT8            reg_bit_size[REGISTER_MAX + 1];
  BOOL             reg_allocatable[REGISTER_MAX + 1];
  BOOL             can_store;
  BOOL		         multiple_save;
  UINT16           register_count;
  const char       *name;
  REGISTER_SET      universe;
  REGISTER_SET      allocatable;
  REGISTER_SET      caller_saves;
  REGISTER_SET      callee_saves;
  REGISTER_SET      function_value;
  REGISTER_SET      function_argument;
  REGISTER_SET      shrink_wrap;
  REGISTER_SET	    stacked;
  REGISTER_SET      rotating;
} REGISTER_CLASS_INFO;


extern ISA_REGISTER_CLASS
REGISTER_CLASS_vec[ISA_REGISTER_CLASS_MAX + 1];

/* Cached information about each ISA_REGISTER_CLASS:
 */
extern REGISTER_CLASS_INFO
REGISTER_CLASS_info[ISA_REGISTER_CLASS_MAX + 1];

/* Accessing cached information about a ISA_REGISTER_CLASS
 */
#define REGISTER_CLASS_reg_allocatable(x)			\
				(REGISTER_CLASS_info[x].reg_allocatable)
#define REGISTER_CLASS_reg_machine_id(x)			\
				(REGISTER_CLASS_info[x].reg_machine_id)
#define REGISTER_CLASS_reg_name(x)				\
				(REGISTER_CLASS_info[x].reg_name)
#define REGISTER_CLASS_reg_bit_size(x)				\
				(REGISTER_CLASS_info[x].reg_bit_size)
#define REGISTER_CLASS_name(x)	(REGISTER_CLASS_info[x].name)
#define REGISTER_CLASS_register_count(x)			\
                                (REGISTER_CLASS_info[x].register_count)
#define REGISTER_CLASS_universe(x)				\
                                (REGISTER_CLASS_info[x].universe)
#define REGISTER_CLASS_allocatable(x)				\
                                (REGISTER_CLASS_info[x].allocatable)
#define REGISTER_CLASS_caller_saves(x)				\
                                (REGISTER_CLASS_info[x].caller_saves)
#define REGISTER_CLASS_callee_saves(x)				\
                                (REGISTER_CLASS_info[x].callee_saves)
#define REGISTER_CLASS_stacked(x)				\
                                (REGISTER_CLASS_info[x].stacked)
#define REGISTER_CLASS_rotating(x)				\
                                (REGISTER_CLASS_info[x].rotating)
#define REGISTER_CLASS_function_value(x)			\
                                (REGISTER_CLASS_info[x].function_value)
#define REGISTER_CLASS_function_argument(x)			\
                                (REGISTER_CLASS_info[x].function_argument)
#define REGISTER_CLASS_shrink_wrap(x)				\
                                (REGISTER_CLASS_info[x].shrink_wrap)
#define REGISTER_CLASS_can_store(x)				\
                                (REGISTER_CLASS_info[x].can_store)
#define REGISTER_CLASS_multiple_save(x)				\
                                (REGISTER_CLASS_info[x].multiple_save)

#define REGISTER_CLASS_last_register(x)				\
			(REGISTER_CLASS_register_count(x) + REGISTER_MIN - 1)

#define REGISTER_machine_id(rclass,reg)				\
                                (REGISTER_CLASS_reg_machine_id(rclass)[reg])
#define REGISTER_allocatable(rclass,reg)			\
				(REGISTER_CLASS_reg_allocatable(rclass)[reg])

#define REGISTER_bit_size(rclass,reg)				\
				(REGISTER_CLASS_reg_bit_size(rclass)[reg])

/* Macro to iterate over all members of a REGISTER_SET. */
#define FOR_ALL_REGISTER_SET_members(set,reg)	\
    for (reg = REGISTER_SET_Choose (set);	\
	 reg != REGISTER_UNDEFINED;		\
	 reg = REGISTER_SET_Choose_Next(set,reg))

typedef struct {
  const char        *name;
  const char        *reg_name[REGISTER_MAX + 1];
  REGISTER_SET       members;
  ISA_REGISTER_CLASS rclass;
} REGISTER_SUBCLASS_INFO;


/* Cached information about each ISA_REGISTER_SUBCLASS:
 */
extern REGISTER_SUBCLASS_INFO
REGISTER_SUBCLASS_info[ISA_REGISTER_SUBCLASS_MAX + 1];

/* Accessing cached information about a ISA_REGISTER_SUBCLASS
 */
#define REGISTER_SUBCLASS_name(x)			\
				(REGISTER_SUBCLASS_info[x].name)
#define REGISTER_SUBCLASS_members(x)			\
				(REGISTER_SUBCLASS_info[x].members)
#define REGISTER_SUBCLASS_register_class(x)		\
				(REGISTER_SUBCLASS_info[x].rclass)
#define REGISTER_SUBCLASS_reg_name(x,reg)		\
				(REGISTER_SUBCLASS_info[x].reg_name[reg])

/* The various special purpose registers:
 */
extern CLASS_REG_PAIR		CLASS_REG_PAIR_zero;
#define REGISTER_zero		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_zero)
#define REGISTER_CLASS_zero	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_zero)
#define CLASS_AND_REG_zero	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_zero)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_ep;
#define REGISTER_ep		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ep)
#define REGISTER_CLASS_ep	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ep)
#define CLASS_AND_REG_ep	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_ep)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_gp;
#define REGISTER_gp		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gp)
#define REGISTER_CLASS_gp	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gp)
#define CLASS_AND_REG_gp	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_gp)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_sp;
#define REGISTER_sp		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_sp)
#define REGISTER_CLASS_sp	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_sp)
#define CLASS_AND_REG_sp	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_sp)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_fp;
#define REGISTER_fp		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fp)
#define REGISTER_CLASS_fp	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fp)
#define CLASS_AND_REG_fp	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_fp)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_ra;
#define REGISTER_ra		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ra)
#define REGISTER_CLASS_ra	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ra)
#define CLASS_AND_REG_ra	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_ra)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_v0;
#define REGISTER_v0		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0)
#define REGISTER_CLASS_v0	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0)
#define CLASS_AND_REG_v0	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_v0)

/* The static link may NOT be the same as v0 (i.e. $2) */
extern CLASS_REG_PAIR		CLASS_REG_PAIR_static_link;
#define REGISTER_static_link		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_static_link)
#define REGISTER_CLASS_static_link	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_static_link)
#define CLASS_AND_REG_static_link	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_static_link)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_pfs;
#define REGISTER_pfs		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_pfs)
#define REGISTER_CLASS_pfs	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_pfs)
#define CLASS_AND_REG_pfs	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_pfs)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_lc;
#define REGISTER_lc		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_lc)
#define REGISTER_CLASS_lc	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_lc)
#define CLASS_AND_REG_lc	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_lc)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_ec;
#define REGISTER_ec		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ec)
#define REGISTER_CLASS_ec	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ec)
#define CLASS_AND_REG_ec	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_ec)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_true;
#define REGISTER_true		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_true)
#define REGISTER_CLASS_true	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_true)
#define CLASS_AND_REG_true	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_true)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_fzero;
#define REGISTER_fzero		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fzero)
#define REGISTER_CLASS_fzero	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fzero)
#define CLASS_AND_REG_fzero	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_fzero)

extern CLASS_REG_PAIR		CLASS_REG_PAIR_fone;
#define REGISTER_fone		CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fone)
#define REGISTER_CLASS_fone	CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fone)
#define CLASS_AND_REG_fone	CLASS_REG_PAIR_class_n_reg(CLASS_REG_PAIR_fone)

// Functions
extern void REGISTER_Begin(void);
extern void REGISTER_Pu_Begin(void);

#endif //OCC_REGISTER_H
