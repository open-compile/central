// ============================================================================
// CLAUDE-MARKER  STATUS: KEEP (data structure + factory methods)
// 配套 opt_coderep.h; 实现 CODEREP / PHI_NODE 的 Print, Make_* 工厂
// ============================================================================
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

// 打印 CODEREP 紧凑形式（无空格，可嵌套）
//   e.g.  VAR#1[aux=3 ver=0 def=null]
//         CONST#2[42]
//         OP#3[ADD kid=2 (1, 2)]
void CODEREP::Print(FILE *f) const {
  if (!f) f = stderr;
  fprintf(f, "%s#%u[", CR_kind_name(_kind), _id);
  switch (_kind) {
    case CK_CONST:
      fprintf(f, "%lld]", (long long)_const_val);
      break;
    case CK_LDA:
      fprintf(f, "st=%d off=%d]", (int)_lda._lda_sym, _lda._lda_offset);
      break;
    case CK_VAR:
      fprintf(f, "aux=%u ver=%u def=%s]",
              (UINT32)_var._aux_id, _var._version,
              _var._def_phi ? "phi" :
                (_var._def_stmt ? "stmt" : "null"));
      break;
    case CK_IVAR:
      fprintf(f, "off=%d ty=%d base=", _ivar._offset, (int)_ivar._ty);
      if (_ivar._base) _ivar._base->Print(f);
      fprintf(f, "]");
      break;
    case CK_OP:
      fprintf(f, "%s k=%d (", OPCODE_name(_op._opc), _op._num_kids);
      for (UINT8 i = 0; i < _op._num_kids; ++i) {
        if (i > 0) fprintf(f, ", ");
        if (_op._kids[i]) _op._kids[i]->Print(f);
        else fprintf(f, "null");
      }
      fprintf(f, ")]");
      break;
    default:
      fprintf(f, "]");
      break;
  }
}

// 漂亮打印：一个 cr 占用一行（递归打印 kids）
static void print_cr_pretty(const CODEREP *cr, INT32 depth, FILE *f) {
  if (!cr) { fprintf(f, "  null\n"); return; }
  for (INT32 i = 0; i < depth; ++i) fprintf(f, "  ");
  fprintf(f, "%s mtype=%d id=%u ", CR_kind_name(cr->_kind), cr->_mtype, cr->_id);
  switch (cr->_kind) {
    case CK_CONST:
      fprintf(f, "val=%lld\n", (long long)cr->_const_val); break;
    case CK_LDA:
      fprintf(f, "st=%d off=%d\n", (int)cr->_lda._lda_sym, cr->_lda._lda_offset); break;
    case CK_VAR:
      fprintf(f, "aux=%u ver=%u def=%s\n",
              (UINT32)cr->_var._aux_id, cr->_var._version,
              cr->_var._def_phi ? "phi" :
                (cr->_var._def_stmt ? "stmt" : "null"));
      break;
    case CK_IVAR:
      fprintf(f, "off=%d ty=%d base=\n", cr->_ivar._offset, (int)cr->_ivar._ty);
      print_cr_pretty(cr->_ivar._base, depth + 1, f);
      break;
    case CK_OP:
      fprintf(f, "%s kids=%d\n", OPCODE_name(cr->_op._opc), cr->_op._num_kids);
      for (UINT8 i = 0; i < cr->_op._num_kids; ++i) {
        print_cr_pretty(cr->_op._kids[i], depth + 1, f);
      }
      break;
    default:
      fprintf(f, "?\n"); break;
  }
}

void CODEREP::Print_pretty(FILE *f, INT32 depth) const {
  print_cr_pretty(this, depth, f);
}

void PHI_NODE::Print(FILE *f) const {
  if (!f) f = stderr;
  fprintf(f, "  phi aux=%u result=v%u  ops=(",
          (UINT32)_aux_id, _result_ver);
  for (UINT32 i = 0; i < _opnd_vers.size(); ++i) {
    if (i > 0) fprintf(f, ", ");
    fprintf(f, "v%u", _opnd_vers[i]);
  }
  fprintf(f, ")  live=%d dead=%d\n",
          (int)_live, (int)_dead);
}
