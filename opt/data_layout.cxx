//
// Created by xc5 on 2020/7/16.
//
#include "data_layout.h"

void DATA_LAYOUT::Allocate_object(ST_IDX obj_sym) {
  AssertThat(ST_sclass(obj_sym) == SYMC_AUTO,
             ("Symbol should be a local (AUTO)"));
  var_allocated.push_back(obj_sym);
  var_on_stack.push_back(obj_sym);
  q_var_on_stack.insert(obj_sym);
  UINT32 ofst_from_sp_to_symbol = local_size_allocated;
  local_size_allocated += Get_sym_stack_size(obj_sym);
  ST_st(obj_sym)->setSpOffset(ofst_from_sp_to_symbol);
  ST_st(obj_sym)->Set_attr(SYM_ATTR::ST_ALLOCATED);
  var_ofst.insert(std::make_pair(obj_sym, ofst_from_sp_to_symbol));
}


void DATA_LAYOUT::Allocate_formal(ST_IDX obj_sym) {
  AssertThat(ST_sclass(obj_sym) == SYMC_FORMAL,
             ("Symbol should be a param, formal"));
  var_allocated.push_back(obj_sym);
  var_on_formal.push_back(obj_sym);
  q_var_on_formal.insert(obj_sym);
  // first four in
  if (var_on_formal.size() <= 4) {
    var_on_formal_reg.push_back(obj_sym);
  }
  // First four params in register as well
  UINT32 ofst_from_fme_to_symbol = formal_size_allocated;
  formal_size_allocated += Get_sym_stack_size(obj_sym);
    ST_st(obj_sym)->setSpOffset(ofst_from_fme_to_symbol);
  ST_st(obj_sym)->Set_attr(SYM_ATTR::ST_ALLOCATED);
  var_ofst.insert(std::make_pair(obj_sym, ofst_from_fme_to_symbol));
}

UINT32 DATA_LAYOUT::Get_sym_stack_size(ST_IDX obj_sym) const {
  AssertThat(ST_ty(obj_sym) == MTYPE_to_ty(MTYPE_I4) ||
             TY_kind(ST_ty(obj_sym)) == KIND_ARRAY ||
             TY_kind(ST_ty(obj_sym)) == KIND_POINTER,
             ("only i4/array/pointer is processed right now"));
  AssertThat(TY_size(ST_ty(obj_sym)) != 0,
             ("the type %d should be of valid size (>0)", ST_ty(obj_sym)));
  return TY_size(ST_ty(obj_sym));
}

void DATA_LAYOUT::Allocate_temp_to_stack(IRNODE_IDX irnode) {
  AssertThat(false, ("not impl."));
}

void DATA_LAYOUT::Allocate_file_statics(void) {
  AssertThat(false, ("not impl."));
}

void DATA_LAYOUT::Initialize_frame(SCOPE *scope, ST_IDX func) {
  // Add 8 byte calling convention
  _func = func;
  PREG_IDX    preg_sp = File()->Create_preg(File()->Save_string(".preg_sp"), 13);
  preg_sp             = File()->Create_preg(File()->Save_string(".preg_fp"), 11);
  ST_TABLE    *sym    = File()->Tables()->Sym();
  for (UINT32 i       = 0; i < sym->Length(scope); i++) {
    ST_IDX sym_idx = (i << 8) | LOCAL_SYMTAB;
    if (ST_sclass(sym_idx) == SYMC_FORMAL) {
      Allocate_formal(sym_idx);
    }
    if (ST_sclass(sym_idx) == SYMC_AUTO) {
      Allocate_object(sym_idx);
    }
  }
  // Allocate for LR and FP at the end of the stack.
  formal_size_allocated += 8;
}

UINT32 DATA_LAYOUT::Calculate_stack_frame_size() {
  // Make it align to 8 bytes
  if (local_size_allocated % 8 != 0) {
    _padding = 8 - (local_size_allocated % 8);
    local_size_allocated += 8 - (local_size_allocated % 8);
  }
  frame_size = local_size_allocated + formal_size_allocated;
  return frame_size;
}

UINT32 DATA_LAYOUT::Get_ST_formal_preg_num(ST_IDX sym) {
  return 0;
}

BOOL DATA_LAYOUT::ST_pu_defined(ST_IDX sym) {
  return FALSE;
}

ST_IDX DATA_LAYOUT::Get_st_ref_base(ST_IDX sym) {
  return 0;
}

UINT32 DATA_LAYOUT::Stack_alignment() {
  return 4;
}

UINT32 DATA_LAYOUT::Get_pu_arg_area_size(PU_IDX pu_idx) {
  return 0;
}

void DATA_LAYOUT::Set_pu_arg_area_size(PU_IDX pu_idx, UINT32 size) {

}

INT DATA_LAYOUT::Stack_offset_adjustment_for_pu() {
  return 0;
}

ST_IDX DATA_LAYOUT::Find_special_return_address_symbol() {
  return 0;
}

BOOL DATA_LAYOUT::ST_on_stack(ST_IDX sym) {
  return q_var_on_stack.find(sym) != q_var_on_stack.end();
}

UINT32 DATA_LAYOUT::Padding() {
  return _padding;
}

void DATA_LAYOUT::Print(FILE * file) {
  fprintf(file, "%s Printing data layout for func %s  \n%s",
          DBAR, ST_name(_func), DBAR);

  fprintf(file, "Total Stack Size: %d \n", frame_size);
  fprintf(file, "Total Local Size: %d \n", local_size_allocated);
  fprintf(file, "Total Formal Size: %d \n", formal_size_allocated);
  fprintf(file, "Padding calculated: %d \n", _padding);
  for (UINT32 i = 0; i < var_on_stack.size(); i++) {
    unsigned int &sym_idx = var_on_stack[i];
    fprintf(file, "Stack-elem ID : %-6d, Kind: %-7s, Sym = %-8s, Sp-offset = %-6d == %-6d\n",
            i,
            ST_sclass(sym_idx) == SYMC_FORMAL ? "FORMAL" : "LOCAL",
            ST_name(sym_idx),
            ST_st(sym_idx)->getSpOffset(),
            var_ofst[sym_idx]);
  }
  fprintf(file, "%s End of data layout for func %s  \n%s",
          DBAR, ST_name(_func), DBAR);
}

UINT32 DATA_LAYOUT::Get_sym_sp_ofst(ST_IDX obj_sym) const {
  AssertThat(var_ofst.find(obj_sym) != var_ofst.end(),
             ("Not found in ofst map. Not allocated? %s", ST_name(obj_sym)));
  AssertThat(frame_size > 0, ("The final calculation should already be done. "
                              "PHASE ordering issues?"));
  return var_ofst.at(obj_sym) + _padding;
}

UINT32 DATA_LAYOUT::Frame_final_size() {
  return frame_size;
}









