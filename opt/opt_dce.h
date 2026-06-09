#ifndef OCC_OPT_DCE_H
#define OCC_OPT_DCE_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"
#include "opt_walk.h"
#include <queue>

class DCE {
public:
  void Run(SSA_CFG *cfg, OPT_STAB *stab);
private:
  std::queue<STMTREP *>  _stmt_work;
  std::queue<PHI_NODE *> _phi_work;

  void Mark_root(SSA_CFG *cfg);
  void Mark_cr(CODEREP *cr);
  void Mark_stmt(STMTREP *s);
  void Mark_phi(PHI_NODE *phi);
  void Sweep(SSA_CFG *cfg);
};

#endif //OCC_OPT_DCE_H
