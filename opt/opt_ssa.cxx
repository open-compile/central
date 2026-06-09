#include "opt_ssa.h"
#include "opt_basic.h"
#include "opt_coderep.h"
#include "opt_stmt.h"
#include "opt_stab.h"
#include "opt_dom.h"
#include "opt_cfg_builder.h"
#include "opt_walk.h"
#include "be_export.h"

#include <stack>
#include <vector>
#include <queue>
#include <map>
#include <cstring>

// 全局 SSA 上下文（保持与现有 SSA_COMPOSITE 接口一致）
static SSA_COMPOSITE *_ssair_opt = nullptr;

SSA_COMPOSITE *SSAMon() {
  if (!_ssair_opt) _ssair_opt = new SSA_COMPOSITE();
  return _ssair_opt;
}

SSAIR::SSAIR() = default;
SSAIR::~SSAIR() {
  for (auto &p : _trees)   delete p.second;
  for (auto &p : _ssa_symtab) delete p.second;
  for (auto &p : _ssa_codemap) delete p.second;
}

SSA_CFG *SSAIR::Get_function(ST_IDX func_sym) {
  if (_trees.find(func_sym) == _trees.end()) {
    _trees[func_sym] = new SSA_CFG();
    _ssa_symtab[func_sym] = new OPT_STAB();
    _ssa_codemap[func_sym] = new CODE_STORE();
  }
  return _trees[func_sym];
}

void SSAIR::Goto_function(ST_IDX sym) {
  if (_trees.find(sym) == _trees.end()) {
    _trees[sym] = new SSA_CFG();
  }
  if (_ssa_symtab.find(sym) == _ssa_symtab.end()) {
    _ssa_symtab[sym] = new OPT_STAB();
  }
  _current = _trees[sym];
  _current_sym = sym;
  _current_symtab = _ssa_symtab[sym];
  // 从 FILE_MANAGER 取该函数的 TREE
  extern FILE_MANAGER *File();
  TREE *t = PU_INFO_pu_info(File()->Tables()->Get_pu_info_by_st_idx(sym))->entry;
  _current_tree = t;
}

void SSAIR::Print(FILE *file) {
  fprintf(file, "=== SSAIR: %lu functions ===\n", _trees.size());
  for (auto &p : _trees) {
    fprintf(file, "  function st=%d\n", (int)p.first);
  }
}

void SSAIR::Print(ST_IDX sym, FILE *file) {
  fprintf(file, "=== SSAIR function st=%d ===\n", (int)sym);
  SSA_CFG *cfg = Get_function(sym);
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = cfg->Node(i);
    fprintf(file, "BB %d (preds=%d, succs=%d):\n",
            bb->Get_id(), bb->Get_preds_count(), bb->Get_succs_count());
    for (auto *phi : bb->_phi_list) phi->Print(file);
    for (auto *s : bb->_stmtreps) s->Print(file);
  }
}

// ===== SSA_COMPOSITE 主流程 =====
// Init/SSA_ir/SSA_builder 已在 opt_basic.h 中 inline 定义

void SSA_COMPOSITE::Print(FILE *f) {
  if (_ssair) _ssair->Print(f);
}

// 收集每个变量的定义 BB
void SSA_COMPOSITE::Collect_defs_bb() {
  SSA_CFG *cfg = _ssair->Cfg();
  OPT_STAB *stab = _ssair->Current_symtab();
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = (SSABB *)cfg->Node(i);
    for (auto *s : bb->_stmtreps) {
      if (s->_lhs && s->_lhs->Is_var()) {
        AUX_ID aux = s->_lhs->Aux_id();
        stab->Var(aux).def_bbs.insert(bb);
      }
    }
  }
}

// Cytron phi 放置
void SSA_COMPOSITE::Place_phi_node() {
  SSA_CFG *cfg = _ssair->Cfg();
  OPT_STAB *stab = _ssair->Current_symtab();

  for (AUX_ID aux = 0; aux < (AUX_ID)stab->Num_vars(); ++aux) {
    if (stab->Var(aux).def_bbs.empty()) continue;
    std::set<SSABB *> placed;
    std::queue<SSABB *> worklist;
    for (auto *b : stab->Var(aux).def_bbs) worklist.push(b);

    while (!worklist.empty()) {
      SSABB *b = worklist.front();
      worklist.pop();
      for (auto it = b->Df_begin(); it != b->Df_end(); ++it) {
        SSABB *y = (SSABB *)(*it);
        if (placed.count(y) > 0) continue;
        placed.insert(y);
        // 插入 phi：操作数 = y 的前驱数
        PHI_NODE *phi = Cr_pool()->Alloc_phi();
        phi->_aux_id = aux;
        phi->_bb = y;
        phi->_opnd_vers.assign(y->Get_preds_count(), 0);
        phi->_opnd_crs.assign(y->Get_preds_count(), nullptr);
        y->Add_phi(phi);
        // 标记 phi 为此变量的 def（rename 阶段会建 CODEREP）
        if (stab->Var(aux).def_bbs.count(y) == 0) {
          worklist.push(y);
        }
      }
    }
  }
}

// 重命名（DFS 支配树）
struct RENAMER {
  OPT_STAB *stab;
  std::vector<std::vector<UINT32>>  stack;  // stack[aux] = 版本栈

  void Push(SSABB *root) {
    // 1. 处理 phi
    for (auto *phi : root->_phi_list) {
      AUX_ID aux = phi->_aux_id;
      UINT32 v = stab->New_version(aux);
      phi->_result_ver = v;
      VER_ENTRY &ve = stab->Ver(aux, v);
      ve.def_phi = phi;
      ve.def_bb  = root;
      // 建 CODEREP
      CODEREP *cr = CODEREP::Make_var(aux, v, stab->Var(aux).mtype);
      cr->Set_def_phi(phi);
      ve.coderep = cr;
      phi->_result_cr = cr;
      if (aux >= stack.size()) stack.resize(aux + 1);
      stack[aux].push_back(v);
    }

    // 2. 处理语句
    for (auto *s : root->_stmtreps) {
      // 先重命名 RHS / cond
      if (s->_rhs) Rename_uses(s->_rhs);
      if (s->_cond) Rename_uses(s->_cond);
      if (s->_lhs && s->_lhs->Is_ivar()) {
        Rename_uses(s->_lhs->_ivar._base);
      }
      // 再处理 lhs
      if (s->_lhs && s->_lhs->Is_var()) {
        AUX_ID aux = s->_lhs->Aux_id();
        UINT32 v = stab->New_version(aux);
        s->_lhs->_var._version = v;
        VER_ENTRY &ve = stab->Ver(aux, v);
        ve.def_stmt = s;
        ve.def_bb   = root;
        // 更新 CODEREP 的 def 指针
        s->_lhs->_var._def_stmt = s;
        s->_lhs->_var._def_phi  = nullptr;
        ve.coderep = s->_lhs;
        if (aux >= stack.size()) stack.resize(aux + 1);
        stack[aux].push_back(v);
      }
    }

    // 3. 填充后继 BB 的 phi 操作数
    for (auto sit = root->Succ_begin(); sit != root->Succ_end(); ++sit) {
      SSABB *succ = (SSABB *)(*sit);
      INT32 pos = succ->Pred_pos(root);
      if (pos < 0) continue;
      for (auto *phi : succ->_phi_list) {
        AUX_ID aux = phi->_aux_id;
        UINT32 v = 0;
        if (aux < stack.size() && !stack[aux].empty()) v = stack[aux].back();
        phi->_opnd_vers[pos] = v;
        // 关联 CODEREP
        if (v > 0) {
          VER_ENTRY &ve = stab->Ver(aux, v);
          CODEREP *cr = CODEREP::Make_var(aux, v, stab->Var(aux).mtype);
          if (ve.def_stmt) cr->Set_def_stmt(ve.def_stmt);
          else if (ve.def_phi) cr->Set_def_phi(ve.def_phi);
          phi->_opnd_crs[pos] = cr;
        }
      }
    }

    // 4. DFS 支配树子节点
    for (auto it = root->Dom_begin(); it != root->Dom_end(); ++it) {
      Push((SSABB *)(*it));
    }

    // 5. 退出时弹栈
    for (auto *phi : root->_phi_list) {
      AUX_ID aux = phi->_aux_id;
      if (aux < stack.size() && !stack[aux].empty()) stack[aux].pop_back();
    }
    for (auto *s : root->_stmtreps) {
      if (s->_lhs && s->_lhs->Is_var()) {
        AUX_ID aux = s->_lhs->Aux_id();
        if (aux < stack.size() && !stack[aux].empty()) stack[aux].pop_back();
      }
    }
  }

  void Rename_uses(CODEREP *cr) {
    if (!cr) return;
    struct V {
      RENAMER *r;
      void Visit_cr(CODEREP *c) {
        if (c->Is_var() && c->Version() == 0) {
          // 第一次见到：查栈顶
          AUX_ID aux = c->Aux_id();
          if (aux < r->stack.size() && !r->stack[aux].empty()) {
            UINT32 v = r->stack[aux].back();
            c->_var._version = v;
            VER_ENTRY &ve = r->stab->Ver(aux, v);
            if (ve.def_stmt) c->Set_def_stmt(ve.def_stmt);
            else if (ve.def_phi) c->Set_def_phi(ve.def_phi);
          }
        }
      }
    } vis{this};
    Walk_cr_postorder(cr, vis);
  }
};

void SSA_COMPOSITE::SSA_Rename() {
  SSA_CFG *cfg = _ssair->Cfg();
  if (cfg->Size() == 0) return;
  RENAMER r;
  r.stab = _ssair->Current_symtab();
  // 入口是 BB 0
  r.Push((SSABB *)cfg->Node(0));
}

BOOL SSA_COMPOSITE::Verify_stack(OPT_SYMTAB *symtab) {
  return TRUE;
}

void SSA_COMPOSITE::SSA_Rename_rhs() {
  // 当前实现中，RHS 已经在 SSA_Rename 内联处理，留作扩展点
}

void SSA_COMPOSITE::Construct_function(ST_IDX func_sym, SCOPE *scope, UINT32 ir_lvl) {
  Init();
  SSA_ir()->Goto_function(func_sym);

  SSA_CFG    *cfg     = SSA_ir()->Cfg();
  OPT_STAB   *stab    = SSA_ir()->Current_symtab();
  TREE       *tree    = SSA_ir()->Current_tree();

  // 1. IR → SSABB + STMTREP
  CFG_FROM_IR builder;
  builder.Build(tree, cfg, stab);

  // 2. 支配树
  DOM_BUILDER dom;
  dom.Build(cfg);

  // 3. 收集 def bb
  Collect_defs_bb();

  // 4. 放置 phi
  Place_phi_node();

  // 5. 重命名
  SSA_Rename();
}

// ===== 外层接口 =====

void Opt_build_ssa(PU_INFO *pu, FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  (void)file; (void)config;
  SSAMon()->Init();
  SSAMon()->SSA_ir()->Goto_function(pu->Proc_sym());
  SSAMon()->Construct_function(pu->Proc_sym(), &pu->scope, (UINT32)level);
}

void Opt_destruct_ssa(PU_INFO *pu_info, FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  (void)level; (void)config;
  SSA_CFG *cfg = SSAMon()->SSA_ir()->Get_function(pu_info->Proc_sym());
  if (!cfg) return;

  // 把每个 phi 转成 copy：每个前驱在末尾追加 STID
  for (UINT32 i = 0; i < cfg->Size(); ++i) {
    SSABB *bb = (SSABB *)cfg->Node(i);
    if (bb->_phi_list.empty()) continue;
    // 用 OPT_STAB 把 aux_id 翻译回 ST_IDX
    OPT_STAB *stab = SSAMon()->SSA_ir()->Symtab_map()[pu_info->Proc_sym()];
    for (auto *phi : bb->_phi_list) {
      AUX_ID aux = phi->_aux_id;
      ST_IDX st = stab->Var(aux).st_idx;
      MTYPE_ID mtype = stab->Var(aux).mtype;
      INT32 pred_idx = 0;
      for (auto pit = bb->Pred_begin(); pit != bb->Pred_end(); ++pit, ++pred_idx) {
        SSABB *pred = (SSABB *)(*pit);
        STMTREP *copy = Cr_pool()->Alloc_stmtrep();
        // 构造一个 STID 语句的 OPCODE
        // 简化：直接用 OPC_I4STID 模式
        OPCODE opc_stid = (OPCODE)(OPR_STID | (0 << 8) | (mtype << 14));
        copy->_opc = opc_stid;
        copy->_bb = pred;
        // lhs = var(aux, result_ver)
        copy->_lhs = CODEREP::Make_var(aux, phi->_result_ver, mtype);
        // rhs = var(aux, opnd_vers[pred_idx])，若为 0 则置 0
        UINT32 opv = phi->_opnd_vers[pred_idx];
        if (opv == 0) {
          copy->_rhs = CODEREP::Make_const(0, mtype);
        } else {
          copy->_rhs = CODEREP::Make_var(aux, opv, mtype);
        }
        // 插入到 pred 的 stmtrep 末尾
        pred->_stmtreps.push_back(copy);
      }
    }
    bb->_phi_list.clear();
  }
}

void Opt_build_ssa_all(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  SSAMon()->Init();
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    PU_INFO *pu = file->Tables()->Pu_info()->Get(it);
    if (pu->proc_sym != 0) {
      Opt_build_ssa(pu, file, level, config);
    }
  }
}

void Opt_destruct_ssa_all(FILE_MANAGER *file, IR_LEVEL level, COMPILER_CONFIG &config) {
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    PU_INFO *pu = file->Tables()->Pu_info()->Get(it);
    if (pu->proc_sym != 0) {
      Opt_destruct_ssa(pu, file, level, config);
    }
  }
}

// ===== Pass 入口 =====

#include "opt_dce.h"
#include "opt_cprop.h"

void Opt_run_dce(FILE_MANAGER *file, COMPILER_CONFIG &config) {
  (void)config;
  SSAIR *ssair = SSAMon()->SSA_ir();
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    PU_INFO *pu = file->Tables()->Pu_info()->Get(it);
    if (pu->proc_sym == 0) continue;
    SSA_CFG *cfg = ssair->Get_function(pu->proc_sym);
    if (!cfg || cfg->Size() == 0) continue;
    OPT_STAB *stab = ssair->Symtab_map()[pu->proc_sym];
    DCE dce;
    dce.Run(cfg, stab);
    Is_Trace(Tracing(COMPONENT_SSA, TRACE_INFO),
             (TFile, "[DCE] completed on pu st=%d\n", (int)pu->proc_sym));
  }
}

void Opt_run_cprop(FILE_MANAGER *file, COMPILER_CONFIG &config) {
  (void)config;
  SSAIR *ssair = SSAMon()->SSA_ir();
  for (UINT32 it = 1; it < file->Tables()->Pu_info()->Length(); it++) {
    PU_INFO *pu = file->Tables()->Pu_info()->Get(it);
    if (pu->proc_sym == 0) continue;
    SSA_CFG *cfg = ssair->Get_function(pu->proc_sym);
    if (!cfg || cfg->Size() == 0) continue;
    OPT_STAB *stab = ssair->Symtab_map()[pu->proc_sym];
    CONST_PROP cp;
    // 不动点迭代：通常 1-2 轮
    for (int round = 0; round < 4; ++round) {
      if (!cp.Run(cfg, stab)) break;
    }
    Is_Trace(Tracing(COMPONENT_SSA, TRACE_INFO),
             (TFile, "[CPROP] completed on pu st=%d\n", (int)pu->proc_sym));
  }
}
