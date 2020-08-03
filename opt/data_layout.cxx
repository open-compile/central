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
  ST_st(obj_sym)->Set_attr(SYM_ATTR::ST_ALLOCATED);
  var_ofst.insert(std::make_pair(obj_sym, ofst_from_sp_to_symbol));
}


void DATA_LAYOUT::Allocate_formal(ST_IDX idx, UINT32 i) {
  AssertThat(ST_sclass(idx) == SYMC_FORMAL,
             ("Symbol should be a param, formal"));
  var_allocated.push_back(idx);
  // First four params in register and storable on the local area.
  UINT32 ofst_from_fme_to_symbol = 0;
  if (var_on_formal.size() <= 4) {
    var_on_stack.push_back(idx);
    q_var_on_stack.insert(idx);
    ofst_from_fme_to_symbol = local_size_allocated;
    var_on_reg_map.insert(std::make_pair(idx, var_on_formal_reg.size()));
    var_on_formal_reg.push_back(idx);
    local_size_allocated += Get_sym_stack_size(idx);
  } else {
    var_on_formal.push_back(idx);
    q_var_on_formal.insert(idx);
    ofst_from_fme_to_symbol = formal_size_allocated;
    formal_size_allocated += Get_sym_stack_size(idx);
  }
  ST_st(idx)->Set_attr(SYM_ATTR::ST_ALLOCATED);
  var_ofst.insert(std::make_pair(idx, ofst_from_fme_to_symbol));
}

UINT32 DATA_LAYOUT::Get_sym_stack_size(ST_IDX obj_sym) const {
  AssertThat(ST_ty(obj_sym) == MTYPE_to_ty(MTYPE_I4) ||
             TY_kind(ST_ty(obj_sym)) == KIND_ARRAY ||
             TY_kind(ST_ty(obj_sym)) == KIND_POINTER,
             ("only i4/array/pointer is processed right now"));
  UINT32 size = TY_size(ST_ty(obj_sym));
  if (ST_sclass(obj_sym) == SYMC_FORMAL && TY_kind(ST_ty(obj_sym)) == KIND_ARRAY) {
    // pointer used.
    size = MTYPE_size(MTYPE_A4);
  }
  AssertThat(size != 0,
             ("the type %d should be of valid size (>0)", ST_ty(obj_sym)));
  return size;
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
  UINT32      args_met  = 0;
  for (UINT32 i       = 0; i < sym->Length(scope); i++) {
    ST_IDX sym_idx = (i << 8) | LOCAL_SYMTAB;
    if (ST_sclass(sym_idx) == SYMC_FORMAL) {
      Allocate_formal(sym_idx, args_met);
      args_met ++;
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
  frame_size = local_size_allocated + formal_size_allocated;
  if (frame_size % 8 != 0) {
    _padding = 8 - (frame_size % 8);
    frame_size += _padding;
  }
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
    fprintf(file, "Stack-elem ID : %-6d, Kind: %-7s, Sym = %-8s, Sp-offset = %-6d\n",
            i,
            ST_sclass(sym_idx) == SYMC_FORMAL ? "FORMAL" : "LOCAL",
            ST_name(sym_idx),
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
  if (ST_sclass(obj_sym) == SYMC_FORMAL &&
      std::find(var_on_formal_reg.begin(), var_on_formal_reg.end(), obj_sym)
               == var_on_formal_reg.end()) {
    return var_ofst.at(obj_sym) + _padding + local_size_allocated;
  }
  return var_ofst.at(obj_sym) + _padding;
}

UINT32 DATA_LAYOUT::Frame_final_size() {
  Calculate_stack_frame_size();
  return frame_size;
}

UINT32 DATA_LAYOUT::Get_sym_reg_num(ST_IDX sym) {
  return var_on_reg_map[sym];
}

vector<ST_IDX> &DATA_LAYOUT::Get_sym_on_formal_reg() {
  return var_on_formal_reg;
}









