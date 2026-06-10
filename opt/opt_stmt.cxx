// ============================================================================
// CLAUDE-MARKER  STATUS: KEEP (data structure)
// 配套 opt_stmt.h; 实现 STMTREP::Print / CR_POOL::Alloc_stmtrep
// ============================================================================
#include "opt_stmt.h"
#include "opt_basic.h"
#include "opt_coderep.h"

STMTREP *CR_POOL::Alloc_stmtrep() {
  STMTREP *s = new STMTREP();
  _stmts.push_back(s);
  return s;
}

BOOL STMTREP::Has_side_effect() const {
  OPERATOR opr = OPCODE_operator(_opc);
  switch (opr) {
    case OPR_CALL:
    case OPR_ICALL:
    case OPR_INTRINSIC_CALL:
    case OPR_PICCALL:
    case OPR_VFCALL:
    case OPR_ISTORE:
    case OPR_ISTOREX:
    case OPR_MSTORE:
    case OPR_STID:
    case OPR_STBITS:
    case OPR_RETURN:
    case OPR_RETURN_VAL:
    case OPR_GOTO:
    case OPR_TRUEBR:
    case OPR_FALSEBR:
    case OPR_CASEGOTO:
    case OPR_AGOTO:
    case OPR_COMPGOTO:
    case OPR_XGOTO:
    case OPR_FUNC_ENTRY:
    case OPR_IO:
    case OPR_IO_ITEM:
    case OPR_REGION:
    case OPR_REGION_EXIT:
      return TRUE;
    default:
      return FALSE;
  }
}

void STMTREP::Print(FILE *f) const {
  if (!f) f = stderr;
  fprintf(f, "  stmt[bb=%d opc=%s live=%d] ",
          _bb ? (INT32)_bb->Get_id() : -1,
          OPCODE_name(_opc), (INT32)_live);
  if (_lhs) {
    fprintf(f, "lhs=");
    _lhs->Print(f);
    fprintf(f, " = ");
  } else {
    fprintf(f, "  ");
  }
  if (_rhs) _rhs->Print(f);
  else fprintf(f, "-");
  if (_cond) {
    fprintf(f, " | cond=");
    _cond->Print(f);
  }
  fprintf(f, "\n");
}

void STMTREP::Print_pretty(FILE *f) const {
  if (!f) f = stderr;
  fprintf(f, "  stmt opc=%s live=%d side_effect=%d\n",
          OPCODE_name(_opc), (INT32)_live, (INT32)Has_side_effect());
  if (_lhs) {
    fprintf(f, "    lhs:\n");
    _lhs->Print_pretty(f, 2);
  }
  if (_rhs) {
    fprintf(f, "    rhs:\n");
    _rhs->Print_pretty(f, 2);
  }
  if (_cond) {
    fprintf(f, "    cond:\n");
    _cond->Print_pretty(f, 2);
  }
}
