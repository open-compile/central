// ============================================================================
// CLAUDE-MARKER  STATUS: REWRITE-REFERENCE (算法会重写)
// 配套 opt_cfg_builder.h
// ============================================================================
#include "opt_cfg_builder.h"
#include "tree.h"
#include "symtab.h"

#include <stack>
#include <queue>

void CFG_FROM_IR::Build(TREE *tree, SSA_CFG *cfg, OPT_STAB *stab) {
  _cfg  = cfg;
  _stab = stab;
  _tree = tree;

  IR_ITER root = _tree->Get_root();
  if (root == _tree->End()) return;

  // 找到 FUNC_ENTRY 节点作为入口
  // 通常 root 是 BLOCK，里面是 FUNC_ENTRY，再嵌套 BLOCK
  // 实际 IR 形式: FUNC_ENTRY → BLOCK(empty) → BLOCK(statements...)
  // 所以找 FUNC_ENTRY 的 BLOCK 子节点中**有 kid 数量 > 0** 的那个
  IR_PRE_ITER pre(root);
  IR_PRE_ITER pre_end = _tree->End();
  bool found_func_entry = false;
  for (; pre != pre_end; ++pre) {
    IRNODE *n = _tree->Get_node(pre);
    if (n && OPCODE_operator(n->Opcode()) == OPR_FUNC_ENTRY) {
      IR_ITER fe = pre;
      for (UINT32 i = 0; i < _tree->Number_of_children(fe); ++i) {
        IR_ITER child = _tree->Get_operand(fe, i);
        IRNODE *cn = _tree->Get_node(child);
        if (cn && OPCODE_operator(cn->Opcode()) == OPR_BLOCK) {
          // 找有孩子的 BLOCK（避免选空的 BLOCK 包装）
          if (_tree->Number_of_children(child) > 0) {
            Build_function_body(child);
            found_func_entry = true;
            break;
          }
        }
      }
      // 若都没 kid，回退到第一个 BLOCK
      if (!found_func_entry) {
        for (UINT32 i = 0; i < _tree->Number_of_children(fe); ++i) {
          IR_ITER child = _tree->Get_operand(fe, i);
          IRNODE *cn = _tree->Get_node(child);
          if (cn && OPCODE_operator(cn->Opcode()) == OPR_BLOCK) {
            Build_function_body(child);
            found_func_entry = true;
            break;
          }
        }
      }
      break;
    }
  }
  if (!found_func_entry) {
    // 退化：把 root 当成 BLOCK 直接处理
    Build_function_body(root);
  }

  // 修复 goto 前驱后继
  Fixup_branches();
}

void CFG_FROM_IR::Build_function_body(IR_ITER block_iter) {
  // 创建 entry BB
  _cur_bb = Start_new_bb();
  // 标记 entry
  _cur_bb->Set_flag(BB_FLAG_ENTRY);

  // 对 BLOCK 的每个子节点（语句）逐个处理
  UINT32 n = _tree->Number_of_children(block_iter);
  for (UINT32 i = 0; i < n; ++i) {
    IR_ITER stmt_it = _tree->Get_operand(block_iter, i);
    IRNODE *node = _tree->Get_node(stmt_it);
    if (!node) continue;

    OPERATOR opr = OPCODE_operator(node->Opcode());

    // 处理 LABEL：先把 label 与当前 BB 关联
    if (opr == OPR_LABEL) {
      LABEL_IDX lbl = node->Get_label_num();
      if (_label_to_bb.find(lbl) == _label_to_bb.end()) {
        _label_to_bb[lbl] = _cur_bb;
        _cur_bb->Set_label_id(lbl);
        _cur_bb->Set_flag(BB_FLAG_LABEL);
      }
      continue;
    }

    // 处理跳转语句：先加入 STMTREP，再切 BB
    STMTREP *sr = Lower_stmt(stmt_it);
    if (!sr) continue;
    _cur_bb->Add_stmtrep(sr);

    BOOL is_branch =
        opr == OPR_GOTO || opr == OPR_TRUEBR || opr == OPR_FALSEBR ||
        opr == OPR_RETURN || opr == OPR_RETURN_VAL ||
        opr == OPR_CASEGOTO || opr == OPR_AGOTO || opr == OPR_XGOTO ||
        opr == OPR_COMPGOTO;

    if (opr == OPR_GOTO) {
      LABEL_IDX lbl = node->Get_label_num();
      Record_goto(_cur_bb, lbl);
      End_bb();
    } else if (opr == OPR_TRUEBR || opr == OPR_FALSEBR) {
      // 条件跳转：true 落入 fall-through，false/true 跳到 label
      LABEL_IDX lbl = node->Get_label_num();
      // 记录跳转目标（前驱 = 当前 bb）
      Record_goto(_cur_bb, lbl);
      // 当前 BB 结束，新开一个 fall-through BB
      End_bb();
      // fall-through 不会再有 label，但是它的后继由 Fixup_branches 在 label 修复时确立
    } else if (opr == OPR_RETURN || opr == OPR_RETURN_VAL) {
      _cur_bb->Set_flag(BB_FLAG_EXIT);
      End_bb();
    } else if (is_branch) {
      // 其它分支类型暂作返回处理
      End_bb();
    }
  }
}

SSABB *CFG_FROM_IR::Start_new_bb(SSABB *after) {
  SSABB *bb = nullptr;
  if (after == nullptr) {
    // 创建一个新 BB（追加到 cfg 末尾）
    UINT32 idx = (UINT32)_cfg->Size();
    // 直接 push_back 一个新的 SSABB
    // SSA_CFG_BB_BASE 通过 _bb_list 维护
    // 我们用 cfg->Add_bb() 拿不到指针，所以手动 push
    bb = new SSABB(idx);
    _cfg->Internal_bb_list().push_back(bb);
    _cfg->Internal_edges().emplace_back();
  } else {
    UINT32 idx = (UINT32)_cfg->Size();
    bb = new SSABB(idx);
    _cfg->Internal_bb_list().push_back(bb);
    _cfg->Internal_edges().emplace_back();
  }
  return bb;
}

void CFG_FROM_IR::End_bb() {
  _cur_bb = Start_new_bb(nullptr);
}

void CFG_FROM_IR::Fixup_branches() {
  // 把每个 _bb_goto_label 关联的 BB 与目标 label BB 建立 succ/pred
  for (size_t i = 0; i < _bb_goto_label.size(); ++i) {
    SSABB *from = _bb_goto_label[i];
    LABEL_IDX lbl = _pending_goto_label[i];
    auto it = _label_to_bb.find(lbl);
    if (it == _label_to_bb.end()) {
      // label 不存在：忽略（可能前端已处理）
      continue;
    }
    SSABB *to = it->second;
    from->Add_succ(to);
    to->Add_pred(from);
  }
}

CODEREP *CFG_FROM_IR::Lower_expr(IRNODE *irn) {
  if (!irn) return nullptr;
  OPERATOR opr = OPCODE_operator(irn->Opcode());
  MTYPE_ID res_type = OPCODE_rtype(irn->Opcode());

  switch (opr) {
    case OPR_INTCONST: {
      INT64 v = (INT64)irn->Get_const_val();
      return CODEREP::Make_const(v, res_type);
    }
    case OPR_CONST: {
      // 通用常数：先当 int64 处理
      INT64 v = (INT64)irn->Get_const_val();
      return CODEREP::Make_const(v, res_type);
    }
    case OPR_LDA: {
      return CODEREP::Make_lda(irn->Get_symbol_idx(), irn->Get_load_offset(), res_type);
    }
    case OPR_LDID: {
      ST_IDX st = irn->Get_symbol_idx();
      AUX_ID aux = _stab->Add_var(st, res_type);
      return CODEREP::Make_var(aux, 0, res_type);
    }
    case OPR_ILOAD: {
      // 孩子 0 是地址表达式
      CODEREP *base = nullptr;
      UINT32 n = irn->extra3.kids[0];  // 注意：这是 IRNODE_IDX
      IRNODE *base_n = _tree->Get_node(n);
      if (base_n) base = Lower_expr(base_n);
      return CODEREP::Make_ivar(base, irn->Get_load_offset(), irn->Get_type_idx(), res_type);
    }
    case OPR_ADD: case OPR_SUB: case OPR_MPY: case OPR_DIV: case OPR_MOD:
    case OPR_BAND: case OPR_BIOR: case OPR_BXOR: case OPR_BNOR:
    case OPR_LAND: case OPR_LIOR:
    case OPR_ASHR: case OPR_LSHR: case OPR_SHL:
    case OPR_EQ:  case OPR_NE:  case OPR_LT:  case OPR_LE:  case OPR_GT: case OPR_GE: {
      // 二元
      UINT32 k0 = irn->extra3.kids[0];
      UINT32 k1 = irn->extra3.kids[1];
      CODEREP *c0 = Lower_expr(_tree->Get_node(k0));
      CODEREP *c1 = Lower_expr(_tree->Get_node(k1));
      return CODEREP::Make_op(irn->Opcode(), c0, c1, res_type);
    }
    case OPR_NEG: case OPR_BNOT: case OPR_LNOT: case OPR_ABS:
    case OPR_CVT: case OPR_CVTL: case OPR_TRUNC: case OPR_RND: case OPR_CEIL: case OPR_FLOOR: {
      // 一元
      UINT32 k0 = irn->extra3.kids[0];
      CODEREP *c0 = Lower_expr(_tree->Get_node(k0));
      return CODEREP::Make_op(irn->Opcode(), c0, nullptr, res_type);
    }
    default: {
      // 不识别：当常数 0 占位
      return CODEREP::Make_const(0, MTYPE_UNKNOWN);
    }
  }
}

STMTREP *CFG_FROM_IR::Lower_stmt(IR_ITER stmt_it) {
  IRNODE *n = _tree->Get_node(stmt_it);
  if (!n) return nullptr;
  OPERATOR opr = OPCODE_operator(n->Opcode());

  STMTREP *s = Cr_pool()->Alloc_stmtrep();
  s->_opc = n->Opcode();
  s->_orig = *stmt_it;
  s->_bb = _cur_bb;

  switch (opr) {
    case OPR_STID: {
      ST_IDX st = n->Get_symbol_idx();
      MTYPE_ID res_type = OPCODE_rtype(n->Opcode());
      AUX_ID aux = _stab->Add_var(st, res_type);
      s->_lhs = CODEREP::Make_var(aux, 0, res_type);
      UINT32 k0 = n->extra3.kids[0];
      IRNODE *rhs_n = _tree->Get_node(k0);
      s->_rhs = Lower_expr(rhs_n);
      break;
    }
    case OPR_ISTORE: {
      // lhs: CK_IVAR（base + offset），rhs: 要存的值
      UINT32 k0 = n->extra3.kids[0];   // addr
      UINT32 k1 = n->extra3.kids[1];   // value
      IRNODE *base_n = _tree->Get_node(k0);
      CODEREP *base = Lower_expr(base_n);
      s->_lhs = CODEREP::Make_ivar(base, n->Get_load_offset(), n->Get_type_idx(),
                                    OPCODE_rtype(n->Opcode()));
      s->_rhs = Lower_expr(_tree->Get_node(k1));
      break;
    }
    case OPR_TRUEBR: case OPR_FALSEBR: {
      UINT32 k0 = n->extra3.kids[0];
      s->_cond = Lower_expr(_tree->Get_node(k0));
      s->_lhs = nullptr;
      s->_rhs = nullptr;
      break;
    }
    case OPR_GOTO: case OPR_RETURN: case OPR_RETURN_VAL: case OPR_LABEL:
    case OPR_FUNC_ENTRY: case OPR_BLOCK:
    case OPR_CALL: case OPR_ICALL: case OPR_PICCALL: case OPR_INTRINSIC_CALL:
    case OPR_VFCALL:
    case OPR_CASEGOTO: case OPR_AGOTO: case OPR_XGOTO: case OPR_COMPGOTO:
    case OPR_PRAGMA: case OPR_COMMENT:
    case OPR_REGION: case OPR_REGION_EXIT:
    case OPR_EXC_SCOPE_BEGIN: case OPR_EXC_SCOPE_END:
    case OPR_IO: case OPR_IO_ITEM:
    case OPR_ASM_STMT: {
      // 控制流/不透明的语句：lhs/rhs 都空，但保留 side-effect 语义
      s->_lhs = nullptr;
      s->_rhs = nullptr;
      break;
    }
    default: {
      // 表达式语句（少见）：只降级 rhs
      UINT32 k0 = n->extra3.kids[0];
      s->_rhs = Lower_expr(_tree->Get_node(k0));
      break;
    }
  }
  return s;
}
