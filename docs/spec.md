# SSA 设计与优化 Spec

> 版本：v1.0  
> **CLAUDE-MARKER**: 本 spec 由 claude 撰写, 描述 SSA / 基础优化的算法与数据结构设计。
>               重写算法时可保留数据结构部分, 算法部分应按本 spec + 经典论文
>               (Cytron'91 / Cooper'01 / Wegman-Zadeck'85) 重新实现。
> 参考：Open64 opt_htable/opt_ssa/opt_dce/opt_prop  
> 目标：在现有 IRNODE/TREE 之上建立 CODEREP-based SSA，支持 DCE 和常量传播

---

## 一、设计原则

1. **SSA 是 IR 之上的独立层**：不修改现有 `IRNODE`/`TREE`，SSA 层（`CODEREP` / `STMTREP`）并行建立，与原始 IR 通过指针关联。
2. **模板化遍历，算法逻辑与结构解耦**：所有 CFG / 表达式树 / 支配树遍历用 `Visitor` 模板实现，DCE、常量传播等 pass 只需实现 `Visit` 方法。
3. **最小化新数据结构**：复用 `CFG_BB_BASE` / `CFG_BASE` 的模板骨架；`CODEREP` 用 `union` 分字段，紧凑内存布局。
4. **单向依赖**：`opt/` 依赖 `common/`，`cg/` 不依赖 `opt/`。

---

## 二、新增数据结构

### 2.1 `CODEREP`（`opt/opt_coderep.h`）

SSA 表达式节点，对应一个 IRNODE 子树中的一个"值"。

```cpp
enum CR_KIND : UINT8 {
  CK_CONST  = 1,  // 整数常数
  CK_LDA    = 2,  // 变量地址（不参与 SSA 版本）
  CK_VAR    = 3,  // 已命名变量，有 SSA 版本
  CK_IVAR   = 4,  // 间接加载（*ptr + offset）
  CK_OP     = 5,  // 操作符节点（binary / unary）
};

class CODEREP {
public:
  CR_KIND   _kind;
  MTYPE_ID  _mtype;   // 值类型（INT32 / INT64 / …）
  UINT32    _id;      // 全局唯一 id（便于 debug / hash）

  union {
    // CK_CONST
    INT64     _const_val;

    // CK_LDA
    struct { ST_IDX _lda_sym; INT32 _lda_offset; } _lda;

    // CK_VAR：versioned variable
    struct {
      AUX_ID    _aux_id;    // OPT_STAB 中的紧凑变量 id
      UINT32    _version;   // SSA 版本号（0 = 未初始化）
      // 指向定义此版本的语句或 phi（互斥）
      STMTREP  *_def_stmt;  // 由普通语句定义
      PHI_NODE *_def_phi;   // 由 phi 定义
    } _var;

    // CK_IVAR：间接加载
    struct {
      CODEREP  *_base;      // 地址表达式
      INT32     _offset;
      TY_IDX    _ty;
    } _ivar;

    // CK_OP：操作符
    struct {
      OPCODE    _opc;
      UINT8     _num_kids;
      CODEREP  *_kids[2];   // 最多两个操作数（一元/二元）
    } _op;
  };

  // 工厂方法（在 .cxx 中实现）
  static CODEREP *Make_const(INT64 val, MTYPE_ID mtype);
  static CODEREP *Make_lda(ST_IDX sym, INT32 offset);
  static CODEREP *Make_var(AUX_ID aux, UINT32 ver, MTYPE_ID mtype);
  static CODEREP *Make_ivar(CODEREP *base, INT32 off, TY_IDX ty, MTYPE_ID mtype);
  static CODEREP *Make_op(OPCODE opc, CODEREP *k0, CODEREP *k1, MTYPE_ID mtype);

  // 类型判断
  BOOL Is_const()  const { return _kind == CK_CONST; }
  BOOL Is_var()    const { return _kind == CK_VAR;   }
  BOOL Is_lda()    const { return _kind == CK_LDA;   }
  BOOL Is_op()     const { return _kind == CK_OP;    }

  // 访问器
  INT64      Const_val()    const;
  AUX_ID     Aux_id()       const;
  UINT32     Version()      const;
  STMTREP   *Def_stmt()     const;
  PHI_NODE  *Def_phi()      const;
  void       Set_def_stmt(STMTREP *s);
  void       Set_def_phi(PHI_NODE *p);

  void Print(FILE *f = stderr) const;
};
```

**内存管理**：用 `CODEREP_POOL`（`std::deque<CODEREP>`）按函数分配，函数结束整体释放，不逐个 `delete`。

---

### 2.2 `PHI_NODE`（`opt/opt_coderep.h`）

```cpp
class PHI_NODE {
public:
  AUX_ID   _aux_id;      // 对应变量
  UINT32   _result_ver;  // 此 phi 定义的结果版本
  SSABB   *_bb;          // 所在 BB

  // 操作数：每个前驱 BB 对应一个版本
  // 顺序与 _bb->_preds 的顺序一致
  vector<UINT32> _opnd_vers;

  // CODEREP 指针（rename 阶段后填充，供后续 pass 使用）
  CODEREP        *_result_cr = nullptr;
  vector<CODEREP *> _opnd_crs;

  // 标志
  BOOL  _live   = FALSE;  // DCE 用
  BOOL  _dead   = FALSE;  // 被 DCE/CP 标记为死
};
```

---

### 2.3 `STMTREP`（`opt/opt_stmt.h` 扩展）

每个原始 IRNODE 语句对应一个 `STMTREP`，存在于 `SSABB::_stmtreps` 中。

```cpp
class STMTREP {
public:
  OPCODE     _opc;          // 对应 IR 语句的 opcode
  CODEREP   *_lhs = nullptr; // 赋值左侧（STID → CK_VAR；ISTORE → CK_IVAR）
  CODEREP   *_rhs = nullptr; // 右侧表达式树
  IRNODE_IDX _orig;          // 回指原 IRNODE，用于最终代码发射
  SSABB     *_bb  = nullptr;
  BOOL       _live = FALSE;  // DCE 标记

  BOOL Has_side_effect() const;  // CALL / ISTORE / RETURN / 分支 = TRUE
  void Print(FILE *f = stderr) const;
};
```

---

### 2.4 `OPT_STAB`（`opt/opt_stab.h`）

将原始 `ST_IDX` 映射为紧凑的 `AUX_ID`，并维护每个变量的版本信息。

```cpp
// 每个变量在优化器中的信息
struct OPT_VAR {
  ST_IDX   st_idx;
  AUX_ID   aux_id;
  MTYPE_ID mtype;
  UINT32   ver_count = 0;  // 已分配版本数
  // 定义所在 BB 集合，用于 phi 放置
  set<SSABB *> def_bbs;
};

// 每个 <AUX_ID, version> 的信息
struct VER_ENTRY {
  AUX_ID    aux_id;
  UINT32    version;
  STMTREP  *def_stmt = nullptr;
  PHI_NODE *def_phi  = nullptr;
  SSABB    *def_bb   = nullptr;
  CODEREP  *coderep  = nullptr;  // rename 阶段填充
};

class OPT_STAB {
public:
  AUX_ID   Add_var(ST_IDX st);
  AUX_ID   Lookup(ST_IDX st) const;
  UINT32   New_version(AUX_ID aux);
  VER_ENTRY &Ver(AUX_ID aux, UINT32 ver);
  OPT_VAR  &Var(AUX_ID aux);
  UINT32    Num_vars() const;

private:
  vector<OPT_VAR>              _vars;        // indexed by AUX_ID
  map<ST_IDX, AUX_ID>          _st_to_aux;
  // ver_tab[aux_id][version] → VER_ENTRY
  vector<vector<VER_ENTRY>>    _ver_tab;
};
```

---

### 2.5 扩展 `SSABB`（`opt/opt_basic.h`）

在现有 `SSA_CFG_BB_BASE` 基础上添加：

```cpp
template <typename NODE_TYPE>
class SSA_CFG_BB_BASE : public CFG_BB_BASE<NODE_TYPE> {
public:
  // PHI 列表（此 BB 头部的所有 phi 函数）
  vector<PHI_NODE *>   _phi_list;

  // STMTREP 列表（替代原来的 _stmts）
  vector<STMTREP *>    _stmtreps;

  void Add_phi(PHI_NODE *phi) { _phi_list.push_back(phi); }
  void Add_stmtrep(STMTREP *s) { _stmtreps.push_back(s); }
};
```

---

## 三、遍历模板（`opt/opt_walk.h`）

所有 pass 使用统一的遍历模板，算法只实现 `Visit` 方法。

### 3.1 CFG 遍历

```cpp
// RPO（逆后序）遍历 CFG，对每个 BB 调用 Visitor::Visit_bb
template <typename CFG_TYPE, typename Visitor>
void Walk_cfg_rpo(CFG_TYPE *cfg, Visitor &vis) {
  // 计算 RPO 顺序
  vector<typename CFG_TYPE::BB_TYPE *> rpo;
  Compute_rpo(cfg, rpo);
  for (auto *bb : rpo)
    vis.Visit_bb(bb);
}

// 支配树 DFS 遍历
template <typename BB_TYPE, typename Visitor>
void Walk_dom_tree(BB_TYPE *root, Visitor &vis) {
  vis.Visit_bb(root);
  for (auto it = root->Dom_begin(); it != root->Dom_end(); ++it)
    Walk_dom_tree(*it, vis);
}
```

### 3.2 表达式树遍历

```cpp
// 后序遍历 CODEREP 树，对每个节点调用 Visitor::Visit_cr
template <typename Visitor>
void Walk_cr_postorder(CODEREP *cr, Visitor &vis) {
  if (!cr) return;
  if (cr->Is_op()) {
    for (int i = 0; i < cr->_op._num_kids; ++i)
      Walk_cr_postorder(cr->_op._kids[i], vis);
  }
  vis.Visit_cr(cr);
}
```

### 3.3 Def-Use 链遍历

版本号本身就是 def-use 的键。通过 `OPT_STAB::Ver(aux, ver)` 找到 `def_stmt`/`def_phi`，即可反向追踪。

---

## 四、支配树计算（`opt/opt_dom.cxx`）

使用**迭代数据流算法**（简单，代码量小，足够正确）。

```
算法（Cooper et al. "A Simple, Fast Dominance Algorithm"）：

输入：CFG，entry BB = bb[0]
初始化：idom[entry] = entry; 所有其他 idom[b] = undef
RPO 编号：rpo_num[b]

repeat:
  changed = false
  for each b in RPO（跳过 entry）:
    new_idom = (任意一个已处理的前驱)
    for each 其他前驱 p of b:
      if idom[p] != undef:
        new_idom = intersect(p, new_idom)
    if idom[b] != new_idom:
      idom[b] = new_idom
      changed = true
until !changed

intersect(b1, b2):
  while b1 != b2:
    while rpo_num[b1] > rpo_num[b2]: b1 = idom[b1]
    while rpo_num[b2] > rpo_num[b1]: b2 = idom[b2]
  return b1
```

计算完 `idom` 后：
1. 填充每个 BB 的 `_dom_list`（idom 的直接子节点列表）
2. 计算 Dominance Frontier（Cytron 算法）：

```
for each bb b:
  if b.preds.size() >= 2:
    for each pred p of b:
      runner = p
      while runner != idom[b]:
        runner._df_list.add(b)
        runner = idom[runner]
```

接口：

```cpp
// opt/opt_dom.h
class DOM_BUILDER {
public:
  void Build(SSA_CFG *cfg);   // 计算 idom, dom_list, df_list
private:
  void Compute_rpo(SSA_CFG *cfg, vector<SSABB *> &rpo);
  void Compute_idom(SSA_CFG *cfg, vector<SSABB *> &rpo);
  void Compute_df(SSA_CFG *cfg);
};
```

---

## 五、SSA 构建（`opt/opt_ssa.cxx` 重写）

### 5.1 整体流程

```
SSA_COMPOSITE::Construct_function(func_sym):
  1. CFG_FROM_IR::Build(tree, cfg)      // IR → SSABB + STMTREP（不含版本）
  2. DOM_BUILDER::Build(cfg)            // 计算 idom / df
  3. OPT_STAB 初始化（扫描所有 STID，建立 AUX_ID 映射）
  4. Collect_defs_bb()                  // 每个变量的 def_bbs 集合
  5. Place_phi_nodes()                  // Cytron phi 放置
  6. SSA_Rename()                       // DFS 支配树重命名
```

### 5.2 步骤 1：IR → CFG（`opt/opt_cfg_builder.h`）

```cpp
class CFG_FROM_IR {
public:
  void Build(TREE *tree, SSA_CFG *cfg, OPT_STAB *stab);
private:
  void Scan_stmts(IR_ITER block, SSABB *cur_bb);
  STMTREP *Lower_stmt(IR_ITER stmt);   // IRNODE → STMTREP + CODEREP 树（未版本化）
  CODEREP *Lower_expr(IR_ITER expr);   // 递归建 CODEREP 树
};
```

`Lower_expr` 递归建立 CK_CONST / CK_LDA / CK_VAR（version=0）/ CK_OP 树：

```
INTCONST → Make_const(val)
LDID     → Make_var(aux_id=stab.Lookup(st), ver=0)
LDA      → Make_lda(st, offset)
ILOAD    → Make_ivar(Lower_expr(base), offset, ty)
ADD/SUB  → Make_op(opc, Lower_expr(k0), Lower_expr(k1))
STID     → stmtrep.lhs = Make_var(aux, 0), stmtrep.rhs = Lower_expr(rhs)
```

遇到 `GOTO`/`TRUEBR`/`FALSEBR`/`LABEL` 时切割 BB，维护 label→BB 映射，最后调用 `Fixup_pred_succ()`。

### 5.3 步骤 4：收集定义点

```cpp
void SSA_COMPOSITE::Collect_defs_bb() {
  // 对每个 BB 的每个 STMTREP，若 lhs 是 CK_VAR，将 bb 加入 stab.Var(aux).def_bbs
  Walk_cfg_rpo(cfg, [&](SSABB *bb) {
    for (auto *s : bb->_stmtreps)
      if (s->_lhs && s->_lhs->Is_var())
        stab->Var(s->_lhs->Aux_id()).def_bbs.insert(bb);
  });
}
```

### 5.4 步骤 5：放置 Phi 节点（Cytron 算法）

```cpp
void SSA_COMPOSITE::Place_phi_nodes() {
  for (AUX_ID aux = 0; aux < stab->Num_vars(); ++aux) {
    // worklist = def_bbs of this variable
    queue<SSABB *> worklist(stab->Var(aux).def_bbs);
    set<SSABB *>   phi_placed;
    while (!worklist.empty()) {
      SSABB *bb = worklist.front(); worklist.pop();
      for (auto df_it = bb->Df_begin(); df_it != bb->Df_end(); ++df_it) {
        SSABB *y = *df_it;
        if (phi_placed.count(y) == 0) {
          // 插入 phi：操作数个数 = y 的前驱数
          PHI_NODE *phi = new PHI_NODE();
          phi->_aux_id = aux;
          phi->_bb = y;
          phi->_opnd_vers.resize(y->Get_preds_count(), 0);
          y->Add_phi(phi);
          phi_placed.insert(y);
          if (stab->Var(aux).def_bbs.count(y) == 0)
            worklist.push(y);
        }
      }
    }
  }
}
```

### 5.5 步骤 6：SSA 重命名（DFS 支配树）

```cpp
// 每个变量维护一个版本栈
vector<stack<UINT32>> rename_stk;  // rename_stk[aux_id]

void SSA_Rename_bb(SSABB *bb) {
  // 1. 处理此 BB 头部的 phi 的结果
  for (auto *phi : bb->_phi_list) {
    UINT32 new_ver = stab->New_version(phi->_aux_id);
    phi->_result_ver = new_ver;
    phi->_result_cr  = CODEREP::Make_var(phi->_aux_id, new_ver, ...);
    stab->Ver(phi->_aux_id, new_ver).def_phi = phi;
    stab->Ver(phi->_aux_id, new_ver).def_bb  = bb;
    rename_stk[phi->_aux_id].push(new_ver);
  }

  // 2. 处理 BB 中的每条语句
  for (auto *s : bb->_stmtreps) {
    // 先重命名 RHS 的所有 CK_VAR（使用）
    Rename_uses(s->_rhs);
    // 再重命名 LHS（定义）
    if (s->_lhs && s->_lhs->Is_var()) {
      AUX_ID aux = s->_lhs->Aux_id();
      UINT32 new_ver = stab->New_version(aux);
      s->_lhs->_var._version = new_ver;
      stab->Ver(aux, new_ver).def_stmt = s;
      stab->Ver(aux, new_ver).def_bb   = bb;
      rename_stk[aux].push(new_ver);
    }
  }

  // 3. 更新后继 BB 的 phi 操作数
  for (auto succ_it = bb->Succ_begin(); succ_it != bb->Succ_end(); ++succ_it) {
    SSABB *succ = *succ_it;
    INT32 pred_pos = succ->Pred_pos(bb);
    for (auto *phi : succ->_phi_list) {
      phi->_opnd_vers[pred_pos] = rename_stk[phi->_aux_id].empty()
                                   ? 0 : rename_stk[phi->_aux_id].top();
    }
  }

  // 4. DFS 支配树子节点
  for (auto it = bb->Dom_begin(); it != bb->Dom_end(); ++it)
    SSA_Rename_bb(*it);

  // 5. 退出此 BB 时弹栈（恢复此 BB 的定义）
  for (auto *phi : bb->_phi_list)
    rename_stk[phi->_aux_id].pop();
  for (auto *s : bb->_stmtreps)
    if (s->_lhs && s->_lhs->Is_var())
      rename_stk[s->_lhs->Aux_id()].pop();
}

// Rename_uses：后序遍历 CODEREP，对 CK_VAR 查栈顶版本
void Rename_uses(CODEREP *cr) {
  Walk_cr_postorder(cr, [&](CODEREP *c) {
    if (c->Is_var()) {
      AUX_ID aux = c->Aux_id();
      UINT32 ver = rename_stk[aux].empty() ? 0 : rename_stk[aux].top();
      c->_var._version = ver;
      if (ver > 0) c->_var._def_stmt = stab->Ver(aux, ver).def_stmt;
    }
  });
}
```

---

## 六、Dead Code Elimination（`opt/opt_dce.h` / `opt_dce.cxx`）

### 算法：标记-清除，基于 def-use 反向传播

```cpp
class DCE {
public:
  void Run(SSA_CFG *cfg, OPT_STAB *stab);
private:
  queue<STMTREP *> _worklist;

  // 标记阶段
  void Mark_phase(SSA_CFG *cfg);
  void Mark_stmt(STMTREP *s);
  void Mark_cr_uses(CODEREP *cr);  // 递归标记 cr 中所有 CK_VAR 的 def

  // 清除阶段
  void Sweep_phase(SSA_CFG *cfg);
};

void DCE::Run(SSA_CFG *cfg, OPT_STAB *stab) {
  Mark_phase(cfg);
  Sweep_phase(cfg);
}
```

**Mark 阶段**：

```
// 初始化 worklist：有副作用的语句天然活跃
Walk_cfg_rpo(cfg, [&](SSABB *bb) {
  for (auto *s : bb->_stmtreps)
    if (s->Has_side_effect()) {
      s->_live = TRUE;
      _worklist.push(s);
    }
});

while (!_worklist.empty()):
  s = pop()
  Mark_cr_uses(s->_rhs)
  if s->_lhs is CK_IVAR: Mark_cr_uses(s->_lhs->_ivar._base)

Mark_cr_uses(cr):
  Walk_cr_postorder(cr, [&](CODEREP *c) {
    if c.Is_var() and c.version > 0:
      if c.def_stmt and not c.def_stmt._live:
        c.def_stmt._live = TRUE
        _worklist.push(c.def_stmt)
      elif c.def_phi and not c.def_phi._live:
        c.def_phi._live = TRUE
        // 将 phi 所有操作数版本的 def_stmt 加入 worklist
        for each opnd_ver in phi._opnd_vers:
          def = stab.Ver(phi.aux_id, opnd_ver).def_stmt
          if def and not def._live:
            def._live = TRUE; worklist.push(def)
  })
```

**Sweep 阶段**：

```
Walk_cfg_rpo(cfg, [&](SSABB *bb) {
  // 删除 dead stmtreps
  bb->_stmtreps.erase_if([](STMTREP *s) { return !s->_live; })
  // 删除 dead phi（result 无活跃使用）
  bb->_phi_list.erase_if([](PHI_NODE *phi) { return !phi->_live; })
})
```

---

## 七、常量传播（`opt/opt_cprop.h` / `opt_cprop.cxx`）

### 算法：简单前向传播（RPO 顺序，无需 SCCP）

```cpp
class CONST_PROP {
public:
  void Run(SSA_CFG *cfg, OPT_STAB *stab);
private:
  // 尝试将 cr 化简为常数，返回新 CODEREP*（可能是 CK_CONST）
  CODEREP *Fold(CODEREP *cr);
  // 对 stmtrep 做常量折叠，返回是否发生变化
  BOOL     Prop_stmt(STMTREP *s);
  // 对 phi 做化简
  BOOL     Prop_phi(PHI_NODE *phi, OPT_STAB *stab);
};
```

**`Fold` 实现**（递归自底向上）：

```
Fold(cr):
  if cr.Is_const(): return cr

  if cr.Is_var():
    // 查找定义：若 def_stmt.rhs 是常数，直接返回该常数
    if cr.def_stmt and Fold(cr.def_stmt.rhs).Is_const():
      return Fold(cr.def_stmt.rhs)
    // 若 def_phi 且所有操作数版本是同一常数，返回该常数
    if cr.def_phi:
      return Try_fold_phi(cr.def_phi)
    return cr

  if cr.Is_op():
    k0 = Fold(kids[0]);  k1 = (num_kids==2) ? Fold(kids[1]) : null
    if k0.Is_const() and (num_kids==1 or k1.Is_const()):
      return Eval_const_op(cr.opc, k0.const_val, k1?.const_val)
    cr.kids[0] = k0; cr.kids[1] = k1
    return cr

Try_fold_phi(phi):
  // 所有操作数都是同一常数 c → 返回 CONST(c)
  c = 第一个 opnd 的折叠结果
  if not c.Is_const(): return null
  for each other opnd: if Fold(opnd) != c: return null
  return c
```

**`Run` 实现**：

```
// RPO 遍历，重复直到不动点（通常 1-2 轮）
changed = true
while changed:
  changed = false
  Walk_cfg_rpo(cfg, [&](SSABB *bb) {
    for phi in bb._phi_list:
      changed |= Prop_phi(phi, stab)
    for s in bb._stmtreps:
      new_rhs = Fold(s._rhs)
      if new_rhs != s._rhs: s._rhs = new_rhs; changed = true
      if s._lhs is CK_IVAR:
        new_base = Fold(s._lhs._ivar._base)
        if new_base != old: changed = true
  })
```

---

## 八、Pass 调度（`opt/opt_main.cxx` 修改）

```cpp
INT32 BE_EXTERNAL_MAIN_NAME(COMPILER_CONFIG &conf) {
  Opt_lower(File(), LEVEL_HIGH, conf);
  Opt_verify(File(), LEVEL_HIGH, conf);

  Opt_lower(File(), LEVEL_MID, conf);    // 修复：去掉重复的 HIGH 调用
  Opt_verify(File(), LEVEL_MID, conf);

  // SSA 构建 + 优化
  Opt_build_ssa_all(File(), LEVEL_MID, conf);  // 内含 CFG build / DOM / phi / rename

  // 基于 SSA 的 pass（依次运行）
  Opt_run_cprop(File(), conf);   // 常量传播
  Opt_run_dce(File(), conf);     // DCE

  Opt_destruct_ssa_all(File(), LEVEL_MID, conf);  // out-of-SSA（phi → copy）

  Opt_lower(File(), LEVEL_LOW, conf);
  Opt_verify(File(), LEVEL_LOW, conf);

  Opt_lower(File(), LEVEL_VLOW, conf);
  Opt_lower(File(), LEVEL_CGIR, conf);
  return 0;
}
```

---

## 九、Out-of-SSA（`Opt_destruct_ssa`）

phi 节点转为 copy 语句，插入到对应前驱 BB 的末尾：

```
for each BB b:
  for each phi in b._phi_list:
    for each pred_idx, pred_bb in b.preds:
      ver = phi._opnd_vers[pred_idx]
      // 在 pred_bb 末尾（跳转语句之前）插入：
      // STID(phi.result_ver) = LDID(ver)
      insert_copy(pred_bb, phi._aux_id, phi._result_ver, ver)
  b._phi_list.clear()
```

---

## 十、文件组织

```
opt/
  opt_coderep.h / opt_coderep.cxx   ← CODEREP, PHI_NODE 定义与工厂方法
  opt_stab.h    / opt_stab.cxx      ← OPT_STAB, OPT_VAR, VER_ENTRY
  opt_stmt.h                        ← STMTREP（现有文件扩展）
  opt_walk.h                        ← Walk_cfg_rpo, Walk_dom_tree, Walk_cr_postorder 模板
  opt_cfg_builder.h / .cxx          ← CFG_FROM_IR（IR → SSABB + STMTREP）
  opt_dom.h     / opt_dom.cxx       ← DOM_BUILDER（idom + df 计算）
  opt_ssa.h     / opt_ssa.cxx       ← SSA_COMPOSITE（统筹 phi 放置 + rename）
  opt_dce.h     / opt_dce.cxx       ← DCE pass
  opt_cprop.h   / opt_cprop.cxx     ← 常量传播 pass
  opt_basic.h                       ← SSABB / SSAIR 扩展（现有文件）
  opt_main.cxx                      ← pass 调度（修复重复 HIGH + 补 destruct）
```

---

## 十一、实现顺序建议

| 步骤 | 文件 | 说明 |
|------|------|------|
| 1 | `opt_coderep.h/cxx` | 定义 CODEREP / PHI_NODE，实现工厂方法和 Print |
| 2 | `opt_stab.h/cxx` | OPT_STAB：AUX_ID 映射、版本分配 |
| 3 | `opt_walk.h` | 三个模板遍历（20 行以内） |
| 4 | `opt_cfg_builder.cxx` | IR → SSABB/STMTREP，Lower_expr 递归 |
| 5 | `opt_dom.cxx` | Cooper 算法 idom + Cytron DF（共约 100 行） |
| 6 | `opt_ssa.cxx` | Collect_defs / Place_phi / SSA_Rename |
| 7 | `opt_dce.cxx` | Mark-sweep DCE（约 80 行） |
| 8 | `opt_cprop.cxx` | Fold + Run（约 80 行） |
| 9 | `opt_main.cxx` | 修复 pass 调度，接入新 pass |
| 10 | 验证 | 用 testcase/ 里的 C 程序跑端到端测试 |
