# Central 编译器现状记录（spec-old.md）

> 记录时间：2026-06-09  
> 目的：记录当前已实现内容，作为后续开发的基线参考

---

## 一、整体编译流程

**入口**：`driver/main.cxx`，核心函数 `Execute()`。

```
源代码
  └─ FE（frontend）     fe/       femain()
  └─ BE（optimizer）    opt/      BE_EXTERNAL_MAIN_NAME()
  └─ CG（code gen）     cg/       CG_full_process()
  └─ ASM（assembler）             系统汇编器
```

BE 内部的降级流水线（`opt/opt_main.cxx`）：

```
VHIGH → HIGH → HIGH（重复，疑似 bug）→ MID
  → Opt_build_ssa_all（SSA 构建）
  → LOW → VLOW → CGIR
```

**已知问题**：
- `opt_main.cxx:42/46`：`Opt_lower(LEVEL_HIGH)` 被调用了两次，第二次应为笔误或应改为其他 level。
- `Opt_build_ssa_all` 之后缺少对应的 `Opt_destruct_ssa_all` 调用，SSA 形式在进入 LOW 降级前未被析构。

---

## 二、前端（FE）

**状态：✅ 完整**

| 文件 | 说明 |
|------|------|
| `fe/grammar.cpp` | Bison 生成，约 80,784 行，完整的 C 子集语法 |
| `fe/token.cpp` | Bison 生成，约 57,203 行，词法分析 |
| `fe/parser_tree.h` | AST 节点定义（见下表） |
| `fe/fe_main.h` | FE 对外接口 |

### AST 节点（`fe/parser_tree.h`）

| 节点 | 行号 | 说明 |
|------|------|------|
| `NExpression` | 61 | 表达式基类 |
| `NStatement` | 81 | 语句基类 |
| `NInteger` / `NDouble` | 128 / 100 | 整数 / 浮点常数 |
| `NIdentifier` | 160 | 标识符 |
| `NBinaryOperator` | 314 | 二元运算符 |
| `NUnaryOperator` | 353 | 一元运算符 |
| `NMethodCall` | 270 | 函数调用 |
| `NAssignment` | 382 | 赋值 |
| `NArrayIndex` / `NArrayAssignment` | 802 / 849 | 数组读写 |
| `NIfStatement` | 666 | if-else |
| `NForStatement` | 712 | for 循环 |
| `NBlock` | 415 | 代码块 |
| `NVariableDeclaration` | 477 | 变量声明 |
| `NFunctionDeclaration` | 531 | 函数声明 |
| `NStructDeclaration` | 591 | 结构体声明 |

- 使用 `shared_ptr` 管理内存
- 每个节点支持 `jsonGen()` 序列化和 `print()` 打印

---

## 三、公共 IR（common）

**状态：✅ 基本完整**

### 3.1 IR 节点（`common/tree.h`）

- **`IRNODE`**（line 84）：IR 基本节点
  - `opcode`：操作码（`OPCODE` 枚举）
  - `extra1.combined.c1`：加载偏移、标签号、标志位
  - `extra1.combined.c2`：`ST_IDX`（符号索引）或 `TY_IDX`（类型索引）
  - `extra3`：操作数索引 `IRNODE_IDX kids[2]` 或常数值

- **`TREE`**（line 211）：函数级 IR 树容器
  - 内部使用 `tree<IR_TREE_ELEM>` 模板库
  - 存储 `std::vector<IRNODE>`
  - 提供迭代器：`IR_ITER`、`IR_PRE_ITER`、`IR_POST_ITER`
  - 方法：`Insert_stmt_to_block()`、`Set_operand()`、`Remove_node_recursive()` 等

### 3.2 符号表（`common/symtab.h`）

| 结构体 | 说明 |
|-------|------|
| `TY` | 类型：`SCALAR/ARRAY/STRUCT/POINTER/FUNCTION/VOID`，含 size、align、mtype |
| `ST` | 符号：name_idx、sym_class、storage_class、type、offset、pu |
| `PU` | 程序单元（函数）：prototype、flags、pu_info_idx |
| `PU_INFO` | 函数详细信息：proc_sym、entry（`TREE*`）、scope |
| `SCOPE` | 作用域：st_tab、label_tab、preg_tab、inito_tab |
| `LABEL` | 标签：name_idx、kind、flags |
| `ARB` | 数组维度边界：flags、lbnd/ubnd/stride（常数或变量） |

访问入口：`FILE_MANAGER::Tables()` → `FILE_SYMTAB*`；函数局部信息通过 `PU_INFO::scope` 访问。

---

## 四、基本块与控制流图（CFG）

**状态：✅ 数据结构完整，支配树计算算法缺失**

### 4.1 `CFG_BB_BASE<NODE_TYPE>`（`common/cfg_common.h:126`）

| 字段 | 类型 | 说明 | 计算状态 |
|------|------|------|---------|
| `_stmts` | `vector<NODE_TYPE*>` | BB 内语句 | ✅ 正常填充 |
| `_flags` | `UINT32` | `BB_FLAG` 枚举（ENTRY/EXIT/CALL/…） | ✅ |
| `_label_id` | `UINT32` | 对应标签 | ✅ |
| `_preds` / `_succs` | `list<BB_TYPE*>` | 前驱 / 后继 | ✅ |
| `_idom` | `BB_TYPE*` | 直接支配者 | ❌ 从未被计算填充 |
| `_ipdom` | `BB_TYPE*` | 直接后支配者 | ❌ 从未被计算填充 |
| `_dom_list` / `_pdom_list` | `list<BB_TYPE*>` | 支配树子节点 | ❌ 从未被计算填充 |
| `_df_list` | `list<BB_TYPE*>` | Dominance Frontier | ❌ 从未被计算填充 |
| `_cd_list` | `list<BB_TYPE*>` | 控制依赖 | ❌ 从未被计算填充 |

`Dominate()` / `Post_dominate()` 方法（line 274/284）通过遍历 `_idom` 链判断支配关系，但 `_idom` 链从未建立，调用会触发 `AssertThat` 失败。

### 4.2 `CFG_BASE<NODE_TYPE, BB_TYPE>`（`common/cfg_common.h:52`）

- `_bb_list`：BB 向量
- `_edges`：`vector<set<CFG_BB_IDX>>`，存储 BB 间出边
- 方法：`Add_bb()`、`Add_succ()`、`Is_succ()`、`Node(idx)`

### 4.3 CFG 变体

| 类型别名 | 模板实例 | 用途 |
|---------|---------|------|
| `SSA_CFG` | `SSA_CFG_BASE<SSANODE>` | SSA 转换阶段 |
| `CG_CFG` | `CFG_BASE<CGOP, CG_CFG_BB>` | 代码生成阶段 |

### 4.4 `CG_CONV_EXTRAINFO`（`common/cfg_common.h:512`）

CG 侧负责 label → BB 映射和 goto 前驱/后继修复，`Fixup_pred_succ()` 已实现。SSA 侧没有等价机制。

---

## 五、SSA 构建

**状态：❌ 完全未实现，仅有空框架**

### 5.1 数据结构（`opt/opt_basic.h`）

| 类 | 问题 |
|----|------|
| `SSANODE`（line 30） | 仅包装 `IRNODE_IDX`，**无版本号（version）**，无 phi 节点表示 |
| `OPT_SYMTAB`（line 46） | 仅 `vector<ST_IDX>`，**无版本计数器、无重命名栈** |
| `CODE_STORE`（line 40） | 完全空类，无任何字段或方法 |
| `SSA_CFG_BB_BASE`（line 76） | 继承 `CFG_BB_BASE<SSANODE>`，无额外字段 |
| `SSAIR`（line 96） | 容器结构完整（函数→CFG 映射），但 CFG 内 BB 从未被填充 |
| `SSA_COMPOSITE`（line 174） | 主控类，所有核心方法均未实现 |

### 5.2 各方法实现状态（`opt/opt_ssa.cxx`）

| 方法 | 行号 | 状态 |
|------|------|------|
| `SSAIR::Goto_function()` | 64 | ✅ 初始化函数上下文（创建 CFG/SYMTAB/CODE_STORE） |
| `SSAIR::Print()` | 37 | ✅ 打印函数 |
| `SSA_COMPOSITE::Construct_function()` | 103 | ⚠️ 框架存在，但仅调用空函数 |
| `SSA_COMPOSITE::Collect_defs_bb()` | 84 | ❌ 函数体为空 |
| `SSA_COMPOSITE::Place_phi_node()` | — | ❌ 在头文件中声明，.cxx 中无实现 |
| `SSA_COMPOSITE::SSA_Rename()` | 88 | ❌ 函数体为空 |
| `SSA_COMPOSITE::SSA_Rename_rhs()` | — | ❌ 在头文件中声明，.cxx 中无实现 |
| `SSA_COMPOSITE::Verify_stack()` | 92 | ❌ 直接返回 `TRUE`，无任何验证 |
| `Opt_destruct_ssa()` | 152 | ❌ 函数体为空 |

### 5.3 SSA 构建完整流程（均缺失）

```
1. 支配树计算（Lengauer-Tarjan）       → _idom 未填充
2. Dominance Frontier 计算             → _df_list 未填充
3. IR → SSA CFG 的 BB 填充             → SSA_CFG 内无 BB 内容
4. Collect_defs_bb()：收集变量定义点    → 空实现
5. Place_phi_node()：在 DF 处插 phi    → 未实现
6. SSA_Rename()：DFS 支配树重命名      → 空实现
7. Opt_destruct_ssa()：phi→copy，out-of-SSA → 空实现
```

---

## 六、代码生成（CG）

**状态：✅ 基本完整**

| 文件 | 说明 |
|------|------|
| `cg/cgir.h` / `cgir.cxx` | CGIR 主体，IR → CG 指令展开（约 1,467 行） |
| `cg/tn.h` / `tn.cxx` | Temporary Node（虚拟寄存器）定义与操作 |
| `cg/register.h` / `register.cxx` | 寄存器分配（活跃范围 + 着色框架，约 952 行） |
| `cg/data_layout.h` / `data_layout.cxx` | 栈帧布局计算 |
| `cg/cg_main.cxx` | CG 主函数，串联各子模块 |
| `cg/cg_basic.h` | `CGOP` / `CGOPC` 指令枚举 |

`CGIR_BUILDER` 已实现的 IR 展开操作：`Handle_stid`、`Handle_ldid`、`Handle_iload`、`Handle_istore`、`Handle_lda`、`Handle_call`、`Handle_goto`、`Handle_ret`、`Expand_expr`。

目标平台：**ARMv7-EABI 32-bit**。

---

## 七、完成度总览

| 模块 | 完成度 | 备注 |
|------|-------|------|
| 前端 FE | ✅ ~100% | Bison 词法/语法完整，AST 构建完成 |
| IR 树结构 | ✅ ~90% | IRNODE/TREE 定义完整，树操作基本完善 |
| 符号表 | ✅ ~95% | TY/ST/PU/SCOPE 完整实现 |
| BB 数据结构 | ✅ ~95% | 字段齐全，pred/succ 操作正常 |
| 支配树计算 | ❌ 0% | 字段存在但计算算法完全缺失 |
| SSA 构建 | ❌ ~5% | 框架骨架存在，核心算法全部缺失 |
| IR 降级（opt） | ⚠️ ~40% | 框架存在，部分 lowering 实现，有重复调用 bug |
| 代码生成 | ✅ ~70% | IR 展开、栈帧布局基本完整 |
| 寄存器分配 | ⚠️ ~60% | 活跃范围框架存在，着色细节待确认 |

---

## 八、待实现的关键工作（优先顺序）

1. **修复 opt_main.cxx**：去掉重复的 `Opt_lower(HIGH)`，补充 `Opt_destruct_ssa_all()` 调用
2. **支配树计算**：实现 Lengauer-Tarjan（或迭代 dataflow），填充 `_idom`、`_dom_list`
3. **Dominance Frontier**：基于支配树计算，填充 `_df_list`
4. **IR → SSA CFG 填充**：将 IR 语句按控制流划分写入 `SSA_CFG` 的 BB 中
5. **`SSANODE` 扩展**：加入版本号字段，定义 phi 节点表示
6. **`OPT_SYMTAB` 扩展**：每个变量维护重命名栈 `stack<UINT32>`
7. **`Collect_defs_bb()`**：遍历 IR，记录每个变量的定义 BB 集合
8. **`Place_phi_node()`**：Cytron-Ferrante 算法，在 DF 处插入 phi 节点
9. **`SSA_Rename()`**：DFS 支配树，按版本栈重命名所有 LDID/STID
10. **`Opt_destruct_ssa()`**：将 phi 节点转为 copy 语句，恢复普通 IR
