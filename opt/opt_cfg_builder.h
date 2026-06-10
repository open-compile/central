//
// ============================================================================
// CLAUDE-MARKER  STATUS: REWRITE-REFERENCE (算法会重写)
// ============================================================================
// 文件作用: CFG_FROM_IR  — 从 IR TREE 构建 SSABB + STMTREP
// 关键函数:
//   - Build                          — 主入口
//   - Build_function_body            — 处理 BLOCK 内的每条语句
//   - Lower_expr / Lower_stmt        — 表达式 / 语句降级
//   - Fixup_branches                 — label → BB, 建立 pred/succ
// 已知问题 (重写时建议):
//   1. IR 形式: FUNC_ENTRY → BLOCK(empty) → BLOCK(stmts); 现在硬选 kids>0
//      的 BLOCK 是 hack, 建议前端调整 IR 让 FUNC_ENTRY 直接包 stmt BLOCK
//   2. Lower_expr 的 switch 覆盖不全, 大量 OPR_ 未处理
//   3. LOOP/EXC_SCOPE/REGION/CALL 等控制流节点未处理
//   4. End_bb() 之后的空 BB (没语句直接 RETURN) 仍会被创建
// ============================================================================

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
