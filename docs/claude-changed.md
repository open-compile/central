# Claude Changed Files (Reference Implementation)

> **目的**：本目录列出自 2026-06-08 起由 claude 完全从 0 写、或者在已有结构上做了显著修改的文件。
> 这些修改作为 SSA / 基础优化 pass 的**参考实现 (reference)** 存在，使用者会自己重写算法。
>
> **约定**：
> - ✅ **保留** = 可直接用、值得参考（数据结构、辅助工具）
> - 🔁 **重写** = 算法部分，使用者会自己实现，建议当作参考资料
> - 🔧 **修改** = 在已有结构上扩展
> - ❌ **删除候选** = 仅作为占位的 stub

---

## SSA Pipeline 总览

> 使用者**会重写**：`opt_cfg_builder.cxx`、`opt_dom.cxx`、`opt_ssa.cxx` (Rename/Place_phi 部分)、`opt_dce.cxx`、`opt_cprop.cxx`
> 使用者**可保留**：`opt_coderep.h/.cxx`、`opt_stab.h/.cxx`、`opt_stmt.h/.cxx`、`opt_walk.h` (数据结构 + 模板 walker)

```
TREE (IR) ──► CFG_FROM_IR::Build
            └─► SSABB (CFG)
                └─► DOM_BUILDER::Build
                    └─► Cooper idom + Cytron DF
                        └─► SSA_RENAME (Cytron, DFS dom tree)
                            └─► STMTREP / CODEREP / PHI_NODE (SSA form)
                                ├─► CPROP (Fold + fixed-point)
                                ├─► DCE (mark-sweep)
                                └─► Opt_destruct_ssa (phi → copy)
```

---

## 一、纯新建文件 (Created from scratch)

### 📁 `opt/opt_coderep.h` / `opt/opt_coderep.cxx` — ✅ 保留
- **类**：`CODEREP`、`PHI_NODE`、`CR_POOL`
- **作用**：SSA 形式的 IR 表达式节点；参照 Open64 `CODEREP` 设计，用 union 紧凑存储
- **关键函数**：
  - `CODEREP::Make_const/lda/var/ivar/op` — 工厂方法
  - `CODEREP::Print` / `Print_pretty` — 调试输出
  - `PHI_NODE::Print` — phi 节点打印
  - `CR_POOL::Alloc_coderep` — 内存池分配

### 📁 `opt/opt_stab.h` / `opt/opt_stab.cxx` — ✅ 保留
- **类**：`OPT_VAR`、`VER_ENTRY`、`OPT_STAB`
- **作用**：把 `ST_IDX` 映射到紧凑的 `AUX_ID`，并维护版本表 (ver_tab)
- **关键函数**：
  - `OPT_STAB::Add_var` / `Lookup` / `New_version`
  - `OPT_STAB::Ver` / `Var` — 访问
  - `OPT_STAB::Print` / `Print_verbose` — 调试

### 📁 `opt/opt_stmt.h` / `opt/opt_stmt.cxx` — ✅ 保留
- **类**：`STMTREP`
- **作用**：包裹 IR 语句；SSA 化之后所有语句都通过 STMTREP 表示
- **关键函数**：
  - `STMTREP::Print` / `Print_pretty`

### 📁 `opt/opt_walk.h` — ✅ 保留
- **作用**：通用模板 walker
- **关键函数**：
  - `Walk_cfg_rpo` — 按 RPO 顺序遍历 BB
  - `Walk_dom_tree` — 沿支配树 DFS
  - `Walk_cr_postorder` — 表达式后序遍历
  - `Compute_rpo` — 计算 RPO 编号

### 📁 `opt/opt_dbg.h` — ✅ 保留
- **作用**：共享宏（`SSA_DBAR` 分隔线）

### 📁 `opt/opt_print.cxx` / `opt/opt_print.h` — ✅ 保留
- **作用**：`SSABB` / `SSA_CFG` / DOM / DF 的 Print 实例化
- **关键函数**：
  - `Print_ssa_cfg(cfg, f, pretty)`
  - `Print_dom_tree(cfg, f)`
  - `Print_df(cfg, f)`

### 📁 `opt/opt_cfg_builder.h` / `opt/opt_cfg_builder.cxx` — 🔁 重写
- **类**：`CFG_FROM_IR`
- **作用**：从 IR TREE 构建 SSABB + STMTREP
- **关键函数**：
  - `CFG_FROM_IR::Build` — 主入口
  - `Build_function_body` — 处理 BLOCK 内的每条语句
  - `Lower_expr` / `Lower_stmt` — 表达式 / 语句降级到 CODEREP / STMTREP
  - `Fixup_branches` — label → BB 关联，建立 pred/succ
- **建议重写**：
  - `Lower_expr` 的 switch 覆盖不全（很多 OPR_ 没处理到）
  - `Build_function_body` 中 LABEL 的处理是 hack
  - 未处理 LOOP/EXC_SCOPE/REGION 等

### 📁 `opt/opt_dom.h` / `opt/opt_dom.cxx` — 🔁 重写
- **类**：`DOM_BUILDER`
- **作用**：计算支配树 (Cooper) 和 支配边界 (Cytron)
- **关键函数**：
  - `Compute_rpo` — 迭代 DFS 求 RPO
  - `Compute_idom` — Cooper 算法
  - `Intersect` — idom 求交
  - `Compute_dom_list` / `Compute_df` — 父-子链表 / DF
- **算法**：参考 Cooper, Harvey & Kennedy "A Simple, Fast Dominance Algorithm" (2001)
- **建议重写**：合并到 `opt_ssa.cxx`；考虑更高效的 Lengauer-Tarjan 或 Cooper 工程优化版

### 📁 `opt/opt_ssa.cxx` — 🔁 重写 (核心)
- **类**：`SSAIR`、`SSA_COMPOSITE`、`RENAMER`
- **作用**：SSA 主流程
- **关键函数**：
  - `SSAIR::Get_function` / `Goto_function` / `Print` / `Dump`
  - `SSA_COMPOSITE::Construct_function` — 5 步流程
  - `Collect_defs_bb` — 收集每个 aux 的 def BB
  - `Place_phi_node` — Cytron 的 `Has_already_on_stack` 优化版
  - `SSA_Rename` — DFS 支配树，version stack
  - `SSA_Rename_rhs` — 重命名表达式中的 use
  - `Opt_destruct_ssa` — phi → copy STID
  - `Opt_build_ssa_all` / `Opt_run_cprop` / `Opt_run_dce` / `Opt_destruct_ssa_all` — 外层接口
- **算法**：
  - SSA construction 走标准 Cytron 路线
  - Destruction 走 phi→copy + work-list 简化版
- **建议重写**：
  - `SSA_Rename` 中 BFS/DFS 顺序是教科书写法
  - Destruction 走简化版 (Sreedhar 的 coalescing 没做)
  - 可考虑 Briggs/Chow 的 out-of-SSA

### 📁 `opt/opt_dce.cxx` / `opt/opt_dce.h` — 🔁 重写
- **作用**：Mark-Sweep 死代码消除
- **关键函数**：
  - `Mark` / `Mark_stmt` / `Mark_cr_uses` / `Sweep`
- **算法**：以 side-effect 根为 mark 起点（call/io/store），递归 mark 使用，删除未 mark 的 STMT
- **建议重写**：
  - 没有做 precise liveness，仅做可达性 mark
  - 没有做部分死代码 (PDE)
  - control-flow DCE（不可达 BB）也没做

### 📁 `opt/opt_cprop.cxx` / `opt/opt_cprop.h` — 🔁 重写
- **作用**：常量折叠
- **关键函数**：
  - `Fold` — 递归 fold CODEREP
  - `Eval_bin` / `Eval_un` — 二元 / 一元求值
  - `Run` — RPO 顺序的 fixed-point 迭代
- **算法**：纯表达式 fold；没有做 value number / lattice-based propagation
- **建议重写**：
  - 真正的 SSA 常量传播需要 lattice (Wegman-Zadeck)
  - 配合 DCE 才能传播到使用点

### 📁 `testcase/simple/opt_basic.c` — ✅ 保留 (testcase)
- 测试 DCE / CPROP / 死分支消除
- 已知 bug：当前 SSA pipeline 在多 BB 函数上 DOM/Phi 会 crash — **待 fix**

---

## 二、修改文件 (Modified)

### 📁 `opt/opt_basic.h` — 🔧 修改
- **改动**：
  - `SSAIR` 改用 `OPT_STAB*`（之前是 `OPT_SYMTAB*`）
  - `SSAIR::Symtab_map` 访问器
  - `SSAIR::Get_function` 改为 lazy 创建
  - `SSA_CFG_BB_BASE<NODE_TYPE>` 增 `_phi_list` / `_stmtreps` 成员
  - 增 `Print` / `Print_pretty` / `Print_dom` / `Print_df`
  - `SSA_COMPOSITE` 增 `_config` 字段和 `Set_config` / `Get_config`
- **保留**：`SSABB` / `SSA_CFG` typedef

### 📁 `opt/opt_main.cxx` — 🔧 修改
- **改动**：
  - `BE_EXTERNAL_MAIN_NAME` 在 `LEVEL_MID` 之后插入 SSA pipeline：
    ```cpp
    Opt_build_ssa_all(File(), LEVEL_MID, conf);
    Opt_run_cprop(File(), conf);
    Opt_run_dce(File(), conf);
    Opt_destruct_ssa_all(File(), LEVEL_MID, conf);
    ```
  - trace 联动：开 debug 时自动 enable 所有 dump
- **删除**：`Opt_lower(LEVEL_HIGH)` 重复调用

### 📁 `opt/CMakeLists.txt` — 🔧 修改
- **改动**：增加新文件到 opt library

### 📁 `common/options.h` — 🔧 修改
- **改动**：
  - 扩展 `OPT_KIND` 加 SSA/CPROP/DCE 等
  - 新增 `enum SSA_DUMP_LEVEL`
  - 新增 `struct OPT_PARAM` / `struct OPT_CONFIG`
  - `COMPILER_CONFIG` 增 `opt_cfg` 字段
- **保留**：`OPT_KIND_ARITH` / `OPT_KIND_LDID_CONST`

### 📁 `common/cfg_common.h` — 🔧 修改
- **改动**：增 `Internal_bb_list()` / `Internal_edges()` 访问器
- **目的**：让 `CFG_FROM_IR` 能直接 push_back BB

### 📁 `common/be_export.h` — 🔧 修改
- **改动**：增 include guard + `#include "options.h"`
- **目的**：让 cg/ opt/ 能用 COMPILER_CONFIG

---

## 三、未修改的相关文件 (供使用者参考)

- `opt/opt_ssa.h` — 只有 forward decl，使用者重写时可作模板
- `opt/opt_main.h` — 接口
- `opt/opt_cprop.h` / `opt_dce.h` — 接口，使用者重写时可改签名

---

## 四、Spec / 设计文档

- `docs/spec.md` — SSA + 基础优化 pass 完整设计 spec
- `docs/spec-old.md` — 改造前的现状记录

---

## 五、已知问题 / TODO (供使用者重写时 fix)

| 问题 | 位置 | 建议 |
|------|------|------|
| DOM builder 在多 BB 函数 crash | `opt_dom.cxx::Compute_idom` | Cooper 是对的，可能迭代顺序问题，调试建议加 fprintf |
| CFG_FROM_IR 选错 BLOCK | `opt_cfg_builder.cxx` | IR 形式是 `FUNC_ENTRY→BLOCK(empty)→BLOCK(stmts)`，现在硬选 kids>0 |
| Rename 后 OP 子节点版本号未刷新 | `opt_ssa.cxx::SSA_Rename_rhs` | 需要递归 push new version on rhs |
| PHI 节点 res ver 未被 New_version 推 | `opt_ssa.cxx::Place_phi_node` | 应该在 place 时就 New_version |
| Destruction 的 coalescing 缺失 | `opt_ssa.cxx::Opt_destruct_ssa` | 现在是 naive 1-1 展开 |
| DCE 没处理 control flow (unreach BB) | `opt_dce.cxx` | 需要先做 unreachable BB elimination |

---

## 六、文件头标记说明

> 每个 claude 改过的文件开头都有类似下面的注释块（"CLAUDE-MARKER" 标识）。
> 数据结构（标 ✅）会标 `STATUS: KEEP`；算法（标 🔁）会标 `STATUS: REWRITE-REFERENCE`。
> 使用者重写算法时，可以保留数据结构 + 重新实现方法体。
