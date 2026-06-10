# CG (Code Generation) 诊断报告

> **生成时间**: 2026-06-10
> **审计者**: claude
> **范围**: `cg/` `cgnova/` 目录所有 .cxx / .h
> **目的**: 在不修改任何 bug 的前提下, 标记 code-gen 当前状态 / 缺失 / 改进点,
>          供使用者后续重写时参考

---

## 0. 整体结构 (CGBB 流程概览)

```
OCIR (TREE)
  │ CGIR_BUILDER::CG_convert_function
  ▼
CG_CFG (= SSA_CFG 同模板, BB 类型为 CGBB)
  │  CGBB 中是 CGOP (CG-level instruction) 序列
  │  TN = Temporary Node (代替 SSA 的 CODEREP)
  ▼
CG_LIVE_RANGE::Analyze_live_range  ← STUB
CG_REG_ALLOC::Register_allocate     ← naive linear scan
  │ 包含 1) 频次统计  2) 干涉图 TODO  3) linear alloc  4) spill rewrite
  ▼
DATA_LAYOUT::Calculate_stack_frame_size / Recalibrate_offset
  ▼
CG_EMITTER::Emit_tree
  ▼
ARMv7 EABI Assembly
```

---

## 1. 文件清单 + 现状打分

| 文件 | 行数 | 现状 | 缺什么 |
|------|------|------|--------|
| `cg/cg_main.cxx` | 313 | ✅ OK | 无 |
| `cg/cg_main.h` | 19 | ⚠️ 极简 | 缺 CGBB / VDG / Pass 接口 |
| `cg/cgir.cxx` | 1467 | 🔧 部分 | 见 §2 |
| `cg/cgir.h` | 425 | 🔧 部分 | 缺 VDG / InterferenceGraph |
| `cg/cg_basic.h` | 250 | ⚠️ 占位 | 缺 ARMv7 calling convention 描述 |
| `cg/cg_opc.h` | 36 | ✅ OK | 无 (CGOPC 枚举完整) |
| `cg/cg_variant.h` | 150 | ✅ OK | 缺 P221 转换表 |
| `cg/data_layout.cxx/.h` | 221/95 | 🔧 简单 | 缺 VLA / alloca / 可变长栈 |
| `cg/register.cxx/.h` | 952/485 | 🔧 复杂但局部 | 缺 callee/caller save 标记 + ABI 分类 |
| `cg/targ_abi.cxx/.h` | 85/231 | ✅ OK | 缺 AAPCS 全量实现 |
| `cg/targ_reg.h` | 191 | ✅ OK | 无 |
| `cg/targ_basic.h` | 35 | ✅ OK | 无 |
| `cg/arm_reg.h` | 70 | ✅ OK | 无 |
| `cg/tn.cxx/.h` | 439/330 | 🔧 部分 | 缺 def/use 链 |
| `cg/util.cxx/.h` | 93/11 | ⚠️ 极简 | 缺 BURS / peephole |
| `cgnova/cg_nova.cxx/.h` | ? | ❓ 待查 | 推测是 CG → NovaIR 转换器 |

**打分**:
- ✅ OK: 设计完整可用
- 🔧 部分: 有结构但有显著缺口
- ⚠️ 极简: 只有占位
- ❓ 待查: 未审计

---

## 2. 关键 Pass 现状

### 2.1 CGIR_BUILDER (IR → CGIR lowering)

**位置**: `cgir.cxx::CGIR_BUILDER::Handle_*`  /  `Expand_expr`  /  `Exp_op*` (line 1180–1500)

**现状**:
- `Handle_stid` / `Handle_ldid` / `Handle_iload` / `Handle_istore` 完整
- `Handle_lda` (取地址) 存在
- `Handle_ret_val` / `Handle_ret` / `Handle_goto` 完整
- `Handle_call` 简单 (只走 `BL` 指令, 不处理返回值/参数传 ABI)
- `Handle_func_body` 简单

**缺**:
- 表达式展开 (`Expand_expr` / `Exp_op*`) 对 FP / 向量 / 结构体返回 覆盖不足
- 缺 Address Mode 选择 (imm12 / [rN, #off] / [rN, rM] / 加载长 imm)
- 缺 peephole 优化 (如 `mov r0, r0` 删除, 冗余 `push/pop`)
- 缺 strength reduction (MPY → SHL+ADD, DIV → magic number)
- 缺分支优化 (cmp 0 消除, cc 复用)

**建议补的 Is_Trace**:
```c
Is_Trace(Tracing(COMPONENT_CG_CONV, TRACE_DATA),
         (TFile, "[CG-BUILDER] Handle_stid at BB %d, sym = %s\n", ...));
```

### 2.2 CG_LIVE_RANGE

**位置**: `cgir.cxx::CG_LIVE_RANGE::Analyze_live_range` (line 600–602)

**现状**: **完全 stub**, 函数体为空

**缺**:
- 反向数据流分析 (gen/kill/in/out)
- 不动点迭代
- 干涉图构造 (interference graph)
- live range 区间化 (first-def … last-use)

**详细 spec 见 `cg.spec.md` §3**

### 2.3 CG_REG_ALLOC (寄存器分配)

**位置**: `cgir.cxx::CG_REG_ALLOC::Register_allocate` (line 609–743)

**现状**:
```
naive linear scan:
  next_register = 1
  for each TN in tn_freq_map:
    if next_register >= 7: spill
    else: assign rN, next_register++
```

**缺 (一长串)**:
1. ❌ 干涉图 (`interference graph`) — TODO 在 line 642
2. ❌ Chaitin / Briggs graph coloring
3. ❌ 优先级启发式 (use-count, loop depth, ...)
4. ❌ coalescing (move-related TN 优先同色)
5. ❌ live range splitting (Chaitin '82)
6. ❌ callee/caller save 区分
7. ❌ 多寄存器类 (int / float / vector) 分别分配
8. ❌ 重新着色 (Rematerialization)
9. ❌ spilling cost model (Chaitin / Belady min-distance)
10. ❌ 寄存器 hint (loop-invariant / argument)

**Spill 部分 (Process_spill_op, line 1105)**:
- 只在 TN 有 `TN_SPILL` flag 时才处理
- 寄存器选择写死: `8 / 10 / 7` (line 1117)
- 没有可重入; 多次 spill 可能冲突

**详细 spec 见 `cg.spec.md` §4**

### 2.4 DATA_LAYOUT

**位置**: `data_layout.cxx::DATA_LAYOUT::Calculate_stack_frame_size` / `Allocate_object` 等

**现状**:
- 单函数 frame 简单计算 (局部 + 形参 + pad)
- 没用 alignment-aware 策略
- 没区分 caller-saved / callee-saved 区域

**缺**:
- 缺 VLA / alloca / dynamic stack
- 缺 longjmp 安全 (alloca 在 frame 内, 还是另开?)

### 2.5 CG_EMITTER (CGIR → asm)

**位置**: `cg_main.cxx::CG_EMITTER::Emit_tree` (line 171–276)

**现状**:
- 逐 CGBB / 逐 CGOP 翻译
- prologue/epilogue 已实现
- label 发射已实现
- 但 emit_operand 里 (line 950–1000) 的 TN → 字符串翻译是手写, 不易扩展

**缺**:
- 缺 instruction scheduling (调度) — 不在 CG 中做 (前文已说 scope 之外)
- 缺 FP / 向量 / NEON 指令发射
- 缺 PIC / GOT 表发射 (PIC 模式)
- 缺 reloc 处理 (目前只硬写 `.word`)
- 缺调试信息 (DWARF)

### 2.6 CG_COMPOSITE / CGnova (输出到 NovaIR?)

**位置**: `cg_main.cxx::CG_COMPOSITE`

**现状**:
- 只作为 CGIR / emitter / builder / lra / reg_alloc 的容器
- 没有 pass pipeline (单 pass 链)

**缺**:
- 缺一个真正的 pass manager (e.g. `CG_run_pass("Lower", ...)`)

---

## 3. Value Dependency Graph (VDG) — 缺失!

> 实际项目**没有**实现 VDG (`grep -r "VDG" cg/` 0 命中)。
> 这是 cggra CG 阶段的关键数据结构, 用于支持 DFA / 死代码消除 / 寄存器分配前的 dataflow 准备。

**VDG 应该有的内容** (参见 Muchnick '97 §6):

```
VDG = (V, E)
  V = (CGBB, CGOP, TN, CGLABEL) ∪ (VAL = 表达式 / 常数)
  E = (Use-Def) ∪ (Def-Use) ∪ (Control-Flow) ∪ (Region)

V 又分为:
  CGBB-Vertex    (BB 节点)
  CGOP-Vertex    (指令节点)
  TN-Vertex      (单条 TN 的 def)
  CGLABEL-Vertex (BB 入口/出口)
  Value-Vertex   (immediate, register-class, ...)
```

**构建步骤** (CGIR 阶段):
1. 扫描所有 CGOP, 给每个 CGOP_Vertex 分配 id
2. 给每个 TN 的 def 分配 TN_Vertex
3. 给每个 BB 分配 CGBB_Vertex
4. 连接 Use → Def (TN-Use-CGOP-Def)
5. 连接 Def → Use (反边)
6. 连接 BB → BB (Control-Flow)
7. 连接 BB → CGOP (containment)

**对其它 pass 的支持**:
- DFA (data-flow analysis): 在 VDG 上做 graph-reachability
- Dead Code Elimination: 在 VDG 上做 backward reachability from side-effect roots
- Register Allocation: VDG 的 "live range" 节点 = 干涉图构建输入
- Scheduling: VDG 的 critical path = priority

**实现位置建议**: `cg/vdg.h` + `cg/vdg.cxx`

---

## 4. 用户特别提到的关键问题

### 4.1 Register Allocator: "很 naive"

**确认**: 当前的 allocator 是 linear scan 的最简化版, 没有干涉图, 没有 spill 优化.

详见 `cg.spec.md` §4 的 spec — 用户可参考 Chaitin'81 / Briggs'92 / Wimmer'10 (LLVMA).

### 4.2 缺 logging / Print / assertion

**已补** (本审计 pass):
- ✅ `CG_REG_ALLOC::Print_freq_map` / `Print_live_range`
- ✅ `CG_LIVE_RANGE::Print`
- ✅ `CG_REG_ALLOC::Register_allocate` 加 `Is_Trace` 入口
- ✅ `Register_allocate` 加 `Is_True` (`tn != nullptr`, `TN_register(tn) == 0`)
- ✅ `Analyze_live_range` stub 加 `Is_Trace(TRACE_WARN)` 提示

**已改 fprintf → Is_Trace**:
- ✅ `cgir.cxx:619` (BB 头 trace)
- ✅ `cgir.cxx:683` (post-LRA-spill trace)
- 其余 fprintf 多在 `Emit_*` / `Print` 中, 那是 asm 输出或 print 方法, 不属于 logging

### 4.3 缺 CGBB 整体的 pass 描述

**现状**: 没有显式的 pass pipeline. 一次性 `CG_process_funcs` 走完 convert + alloc + layout + emit.

**建议** (用户后续可重构):
```cpp
class CG_PASS {
public:
  virtual void Run(PU_INFO *pu, CGIR *cgir, OPT_CONFIG &cfg) = 0;
  virtual const char *Name() = 0;
};

// Pass chain example:
CG_PASS *chain[] = {
  new CG_Build_Pass(),         // IR → CGIR
  new CG_Chaitin_Pass(),       // GRA (interference + coloring)
  new CG_LinearScan_Pass(),    // LRA for remaining
  new CG_SpillRewrite_Pass(),  // rewrite to load/store
  new CG_Layout_Pass(),        // stack frame
  new CG_Emit_Pass(),          // to asm
};
```

---

## 5. 已知问题 / TODO (不修, 仅记录)

| 问题 | 位置 | 备注 |
|------|------|------|
| `Register_allocate` 没建干涉图 | `cgir.cxx:642` | TODO 注释 |
| `_tn_live_range` 用 `((bb<<32) | cgop_id)` 编码, 若 cgop_id > 32-bit 就会错 | `cgir.cxx:1098` | uint64 设计 |
| `Spill_tn` 寄存器选择写死 `8 / 10 / 7` | `cgir.cxx:1117` | 应通过 ABI 推导 |
| `Process_spill_op` 可能多次选择相同寄存器 | `cgir.cxx:1147-1169` | 没做 anti-alias 检查 |
| `CG_LIVE_RANGE::Analyze_live_range` 是空函数 | `cgir.cxx:600` | STUB |
| `Get_addr_label` malloc 100 字节不确定够 | `cgir.cxx:1029` | 应换 std::string |
| 没处理 long long (8-byte) 常数 | `cgir.cxx:201-203` | mov+movt 拼 32-bit 不够 |
| 没处理 vargs | `cg_main.cxx:Handle_call` | 当前只支持定参 |
| `Emit_function` 用 IR 直接输出, 不用 CGIR | `cg_main.cxx:167` (TODO 注明) | 不一致 |
| CGBB 缺 `Move_stmt_to_before/after` 的实际使用 | `cgir.cxx:707-717` (被注释) | work_list 没人用 |

---

## 6. 给后续使用者的建议

1. **不要直接 patch `Register_allocate`**: 应该写一个 `CG_REG_ALLOC_V2`, 跟旧版并存
2. **CG_LIVE_RANGE::Analyze_live_range**: 先写 reverse data-flow, 然后构造 interference graph
3. **VDG**: 重写 builder 时把 CGBB → CGOP → TN 三层都建顶点, 之后所有 data-flow pass 都跑 VDG
4. **pass pipeline**: 加 `CG_COMPOSITE::Add_pass` / `Run_passes`, 取代 `CG_process_funcs` 一次性
5. **logging** 全部走 `Is_Trace(Tracing(COMPONENT_CG_LRA, TRACE_DATA), (TFile, ...))`
6. **spill 寄存器**: 走 ABI 推导 (r12 是 IP scratch, 优先用 r12, 再考虑 callee-saved 区域)
7. **测试**: 用 `testcase/simple/*.c` 多写几个, 跑 `--beloglevel=255` 看 LRA dump
