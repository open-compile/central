#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_basic.h"
#include <cstring>

static CR_POOL *_pool = nullptr;

CR_POOL *Cr_pool() {
  if (!_pool) _pool = new CR_POOL();
  return _pool;
}

CODEREP *CODEREP::Make_const(INT64 val, MTYPE_ID mtype) {
  CODEREP *cr = Cr_pool()->Alloc();
  cr->_kind = CK_CONST;
  cr->_mtype = mtype;
  cr->_const_val = val;
  return cr;
}

CODEREP *CODEREP::Make_lda(ST_IDX sym, INT32 offset, MTYPE_ID mtype) {
  CODEREP *cr = Cr_pool()->Alloc();
  cr->_kind = CK_LDA;
  cr->_mtype = mtype;
  cr->_lda._lda_sym = sym;
  cr->_lda._lda_offset = offset;
  return cr;
}

CODEREP *CODEREP::Make_var(AUX_ID aux, UINT32 ver, MTYPE_ID mtype) {
  CODEREP *cr = Cr_pool()->Alloc();
  cr->_kind = CK_VAR;
  cr->_mtype = mtype;
  cr->_var._aux_id = aux;
  cr->_var._version = ver;
  cr->_var._def_stmt = nullptr;
  cr->_var._def_phi = nullptr;
  return cr;
}

CODEREP *CODEREP::Make_ivar(CODEREP *base, INT32 off, TY_IDX ty, MTYPE_ID mtype) {
  CODEREP *cr = Cr_pool()->Alloc();
  cr->_kind = CK_IVAR;
  cr->_mtype = mtype;
  cr->_ivar._base = base;
  cr->_ivar._offset = off;
  cr->_ivar._ty = ty;
  return cr;
}

CODEREP *CODEREP::Make_op(OPCODE opc, CODEREP *k0, CODEREP *k1, MTYPE_ID mtype) {
  CODEREP *cr = Cr_pool()->Alloc();
  cr->_kind = CK_OP;
  cr->_mtype = mtype;
  cr->_op._opc = opc;
  cr->_op._num_kids = (k1 == nullptr) ? 1 : 2;
  cr->_op._kids[0] = k0;
  cr->_op._kids[1] = k1;
  return cr;
}

static const char *CR_kind_name(CR_KIND k) {
  switch (k) {
    case CK_CONST: return "CONST";
    case CK_LDA:   return "LDA";
    case CK_VAR:   return "VAR";
    case CK_IVAR:  return "IVAR";
    case CK_OP:    return "OP";
    default:       return "?";
  }
}

void CODEREP::Print(FILE *f) const {
  fprintf(f, "%s#%u[%s]=", CR_kind_name(_kind), _id, CR_kind_name(_kind));
  switch (_kind) {
    case CK_CONST: fprintf(f, "%lld", (long long)_const_val); break;
    case CK_LDA:   fprintf(f, "st=%d off=%d", (int)_lda._lda_sym, _lda._lda_offset); break;
    case CK_VAR:   fprintf(f, "v%d_v%d", (int)_var._aux_id, (int)_var._version); break;
    case CK_IVAR:  fprintf(f, "offset=%d ty=%d", _ivar._offset, (int)_ivar._ty); break;
    case CK_OP:    fprintf(f, "%s k=%d", OPCODE_name(_op._opc), _op._num_kids); break;
    default: break;
  }
}

void PHI_NODE::Print(FILE *f) const {
  fprintf(f, "phi v%d_v%d = (", (int)_aux_id, (int)_result_ver);
  for (UINT32 i = 0; i < _opnd_vers.size(); ++i) {
    if (i > 0) fprintf(f, ", ");
    fprintf(f, "v%d_v%d", (int)_aux_id, (int)_opnd_vers[i]);
  }
  fprintf(f, ")\n");
}
