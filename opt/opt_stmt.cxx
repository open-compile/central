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
  fprintf(f, "  stmt ");
  if (_lhs) {
    fprintf(f, "lhs=");
    _lhs->Print(f);
    fprintf(f, " = ");
  }
  if (_rhs) _rhs->Print(f);
  if (_cond) {
    fprintf(f, " cond=");
    _cond->Print(f);
  }
  fprintf(f, "\n");
}
