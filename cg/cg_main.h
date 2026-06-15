//
// Created by xc5 on 2020/7/24.
//

#ifndef OCC_CG_MAIN_H
#define OCC_CG_MAIN_H

#include "cgir.h"

CG_COMPOSITE *Cgmon();
void CG_process_funcs(FILE_MANAGER *file, COMPILER_CONFIG &config);
INT32 CG_full_process(COMPILER_CONFIG &conf);

// Emitting data section
INT32 Emit_section_data(FILE *out, FILE_MANAGER *man);
void Emit_section_code(FILE *output, FILE_MANAGER *file);
void Emit_function(PU_INFO *func, FILE *out, FILE_MANAGER *file);


#endif //OCC_CG_MAIN_H
