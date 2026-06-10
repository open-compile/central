# CG (Code Generation) 算法 Spec

> **目的**: 给 central 项目的 code-gen 阶段写一份算法 spec.
> 当前实现是非常 naive 的, 本 spec 描述**正确**的算法, 供使用者参考实现.
>
> **范围**:
> 1. CG 主流程的 pass 划分
> 2. Value Dependency Graph (VDG) 设计与构造
> 3. Live Range Analysis (反向 dataflow)
> 4. Register Allocation: Chaitin + Briggs graph coloring (经典) + Linear Scan (Wimmer LLVM)
> 5. Spill Rewrite + Rematerialization
> 6. Frame Layout
>
> **目标平台**: ARMv7-A EABI / AAPCS

---

## 0. 背景与术语

### 0.1 TN (Temporary Node)

`TN` = SSA 化之前对寄存器 / 立即数的"逻辑槽位".
每个 TN 在 register allocation 后会被映射到:
- 一个物理寄存器 (e.g. r0, r3, ...)
- 或者一个栈槽位 (spill)
- 或者一个立即数

TN 的种类:
```
TN is_symbol    : 对应一个 ST_IDX (全局/局部变量)
TN is_label     : 对应一个 LABEL_IDX (函数地址)
TN is_constant  : 立即数
TN is_register  : 已经是物理寄存器 (dedicated)
TN is_preallocated: 已分配 (被 spilling 强制选过)
TN is_dedicated : AAPCS 保留 (r0-r3, r12, r13=sp, r14=lr, r15=pc)
```

### 0.2 CGOP (CG-level instruction)

`CGOP` = ARM 指令的一条 (`ADD`, `LDR`, `STR`, `B`, ...).
3-operand 形式: `result <- op1 op op2`.

### 0.3 CGBB

`CGBB` = ARM 级别的基本块. 包含一个 CGOP 序列.

### 0.4 关键不变量

- 每个 TN 的 def **唯一** (在 CGIR 阶段已经是 SSA-like)
- 每个 CGBB 的最后一条 CGOP 是 branch / return / 间接 jump
- 没有空 CGBB (可以有空, 但 trivially removable)

---

## 1. CG 主流程的 Pass 划分

```
┌──────────────────────────────────────────────────────────────┐
│  OCIR (TREE)                                                  │
└──────────────────────────────────────────────────────────────┘
       │ CG_Build_Pass
       ▼
┌──────────────────────────────────────────────────────────────┐
│  CGIR (CG_CFG + TN table)                                     │
│  - CGBB / CGOP 已生成                                          │
│  - TN 已分配但 register = 0                                    │
└──────────────────────────────────────────────────────────────┘
       │ CG_LiveRange_Pass
       ▼
┌──────────────────────────────────────────────────────────────┐
│  Live Range Information (per TN: [first_def, last_use])        │
│  + Interference Graph (CG_IFG)                                 │
└──────────────────────────────────────────────────────────────┘
       │ CG_GRA_Pass (Chaitin / Briggs)
       │  (失败 spill → 走 CG_SpillRewrite_Pass)
       ▼
┌──────────────────────────────────────────────────────────────┐
│  CGIR with TN.register set                                    │
└──────────────────────────────────────────────────────────────┘
       │ CG_FrameLayout_Pass
       ▼
┌──────────────────────────────────────────────────────────────┐
│  CGIR + DATA_LAYOUT                                            │
└──────────────────────────────────────────────────────────────┘
       │ CG_Emit_Pass
       ▼
┌──────────────────────────────────────────────────────────────┐
│  ARMv7 Assembly (.s file)                                      │
└──────────────────────────────────────────────────────────────┘
```

**实现建议** (取代现有 `CG_process_funcs`):
```cpp
class CG_PASS {
public:
  virtual void Run(PU_INFO *pu, CGIR *cgir, OPT_CONFIG &cfg) = 0;
  virtual const char *Name() const = 0;
  virtual ~CG_PASS() {}
};

class CG_COMPOSITE {
  std::vector<CG_PASS *> _passes;
public:
  void Add_pass(CG_PASS *p) { _passes.push_back(p); }
  void Run_passes(PU_INFO *pu) {
    for (auto *p : _passes) {
      Is_Trace(Tracing(COMPONENT_CG, TRACE_INVOCATION),
               (TFile, "[CG] running pass %s\n", p->Name()));
      p->Run(pu, _cgir, _cfg);
    }
  }
};
```

---

## 2. Value Dependency Graph (VDG)

> VDG 是 central 缺失的关键数据结构.
> Spec 来源: Muchnick '97 §6 / Click '95 / Pingali '92.

### 2.1 节点

```
enum VDG_NODE_KIND {
  VDG_BB,        // 一个 CGBB
  VDG_OP,        // 一条 CGOP
  VDG_TN_DEF,    // 一个 TN 的 def 节点
  VDG_TN_USE,    // 一个 TN 的 use 节点
  VDG_REGION,    // 嵌套 region (用于 hierarchical VDG)
  VDG_LABEL,     // BB 入口 label
};

struct VDG_NODE {
  VDG_NODE_KIND kind;
  UINT32        id;             // 唯一 id
  void         *orig;           // 指向 BB / CGOP / TN
  union {
    struct { SSABB *bb; }            v_bb;
    struct { CGOP  *op; }            v_op;
    struct { TN    *tn; UINT32 ver; } v_tn;
  };
};
```

### 2.2 边

```
enum VDG_EDGE_KIND {
  EDGE_USE_DEF,         // TN-Use → TN-Def (use 来自 def)
  EDGE_DEF_USE,         // 反向
  EDGE_BB_CONTAINS_OP,  // BB 包含 OP
  EDGE_OP_DEFS_TN,      // OP 定义了 TN
  EDGE_OP_USES_TN,      // OP 使用了 TN
  EDGE_BB_SUCC,         // 控制流 succ
  EDGE_BB_PRED,         // 控制流 pred
  EDGE_REGION_CONTAINS, // region 嵌套
  EDGE_DATA_DEP,        // RAW / WAR / WAW
};
```

### 2.3 构造算法

```cpp
void Build_VDG(CGIR *cgir, VDG *vdg) {
  vdg->Clear();

  // 1. 顶点: CGBB
  for (auto *bb : cgir->Cfg()->BBs()) {
    vdg->Add_node({VDG_BB, .v_bb={bb}});
  }

  // 2. 顶点: CGOP, 边 BB-contains-OP
  for (auto *bb : cgir->Cfg()->BBs()) {
    for (auto *op : bb->Stmts()) {
      UINT32 op_id = vdg->Add_node({VDG_OP, .v_op={op}});
      vdg->Add_edge(bb_id, op_id, EDGE_BB_CONTAINS_OP);
    }
  }

  // 3. 顶点: TN-Def (每个 TN 一次)
  //    顶点: TN-Use (每个 use 一次, 区分位置)
  //    边 OP-defs-TN / OP-uses-TN
  for (auto *op : all_ops) {
    if (op->Has_result()) {
      TN *tn = op->Get_result_tn();
      UINT32 def_id = vdg->Get_or_add_tn_def(tn);
      vdg->Add_edge(op_id, def_id, EDGE_OP_DEFS_TN);
    }
    for (UINT32 k = 0; k < op->Num_opr(); k++) {
      TN *tn = op->Get_opr_tn(k);
      UINT32 use_id = vdg->Add_tn_use(tn, op, k);
      vdg->Add_edge(op_id, use_id, EDGE_OP_USES_TN);
    }
  }

  // 4. 边 TN-Use → TN-Def
  for (auto &use : vdg->All_tn_uses()) {
    TN *tn = use.tn;
    UINT32 def_id = vdg->Get_tn_def(tn);  // 唯一 def
    vdg->Add_edge(use.id, def_id, EDGE_USE_DEF);
  }

  // 5. 边 BB-Succ / BB-Pred
  for (auto *bb : cgir->Cfg()->BBs()) {
    for (auto *succ : bb->Succs()) {
      vdg->Add_edge(bb->id, succ->id, EDGE_BB_SUCC);
    }
  }
}
```

### 2.4 VDG 上的算法

**Dead Code Elimination**:
```cpp
Set<UINT32> live;
Worklist<UINT32> W;

// 标记 side-effect 根
for (auto *op : cgir->All_ops()) {
  if (op->Has_side_effect()) {
    W.push(op->id);
    live.insert(op->id);
  }
}

// 反向 mark
while (!W.empty()) {
  UINT32 v = W.pop();
  for (auto *e : vdg->In_edges(v)) {
    if (e->kind == EDGE_USE_DEF) {
      // 走 USE→DEF 链, 标记 def
      UINT32 def = e->from;
      if (!live.count(def)) { live.insert(def); W.push(def); }
    } else if (e->kind == EDGE_BB_CONTAINS_OP) {
      // 标记整个 BB
      UINT32 bb = e->from;
      if (!live.count(bb)) { live.insert(bb); W.push(bb); }
    }
    // 其它边也 mark
  }
}

// Sweep: 删除未 live 的 OP
for (auto *bb : cgir->Cfg()->BBs()) {
  bb->Remove_ops_if([&](CGOP *op) { return !live.count(op->id); });
}
```

**Data-Flow Analysis** (liveness, reaching-defs, ...):
- 在 VDG 上做 graph-reachability
- 比传统 bit-vector 快得多 (Click '95)

**Register Allocation 输入**:
- 干涉图 = VDG 的 TN 节点 + "live-range overlap" 边 (从 data-flow 推出)

---

## 3. Live Range Analysis

### 3.1 定义

`live_range(tn) = [d, u]` 其中:
- `d` = tn 的 def 在 CGOP 序列中的位置 (用线性编号 `bb * 1000 + stmt_id`)
- `u` = tn 的 last use 的位置

### 3.2 反向数据流

```
gen[BB]  = { tn : tn is defined in BB, before any use in BB }
kill[BB] = { tn : tn is used in BB, before any def in BB }
live_in[BB]  = (use[BB]) ∪ (live_out[BB] - def[BB])
live_out[BB] = ∪_{succ in BB.succs} live_in[succ]

use[BB]  = { tn : tn used in BB, before any def in BB }
def[BB]  = { tn : tn defined in BB, before any use in BB }
```

### 3.3 不动点算法

```cpp
void Analyze_live_range(PU_INFO *info) {
  auto &bb_live_in = ...;   // vector<BitSet<TN_IDX>>
  auto &bb_live_out = ...;
  auto &use = ...;
  auto &def = ...;

  // 初始化: 全部 false
  for (auto *bb : cfg->BBs()) {
    bb_live_in[bb->id]  = BitSet(N_TN);
    bb_live_out[bb->id] = BitSet(N_TN);
  }

  bool changed = true;
  while (changed) {
    changed = false;
    // 反向 RPO 顺序迭代
    for (auto *bb : rpo_reverse(cfg)) {
      BitSet new_out = ∅;
      for (auto *succ : bb->Succs()) {
        new_out |= bb_live_in[succ->id];
      }
      BitSet new_in = use[bb->id] | (new_out - def[bb->id]);
      if (new_in != bb_live_in[bb->id] || new_out != bb_live_out[bb->id]) {
        bb_live_in[bb->id]  = new_in;
        bb_live_out[bb->id] = new_out;
        changed = true;
      }
    }
  }

  // 拼出每个 TN 的 live range
  for (auto &tn : tn_table) {
    tn.first_def = ∞;
    tn.last_use  = -1;
    for (auto *bb : cfg->BBs()) {
      for (auto *op : bb->Stmts()) {
        UINT32 pos = bb->id * 1000 + op->index_in_bb;
        if (op->Defs(tn)) tn.first_def = min(tn.first_def, pos);
        if (op->Uses(tn)) tn.last_use  = max(tn.last_use, pos);
      }
    }
  }
}
```

### 3.4 优化

- **BitSet 实现**: 用 `std::bitset<>` 或 `boost::dynamic_bitset`
- **Worklist 算法**: 替代 RPO 迭代, 复杂度 O(E · |TN|)
- **Sparse**: 只对实际 live 的 TN 维护

---

## 4. Register Allocation

### 4.1 当前实现的问题

```
next_register = 1
for each TN in tn_freq_map:
  if next_register >= 7: spill all
  else: assign rN
```

**问题**:
- 没建干涉图, 可能两个干扰的 TN 分配到同一寄存器
- 没考虑 live range 重叠
- spill cost 没模型 (all-or-nothing)
- 不区分 caller/callee-save

### 4.2 目标: 干涉图 + Graph Coloring (Chaitin / Briggs)

#### 4.2.1 构造干涉图 (Interference Graph)

```cpp
void Build_Interference_Graph(CGIR *cgir, Ifg &G) {
  G.Clear();

  // 1. 顶点: 每个 TN (除了已 dedicated 的)
  for (auto &tn : cgir->TN_table()) {
    if (tn.is_dedicated || tn.is_preallocated) continue;
    G.Add_vertex(tn);
  }

  // 2. 边: TN a 干扰 TN b iff 它们的 live range 重叠
  //    简化: O(N^2) pair-wise check
  for (auto a : G.Vertices()) {
    for (auto b : G.Vertices()) {
      if (a == b) continue;
      if (Live_ranges_overlap(a, b)) {
        G.Add_edge(a, b);
      }
    }
  }
}
```

Live-range overlap 简化判定:
```cpp
bool Live_ranges_overlap(TN *a, TN *b) {
  // 两个 TN 在同一 BB 同时 live → 干扰
  for (auto *bb : cfg->BBs()) {
    if (bb->Liveness().Contains(a) && bb->Liveness().Contains(b)) {
      return true;
    }
  }
  return false;
}
```

精确判定 (Webster'94): 用区间 [first_def, last_use] 的 overlap.

#### 4.2.2 Chaitin 图染色 (原始版, 1981)

```
K = 可用寄存器数 (ARMv7: r0-r3 + r4-r10 = 11 个 caller/callee save 池, 视 ABI)
Loop:
  1. 把 K 阶以下 (degree < K) 的节点压栈 (Simplify)
  2. 重复 1, 直到剩 ≥K 阶的节点
  3. 如果有, 选 spill 候选 (启发式), 压栈 (Spill)
  4. 重复 1-3, 直到空
  5. 反向弹栈, 选 K 种颜色之一 (Select)
  6. 若弹出时发现无色可选 → 实际 spill (rewrite)
```

#### 4.2.3 Briggs 改进 (1992)

- **Conservative coalescing**: 移除 move 关联边 (TN a = TN b), 直到不能 simplify 为止
- **Optimistic coloring**: 在 Simplify 时把 ≥K 阶的节点也压栈, 但记为 "potentially spillable"
- **Spill heuristics**: cost = (num uses + num defs) / degree; cost < threshold 的节点才真 spill

```cpp
void Chaitin_Briggs(Ifg &G, UINT32 K, std::vector<TN *> &spills) {
  std::stack<TN *> stk;
  std::set<TN *>   spilled;

  // Phase 1: Simplify
  while (!G.Empty()) {
    auto v = G.Find_low_degree(K);
    if (v) {
      stk.push(v);
      G.Remove(v);   // 暂时移除, 推栈
    } else {
      // Briggs 优化: 选 cost 最低的节点
      auto s = G.Find_spill_candidate();
      stk.push(s);
      G.Remove(s);
    }
  }

  // Phase 2: Select
  while (!stk.empty()) {
    auto v = stk.top(); stk.pop();
    G.Restore(v);
    auto avail = Available_colors(v, K);
    if (!avail.empty()) {
      v->color = *avail.begin();
    } else {
      spilled.insert(v);
    }
  }

  spills.assign(spilled.begin(), spilled.end());
}
```

#### 4.2.4 颜色 (颜色池)

ARMv7 AAPCS:
- r0-r3: caller-saved, 参数/返回值
- r4-r11: callee-saved (含 r9 = platform reg)
- r12: IP, scratch
- r13 = sp, r14 = lr, r15 = pc: 不可用

**Pooling**:
```
Integer class:  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r10, r11}  (11 个, 不含 r9, r12)
Float class:    {s0-s15} (或 d0-d7)
```

### 4.3 Linear Scan (Wimmer 2010, LLVM)

> 适合 JIT / 短编译时间的场景; 中央项目若做 LTO / 大函数, 走 coloring 更优.

```
Sort live ranges by start point.
active = ∅
for each interval I in order:
  Expire_old_intervals(active, I.start)
  if active.size() == K:
    spill I or some J in active (heuristic: longest)
  else:
    allocate register to I
  add I to active
```

优点: O(N) ~ O(N log N).
缺点: 染色质量比 coloring 略差.

### 4.4 选哪个?

- **小函数** (N < 100): Linear Scan 更快
- **大函数** (N ≥ 100): Chaitin-Briggs 染色质量更好
- **JIT / 实时代码**: Linear Scan
- **AOT / 静态编译**: Chaitin-Briggs

central 项目目前是 AOT, 建议: **Chaitin-Briggs** 为 GRA, **Linear Scan** 为 LRA fallback.

---

## 5. Spill Rewrite

### 5.1 选 Spill 候选

启发式 (Chaitin):
```
spill_cost(tn) = (num_uses * weight_use) + (num_defs * weight_def)
spill_priority(tn) = spill_cost(tn) / degree(tn)
```

权重:
- `weight_use` = 1
- `weight_def` = 1
- 循环内: × 10
- 跨 call: × 5 (因 caller-save)

### 5.2 Rewrite

对每个 spill 的 TN, 在每个 def 前插 `STR tn, [sp, #off]`, 在每个 use 后插 `LDR tn, [sp, #off]`.

```cpp
void Rewrite_spill(TN *tn, UINT32 slot_offset) {
  for (auto *bb : cfg->BBs()) {
    for (auto *op : bb->Stmts()) {
      if (op->Defs(tn)) {
        // 在 op 之前插 STR
        bb->Insert_before(op, Make_STR(tn, slot_offset));
      }
      if (op->Uses(tn)) {
        // 在 op 之后插 LDR
        bb->Insert_after(op, Make_LDR(tn, slot_offset));
      }
    }
  }
}
```

### 5.3 Rematerialization

若 `tn` 是便宜重算的 (e.g. `tn = constant`), 不要 spill, 而是在每个 use 处重新算.
- cost(tn) = ∞, degree(tn) = 0, 不需要存
- rewrite: 在 use 处插 `tn = const_value`

---

## 6. Frame Layout

### 6.1 ARMv7 AAPCS Frame

```
高地址
+-----------------------+
|  arg build area (可选) |  ← 出参 (caller) 写入
+-----------------------+
|  prev frame pointer    |  ← [fp]
+-----------------------+
|  saved LR (link reg)   |  ← [fp, #4]   (用 push {fp, lr} 一起保存)
+-----------------------+
|  saved FP              |  ← [fp]
+-----------------------+
|  saved callee-save regs|  ← 用了 r4-r11 才保存
+-----------------------+
|  spill slots           |  ← 寄存器分配后的溢出
+-----------------------+
|  local variables       |  ← 局部变量, 8-byte 对齐
+-----------------------+
|  outgoing args          |  ← 调用 >4 个参数函数时, 写入此区
+-----------------------+
低地址 ← sp
```

### 6.2 计算

```cpp
void Layout_frame(CGIR *cgir) {
  UINT32 offset = 0;
  // 1. 8-byte 对齐
  offset = Align_up(offset, 8);

  // 2. 保存的 callee-save 寄存器
  for (auto &tn : cgir->TN_table()) {
    if (tn.is_preallocated && tn.reg_class == INT) {
      cgir->Layout()->Allocate_object(tn.spill_sym, 4);
      tn.spill_offset = cgir->Layout()->Get_sym_sp_ofst(tn.spill_sym);
    }
  }

  // 3. Spill slots
  for (auto &tn : cgir->TN_table()) {
    if (tn.needs_spill_slot) {
      cgir->Layout()->Allocate_object(tn.spill_sym, 4);
      tn.spill_offset = cgir->Layout()->Get_sym_sp_ofst(tn.spill_sym);
    }
  }

  // 4. 局部变量
  // 5. outgoing args
  for (auto &v : cgir->Local_vars()) {
    cgir->Layout()->Allocate_object(v.sym, v.size);
  }
}
```

### 6.3 Prologue / Epilogue

```asm
; Prologue
push    {fp, lr}              ; 8 bytes
push    {r4-r10}              ; 28 bytes (if needed)
add     fp, sp, #28           ; fp = sp after pushes
sub     sp, sp, #N            ; N = local + spill + outgoing

; Epilogue
sub     sp, fp, #28
pop     {r4-r10}
pop     {fp, pc}              ; 用 pc 而非 lr, 直接返回
```

---

## 7. 实现路线图 (建议)

> 给使用者的实施顺序, 从小到大.

### Phase 1: 基础补齐
1. 实现 `CG_LIVE_RANGE::Analyze_live_range` (反向 dataflow, 不动点)
2. 实现 `CG_REG_ALLOC::Build_Interference_Graph`
3. 把 naive alloc 替换成 Chaitin-Briggs

### Phase 2: 优化
4. 加 callee/caller-save 区分
5. 加 spill cost model
6. 加 rematerialization

### Phase 3: VDG
7. 写 `VDG` 数据结构
8. 在 CG_Build 阶段构造 VDG
9. 把 DCE / 数据流分析迁到 VDG 上

### Phase 4: Pass Pipeline
10. 写 `CG_PASS` 抽象基类
11. 把现有 build / alloc / layout / emit 拆成 4 个 pass
12. 加 `--cgpasses` 命令行选项

---

## 8. 经典参考

- **Chaitin '81**: "Register Allocation & Spilling via Graph Coloring" (原始)
- **Chaitin '82**: "Register Allocation & Spilling via Graph Coloring" (改进)
- **Briggs '92**: "Improvements to an Iterative Graph Coloring Register Allocator"
- **Wimmer '10**: "Linear Scan Register Allocation on SSA Form"
- **Click '95**: "Global Code Motion / Global Value Numbering"
- **Muchnick '97**: "Advanced Compiler Design and Implementation" §6, §16-18
- **AAPCS**: ARM Architecture Procedure Call Standard

---

## 9. 关键公式 / 备忘

```
# 干涉图
G = (V, E)
V = { tn : tn needs a register }
E = { (a, b) : live_range(a) ∩ live_range(b) ≠ ∅ }

# Chaitin simplify 终止条件
∀ v ∈ remaining: degree(v) ≥ K

# Briggs spill cost
cost(v) = Σ_{op ∈ ops(v)} (10 ^ in_loop(op))  /  degree(v)

# Linear scan
order: by start point of live range
active: { live ranges that started but not ended }
```
