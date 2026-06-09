#ifndef OCC_OPT_CFG_BUILDER_H
#define OCC_OPT_CFG_BUILDER_H

#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"

class TREE;

class CFG_FROM_IR {
public:
  // 从 PU 的 IR 树构建 SSA CFG
  // 返回: cfg 中所有 BB 和 STMTREP 填充完毕
  //       stab 中所有 STID 对应的变量已注册（不带版本号）
  //       PHI 未插入，SSA 未建立
  void Build(TREE *tree, SSA_CFG *cfg, OPT_STAB *stab);

private:
  SSA_CFG    *_cfg;
  OPT_STAB   *_stab;
  TREE       *_tree;
  SSABB      *_cur_bb;
  std::map<LABEL_IDX, SSABB *> _label_to_bb;
  std::vector<SSABB *>         _bb_goto_label;  // 待修复：goto BB -> target label

  // 子函数
  void   Build_function_body(IR_ITER block_iter);
  SSABB *Start_new_bb(SSABB *after = nullptr);
  void   End_bb();
  void   Fixup_branches();

  // IR 节点降级
  CODEREP  *Lower_expr(IRNODE *irn);
  STMTREP  *Lower_stmt(IR_ITER stmt_it);

  // 工具
  SSABB *Get_or_create_label_bb(LABEL_IDX lbl);
  void   Record_goto(SSABB *from, LABEL_IDX lbl) { _bb_goto_label.push_back(from); _pending_goto_label.push_back(lbl); }
  std::vector<LABEL_IDX> _pending_goto_label;
};

#endif //OCC_OPT_CFG_BUILDER_H
