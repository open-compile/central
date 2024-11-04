//
// Created by xc5 on 2020/6/20.
//

#ifndef OCC_OPT_MAIN_H
#define OCC_OPT_MAIN_H

#include "opt_basic.h"

// TODO: separate different level lowering to different functions.
//void Opt_lower_function_high(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL target, COMPILER_CONFIG &config);
//void Opt_lower_function_mid(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL target, COMPILER_CONFIG &config);
//void Opt_lower_function_low(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL target, COMPILER_CONFIG &config);
//void Opt_lower_function_very_low(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL target, COMPILER_CONFIG &config);

// SSA conversion
void Opt_build_ssa(PU_INFO *pu, FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config);
void Opt_build_ssa_all(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config);

void Opt_lower(FILE_MANAGER *file, IR_LEVEL target, COMPILER_CONFIG &config);

void Opt_verify(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &conf);

void Opt_verify_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                         COMPILER_CONFIG &conf);

void Opr_lower_function(PU_INFO *func, FILE_MANAGER *file, IR_LEVEL level,
                        COMPILER_CONFIG &conf);

IR_ITER Opt_lower_stmt(IR_ITER stmt, PU_INFO *func, FILE_MANAGER *file,
                       IR_LEVEL level, COMPILER_CONFIG &conf);

#endif //OCC_OPT_MAIN_H
