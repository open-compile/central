//
// Created by lugt on 24-4-17.
//
#ifndef CG_NOVA_H
#define CG_NOVA_H
#include "cgn_common.h"
#include "options.h"
#include "cfg_common.h"

// Define CG-opcode
class CGNOP {

};

template<typename N>
class CGN_CFG_BB_BASE: public CFG_BB_BASE<N> {
  CGN_CFG_BB_BASE(): CFG_BB_BASE<N>() {
  }
};

// BB definitions
typedef CGN_CFG_BB_BASE<CGNOP> CGBB;
typedef vector<CGBB *> CGBB_VECTOR;
typedef typename vector<CGBB *>::iterator CGBB_ITER;

CG_MANAGER *Cgman();
void CGN_process_funcs(FILE_MANAGER *file, COMPILER_CONFIG &config);
INT32 CGN_full_process(COMPILER_CONFIG &conf);

// Emitting data section
INT32 Cgn_emit_section_data(FILE *out, FILE_MANAGER *man);
void Cgn_emit_section_code(FILE *output, FILE_MANAGER *file);
void Cgn_emit_function(PU_INFO *func, FILE *out, FILE_MANAGER *file);


#endif //CG_NOVA_H
