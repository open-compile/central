#include "opt_cprop.h"
#include "opt_dom.h"
#include <stack>
#include <queue>
#include <unordered_map>

CODEREP *CONST_PROP::Fold(CODEREP *cr) {
  if (!cr) return nullptr;
  if (cr->Is_const() || cr->Is_lda()) return cr;
  if (cr->Is_var()) {
    if (cr->Version() == 0) return cr;
    STMTREP *def = cr->Def_stmt();
    if (def && def->_rhs) {
      CODEREP *folded = Fold(def->_rhs);
      if (folded != def->_rhs) def->_rhs = folded;
      if (folded->Is_const()) return folded;
    }
    PHI_NODE *phi = cr->Def_phi();
    if (phi) {
      // 全部操作数同值则折叠
      CODEREP *first = nullptr;
      for (UINT32 i = 0; i < phi->_opnd_vers.size(); ++i) {
        UINT32 v = phi->_opnd_vers[i];
        if (v == 0) return cr;  // 有未版本化的操作数，不能折叠
        // 从 stab 找 def 再 fold
        // 简化：直接返回 cr（phi 折叠在 phi-pass 单独做）
      }
      // 简化：先不做跨 phi 的折叠，留作扩展
    }
    return cr;
  }
  if (cr->Is_op()) {
    CODEREP *k0 = Fold(cr->Opnd(0));
    if (k0 != cr->Opnd(0)) cr->_op._kids[0] = k0;
    if (cr->Num_kids() == 2) {
      CODEREP *k1 = Fold(cr->Opnd(1));
      if (k1 != cr->Opnd(1)) cr->_op._kids[1] = k1;
      if (k0->Is_const() && k1->Is_const()) {
        return CODEREP::Make_const(Eval_bin(cr->Op(), k0->Const_val(), k1->Const_val()), cr->_mtype);
      }
    } else {
      if (k0->Is_const()) {
        return CODEREP::Make_const(Eval_un(cr->Op(), k0->Const_val()), cr->_mtype);
      }
    }
    return cr;
  }
  if (cr->Is_ivar()) {
    if (cr->_ivar._base) {
      CODEREP *nb = Fold(cr->_ivar._base);
      if (nb != cr->_ivar._base) cr->_ivar._base = nb;
    }
    return cr;
  }
  return cr;
}

INT64 CONST_PROP::Eval_bin(OPCODE opc, INT64 a, INT64 b) {
  OPERATOR opr = OPCODE_operator(opc);
  switch (opr) {
    case OPR_ADD: return a + b;
    case OPR_SUB: return a - b;
    case OPR_MPY: return a * b;
    case OPR_DIV: return b == 0 ? 0 : a / b;
    case OPR_MOD: return b == 0 ? 0 : a % b;
    case OPR_BAND: return a & b;
    case OPR_BIOR: return a | b;
    case OPR_BXOR: return a ^ b;
    case OPR_ASHR: return (INT64)a >> (INT64)b;
    case OPR_LSHR: return (UINT64)a >> (UINT64)b;
    case OPR_SHL:  return (UINT64)a << (UINT64)b;
    case OPR_EQ:   return a == b;
    case OPR_NE:   return a != b;
    case OPR_LT:   return a <  b;
    case OPR_LE:   return a <= b;
    case OPR_GT:   return a >  b;
    case OPR_GE:   return a >= b;
    case OPR_LAND: return a && b;
    case OPR_LIOR: return a || b;
    default:       return 0;
  }
}

INT64 CONST_PROP::Eval_un(OPCODE opc, INT64 a) {
  OPERATOR opr = OPCODE_operator(opc);
  switch (opr) {
    case OPR_NEG:  return -a;
    case OPR_BNOT: return ~a;
    case OPR_LNOT: return !a;
    case OPR_ABS:  return a < 0 ? -a : a;
    default:       return 0;
  }
}

BOOL CONST_PROP::Run(SSA_CFG *cfg, OPT_STAB *stab) {
  (void)stab;
  if (cfg->Size() == 0) return FALSE;
  changed = FALSE;
  // RPO 顺序遍历
  std::vector<SSABB *> rpo;
  {
    DOM_BUILDER d;
    d.Compute_rpo(cfg, rpo);
  }
  for (auto *bb : rpo) {
    for (auto *s : bb->_stmtreps) {
      if (s->_rhs) {
        CODEREP *nf = Fold(s->_rhs);
        if (nf != s->_rhs) {
          s->_rhs = nf;
          changed = TRUE;
        }
      }
      if (s->_cond) {
        CODEREP *nf = Fold(s->_cond);
        if (nf != s->_cond) {
          s->_cond = nf;
          changed = TRUE;
        }
      }
    }
  }
  return changed;
}
