#ifndef OCC_OPT_CPROP_H
#define OCC_OPT_CPROP_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"
#include "opt_walk.h"

class CONST_PROP {
public:
  // 在 SSA 上做常量传播 + 折叠
  // 返回是否发生了变化
  BOOL Run(SSA_CFG *cfg, OPT_STAB *stab);

private:
  CODEREP *Fold(CODEREP *cr);
  CODEREP *Fold_rhs(CODEREP *cr);
  INT64    Eval_bin(OPCODE opc, INT64 a, INT64 b);
  INT64    Eval_un(OPCODE opc, INT64 a);
  BOOL changed;
};

#endif //OCC_OPT_CPROP_H
