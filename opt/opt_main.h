//
// Created by xc5 on 2020/6/20.
//

#ifndef OCC_OPT_MAIN_H
#define OCC_OPT_MAIN_H

// Emitting data section
INT32 Emit_section_data(FILE *out, FILE_MANAGER *man);

enum IR_LEVEL {
  LEVEL_VHIGH = 1,
  LEVEL_HIGH  = 2,
  LEVEL_MID   = 3,
  LEVEL_LOW   = 4,
  LEVEL_VLOW  = 5,
  LEVEL_CGIR  = 6,
};

void Emit_section_code(FILE *output, FILE_MANAGER *file);

void Emit_function(PU_INFO *func, FILE *out, FILE_MANAGER *file);

void Emit_tree(PU_INFO *func, TREE *tree, FILE *out, FILE_MANAGER *file);

void Opt_lower(FILE_MANAGER *file, IR_LEVEL target, COMPILER_CONFIG &config);

void Opt_verify(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &conf);

void Opt_verify_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                         COMPILER_CONFIG &conf);

void Opr_lower_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                        COMPILER_CONFIG &conf);

#endif //OCC_OPT_MAIN_H
