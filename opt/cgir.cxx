//
// Created by xc5 on 2020/7/13.
// This is the IR definition for CG to use, i.e. CG-IR
//
#include "cgir.h"

void CGIR::CG_Init(SCOPE *scope) {
}

void CGIR::Data_layout(SCOPE *scope) {
  layout.Initialize_frame();
  layout.Allocate_file_statics();
}
