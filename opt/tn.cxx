//
// Created by xc5 on 2020/7/24.
//
#include <vector>
#include "tn.h"
#include <memory.h>
#include <ir.h>
#include "options.h"
#include "cg_main.h"

std::vector<TN>              _global_tn_vec;

#define POINTER_SIZE 4

/*
 *
 *  *   TN
 *
 *     The Temporary Name (TN) structure describes the operands and result
 *     of OPs. A TN can be used to describe either a register or a constant.
 *     The TN_flags(tn) field is applicable to all TNs and is used to determine
 *     the correct variant of the TN. The TN_size(tn) field is also applicable
 *     to all TNs and gives the size of the operand/result in bytes.
 *     All the other fields are applicable in specific contexts as described
 *     below.
 *
 */
namespace TN_CONTEXT {
  BOOL is_str_expand = true;

/**
 * Dedicated TN Groups
 */
  static TN *ded_tns[ISA_REGISTER_CLASS_MAX + 1][REGISTER_MAX + 1];
  static TN *f4_ded_tns[REGISTER_MAX + 1];
  static TN *v16_ded_tns[REGISTER_MAX + 1];
  static TN *v32_ded_tns[REGISTER_MAX + 1];
  static TN *i1_ded_tns[REGISTER_MAX + 1];
  static TN *i2_ded_tns[REGISTER_MAX + 1];
  TN *i4_ded_tns[REGISTER_MAX + 1];

  TN *RA_TN    = NULL;
  TN *SP_TN    = NULL;
  TN *FP_TN    = NULL;
  TN *Ep_TN    = NULL;
  TN *GP_TN    = NULL;
  TN *Zero_TN  = NULL;
  TN *Pfs_TN   = NULL;
  TN *True_TN  = NULL;
  TN *FZero_TN = NULL;
  TN *FOne_TN  = NULL;
  TN *LC_TN    = NULL;

  /* Keep track of the TN_number for the last register TN generated. The
   * first numbered TN is #1; #0 must remain unused (various algorithms
   * make special use of 0).
   */
  // TN_IDX Last_TN = 0;, Use Last_TN() instead, and Increment_last_tn();

/* TN_number of the last dedicated TN */
  TN_IDX Last_Dedicated_TN = 0;
/* TN_number of the last distinct dedicated TN*/
  TN_IDX Last_Distinct_Dedicated_TN = 0;
/* TN_number of the first non-dedicated TN. */
  TN_IDX First_Regular_TN = 0;
/* TN_number of the first non-dedicated TN in the current REGION. */
  TN_IDX First_REGION_TN = 0;

};

using namespace TN_CONTEXT;

TN *Gen_TN() {
  UINT32 sz = Cgir()->Get_tn_table().size();
  Cgir()->Get_tn_table().push_back(TN(sz));
  Cgir()->Get_tn_table()[sz].Set_idx(sz);
  AssertThat(Cgir()->Get_tn_table()[sz].Get_tn_idx() == sz, ("Generation failed somehow."));
  AssertThat(TN_tn(Cgir()->Get_tn_table()[sz].Get_tn_idx()) ==
             &(Cgir()->Get_tn_table()[sz]), ("Generation failed somehow."));
  return &(Cgir()->Get_tn_table()[sz]);
}

TN_IDX Gen_TN(MTYPE_ID mtype) {
  TN *tn = Gen_TN();
  tn->Set_type(mtype);
  tn->Set_size(MTYPE_size(mtype));
  UINT32 tn_id = TN_tn_idx(tn);
  AssertThat(tn_id > 0 && tn_id < 40960, ("Incorrect range of tn_idx = %u", tn_id));
  return tn_id;
}

TN *TN_tn(TN_IDX tn_idx) {
  AssertThat(Cgir()->Get_tn_table().size() > tn_idx,
             ("TN_IDX = %d is out of max range = %d", tn_idx, Cgir()->Get_tn_table().size()));
  return &(Cgir()->Get_tn_table()[tn_idx]);
}

namespace TNS {
  UINT32 _last_tn = 0;
}

void Check_TN_Vec_Size() {
  AssertThat(TNS::_last_tn < 4096 && Cgir()->Get_tn_table().size() < 4096,
             ("Too much TNs used"));
}

/* ====================================================================
 *
 * Dup_TN
 *
 * Duplicate a TN with a new number.
 *
 * The TN_GLOBAL_REG flag and any spill location associated with this TN
 * is cleared in the new TN.
 *
 * ====================================================================
 */

TN *
Dup_TN ( TN *tn )
{
  TN *new_tn = Gen_TN();

  AssertThat(! TN_is_dedicated(tn),("Dup_TN of a dedicated TN: TN%d",
    TN_tn_idx(tn)));

  new_tn->Dup_from(tn);
  if (!TN_is_constant(new_tn)) {
    Reset_TN_is_global_reg(new_tn);
    TN_Allocate_Register (new_tn, REGISTER_UNDEFINED);
    /* copy over TN_home for rematerializable TNs. */
    if (!TN_is_rematerializable(tn) && !TN_is_gra_homeable(tn)) {
      Set_TN_spill(new_tn, 0);
    }
  }
  return new_tn;
}


/* ====================================================================
 *
 * Create_Dedicated_TN
 *
 * Create and initialize a new dedicated TN and remember it for later.
 *
 * ====================================================================
 */
static TN *
Create_Dedicated_TN (ISA_REGISTER_CLASS rclass, REGISTER reg)
{
  INT size = REGISTER_bit_size(rclass, reg) / 8;
  /* Allocate the dedicated TN at file level, because we reuse them
   * for all PUs.
   */
  TN *tn = Gen_TN();
  Set_TN_is_dedicated(tn);
  Set_TN_register_class(tn, rclass);
  Set_TN_register(tn, reg);
  Set_TN_size(tn, size);
  return(tn);
}

/* ====================================================================
 *
 * Init_Dedicated_TNs
 *
 * See interface description.
 *
 * ====================================================================
 */
void
Init_Dedicated_TNs(void) {
  ISA_REGISTER_CLASS rclass;
  REGISTER           reg;
  TN_IDX             tnum = 0;
  Create_Dedicated_TN((ISA_REGISTER_CLASS_integer),  0);
  FOR_ALL_ISA_REGISTER_CLASS(rclass) {
    for (reg = REGISTER_MIN;
         reg <= REGISTER_CLASS_last_register(rclass);
         reg++) {
      ded_tns[rclass][reg] = Create_Dedicated_TN(rclass, reg);
    }
  }
  Last_Distinct_Dedicated_TN = tnum;
  for (reg = REGISTER_MIN;
       reg <= REGISTER_CLASS_last_register(ISA_REGISTER_CLASS_integer);
       reg++) {
    i4_ded_tns[reg] = Create_Dedicated_TN(ISA_REGISTER_CLASS_integer, reg);
    Set_TN_size(i4_ded_tns[reg], 4);
  }
  Last_Dedicated_TN = Cgir()->Get_tn_table().size();
}



/* ====================================================================
 *
 * Build_Dedicated_TN
 *
 * See interface description.
 *
 * ====================================================================
 */
TN *
Build_Dedicated_TN (ISA_REGISTER_CLASS rclass, REGISTER reg, INT size)
{
  if(rclass == ISA_REGISTER_CLASS_integer) {
    return i4_ded_tns[reg];
  }

  AssertThat(ded_tns[rclass][reg] != NULL,
             ("Cannot use such TN, rclass = %d, reg = %d", rclass, reg));
  return ded_tns[rclass][reg];
}

TN *
Gen_Register_TN (ISA_REGISTER_CLASS rclass, INT size)
{
  AssertThat(rclass != ISA_REGISTER_CLASS_UNDEFINED,
             ("Gen_Register_TN called with undefined reg class"));
  if ( REGISTER_SET_EmptyP(REGISTER_CLASS_allocatable(rclass)) ) {
    // only one reg in class, so make dedicated tn
    AssertThat(REGISTER_CLASS_register_count(rclass) == 1,
              ("don't know how to make dedicated TN for class %s",
                REGISTER_CLASS_name(rclass)));
    return Build_Dedicated_TN(rclass, REGISTER_MIN, size);
  }
  else {
    TN *tn = Gen_TN();
    if ( size > 32 ) {
      AssertThat(false, ("Size should not exceed 32bit."));
    }
    Set_TN_size(tn, size);
    Set_TN_register_class(tn, rclass);
    return tn;
  }
}

void TN::Print(FILE *file) {
  // Printing the TN.
  fprintf(file, "flags : ");
  if (TN_is_dedicated(this)) {
    fprintf(file, "dedicated ");
  }
  if (TN_is_preallocated(this)) {
    fprintf(file, "preallocated ");
  }
  if (TN_is_ra_reg(this)) {
    fprintf(file, "r0(ret-value) ");
  }
  if (TN_is_sp_reg(this)) {
    fprintf(file, "sp(stack pointer) ");
  }
  fprintf(file, ", kv : ");
  if (TN_is_constant(this)) {
    fprintf(file, "const = %lld ", TN_value(this));
  }
  if (TN_is_register(this)) {
    fprintf(file, "register = %d, class = %d ", TN_register(this), TN_register_class(this));
    if (TN_register_class(this) == REGISTER_CLASS_fp) {
      fprintf(file, "(fp)");
    }
  }
  if (TN_is_label(this)) {
    fprintf(file, "label = %d ", TN_label(this));
  }
  fprintf(file, "\n");
}

void TN::Dup_from(TN *pTn) {
  UINT32 tn_number = this->Get_tn_idx();
  memcpy(this, pTn, sizeof(TN));
  this->tn_idx = tn_number;
}

void TN::Set_type(MTYPE_ID id) {

}

void TN::Set_idx(UINT32 i) {
  tn_idx = i;
}


// gen unique literal tn
TN *
Gen_Unique_Literal_TN (INT64 ivalue, INT size)
{
  TN *tn = Gen_TN ();
  Set_TN_size(tn, size);
  Set_TN_is_constant(tn);
  Set_TN_has_value(tn);
  Set_TN_value(tn, ivalue);
  return tn;
}

/* ====================================================================
 *
 * Gen_Literal_TN
 *
 * Produce a literal TN with the given literal value and size, either a
 * pre-existing one or a newly created one.
 *
 * ====================================================================
 */

TN *
Gen_Literal_TN ( INT64 ivalue, INT size )
{
  TN *tn;
  AssertThat(size != 4 || (ivalue >= -2147483648LL && ivalue <= 4294967295LL),
             ("Gen_Literal_TN: 4-byte literal 0x%016llx is out-of-range", ivalue));

  /* Check if there is already a constant TN with this value. Otherwise
   * create a new one and add it to the hash table.
   */
  tn = NULL; //Search_For_Previous_Constant (ivalue, size);
  if (tn == NULL) {
    tn = Gen_TN ();
    Set_TN_size(tn, size);
    Set_TN_is_constant(tn);
    Set_TN_has_value(tn);
    TN_value(tn) = ivalue;
  }
  return tn;
}

/* ====================================================================
 *
 * Gen_Symbol_TN
 *
 * Produce a TN with the given symbol value, offset and relocs.
 * This TN will represent the value of the symbol's address plus the
 * offset.  Note that, for stack frame symbols, the address represented
 * is relative to the stack pointer rather than absolute.
 *
 * ====================================================================
 */

TN *
Gen_Symbol_TN ( ST_IDX st, INT64 offset, INT32 relocs)
{
  TN *tn;
  INT hash_value;

  /* First try to find an existing symbol TN */
  tn = NULL; // Search_For_Previous_Symbol (st, offset, relocs);
  if (tn == NULL) {
    tn = Gen_TN ();
    Set_TN_size(tn, POINTER_SIZE);
    Set_TN_is_constant(tn);
    Set_TN_is_symbol(tn);
    Set_TN_var(tn, st);
    TN_offset(tn) = offset;
    TN_relocs(tn) = relocs;
    // TODO: Save somewhere
  }
  return tn;
}


/* ====================================================================
 *
 * Gen_Label_TN
 *
 * Produce a TN with the given label value and offset.  This TN will
 * represent the value of the label's address plus the offset.
 *
 * ====================================================================
 */

TN *
Gen_Label_TN ( LABEL_IDX lab, INT64 offset )
{
  TN *tn;

  /* Make an new one and put it into the table: */
  tn = Gen_TN ();
  Set_TN_size(tn, POINTER_SIZE);
  Set_TN_is_constant(tn);
  Set_TN_is_label(tn);
  TN_label(tn) = lab;
  TN_offset(tn) = offset;
  return tn;
}

TN *
Gen_Tag_TN ( LABEL_IDX tag)
{
  TN *tn;

  /* Make an new one and put it into the table: */
  tn = Gen_TN ();
  Set_TN_size(tn, POINTER_SIZE);
  Set_TN_is_constant(tn);
  Set_TN_is_tag(tn);
  TN_label(tn) = tag;
  return tn;
}
