#ifndef OCC_OPT_STMT_H
#define OCC_OPT_STMT_H

#include "basic.h"
#include "consts.h"
#include "tree.h"
#include "opt_coderep.h"

class CODEREP;
class PHI_NODE;

class STMTREP {
public:
  OPCODE    _opc;          // 对应 IR 语句的 opcode
  CODEREP  *_lhs;          // 赋值左侧；STID → CK_VAR，ISTORE → CK_IVAR
  CODEREP  *_rhs;          // 右侧表达式（可能为 nullptr）
  IRNODE_IDX _orig;        // 回指原 IRNODE
  SSABB    *_bb;
  BOOL      _live;         // DCE 标记

  // 条件跳转等还需要单独的 cond 操作数（不是 _rhs）
  CODEREP  *_cond;

  STMTREP()
    : _opc((OPCODE)0),
      _lhs(nullptr), _rhs(nullptr), _orig(0), _bb(nullptr),
      _live(FALSE), _cond(nullptr) {}

  BOOL Has_side_effect() const;
  void Print(FILE *f = stderr) const;
};

#endif //OCC_OPT_STMT_H
