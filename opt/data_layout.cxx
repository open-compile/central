//
// Created by xc5 on 2020/7/16.
//
#include "data_layout.h"

void DATA_LAYOUT::Allocate_object(ST_IDX obj_sym) {

}

void DATA_LAYOUT::Allocate_temp_to_stack(IRNODE_IDX irnode) {

}

void DATA_LAYOUT::Allocate_file_statics(void) {

}

void DATA_LAYOUT::Initialize_frame() {
  // Add 8 byte calling convention
}

UINT32 DATA_LAYOUT::Calculate_stack_frame_size() {
  return 0;
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
  return TRUE;
}









