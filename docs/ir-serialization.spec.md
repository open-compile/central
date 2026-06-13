# 二进制 IR Dump / Load — Spec & 使用手册

> **目的**：为 OCC 编译器添加"在任意 pipeline 阶段把 in-memory IR 序列化到磁盘 / 从磁盘恢复"的能力，
> 让调试、对比、reduce 与端到端测试都不必每次从源码重跑一整条 FE → OPT → CG。
>
> **范围**：MVP 覆盖 `FILE_MANAGER` 全状态（symtab + 每个函数的 TREE），不含 SSAIR / CGIR
> （这些是 OPT/CG 的派生数据，能从 TREE 重新构造；且 CG 还在迭代）。
> 后续以 "section kind 预留 + per-section version" 的方式扩展。
>
> **状态**：v1，已实现 + 已通过 simple 测例端到端 .s diff（13/14 通过，1 失败为 OPT pre-existing bug）。

---

## 1. Why

OCC 当前只能在内存里跑完整条 FE → OPT → CG。要调试中间某个阶段、压一个 pass 的前/后状态、
或者 bisect 一个 lowering 改动，原本得每次源码重新走到那个点。`common/symtab.cxx:386-396`
之前有三个 stub `Open_ir_file / Create_ir_file / Write_data_to_file`，但全是 `AssertThat(FALSE, ...)`。

**关键观察**：所有"数据元素"（`TY` / `ST` / `INITV` / `IRNODE` / `LABEL` / `PREG` / `ARB`
/ `TYLIST` / `PU` / `INITO`）**全部由 `*_IDX (UINT32)` 和标量字段组成，没有任何 raw pointer**。
带 pointer 的只在容器层：

- `tree<IRNODE_IDX>`（拓扑：5 个 sibling/parent/child 指针）
- `GROWING_TABLE` 内部 `std::vector<void*>`
- `FILE_SYMTAB` 持有的 9 个 `*_tab` 指针 + `SCOPE_MANAGER::_in_memory_function_info`

也就是说：**元素的二进制 dump 接近"struct 字节复制"**，唯一需要重建 pointer 网络的只有
`tree<IRNODE_IDX>` 拓扑——而拓扑用 `(parent_pos, node_idx)` 列表就能完整表达。

---

## 2. 文件格式

### 2.1 总体布局（ELF 风格，尾部 section table）

```
+--------------------------------+ 0
|        FILE_HEADER (40B)       |
+--------------------------------+ 40
|  STRTAB section                |
|  TY / TYLIST / ARB / PU /      |
|  PU_INFO / ST / LABEL / PREG / |
|  INITO  global sections        |
|  FUNC_BLOB (concat sub-images) |
|  FUNC_TAB                      |
+--------------------------------+ section_table_offset
|  N × SECTION_ENTRY (24B each)  |
+--------------------------------+ EOF
```

为什么是尾部 section table？写入时不需要预知每个 section 大小，只在 `End_section` 收
`Tell()` 即可；同时未来想做 `mmap`-based loader 也能 O(1) 索引（未实现）。

### 2.2 `FILE_HEADER` (40 字节, offset 0)

| 偏移 | 大小 | 字段                    | 含义                                          |
| ---- | ---- | ----------------------- | --------------------------------------------- |
| 0    | 4    | magic                   | 固定 `"IRB1"` (`0x31425249` LE)               |
| 4    | 2    | file_format_version     | 顶层格式版本，v1 起步                         |
| 6    | 2    | flags                   | bit0=little-endian, bit1=has_strtab           |
| 8    | 8    | section_table_offset    | `Finish()` 时回填                             |
| 16   | 4    | section_count           | `Finish()` 时回填                             |
| 20   | 4    | target_arch_id          | 目标架构（暂未启用具体语义）                  |
| 24   | 8    | source_file_strtab_idx  | 输入文件路径在 STRTAB 里的偏移（保留位）      |
| 32   | 8    | reserved                | 0                                             |

### 2.3 `SECTION_ENTRY` (24 字节, 尾部 `section_count` 个)

| 偏移 | 大小 | 字段     | 含义                                        |
| ---- | ---- | -------- | ------------------------------------------- |
| 0    | 2    | kind     | `SECTION_KIND` (UINT16)                     |
| 2    | 2    | version  | per-section version (与 file version 解耦)  |
| 4    | 4    | flags    | 保留                                        |
| 8    | 8    | offset   | 绝对文件偏移                                |
| 16   | 8    | size     | 字节数                                      |

**Per-section version 是关键扩展点**：v2 想改 `IRNODE` 编码不需要动 `file_format_version`；只要 bump
对应 section 的 version，旧 reader 在该 section 上 fallback 即可。

### 2.4 `SECTION_KIND` (UINT16)

```
SK_STRTAB         = 1
SK_TY_TAB         = 2
SK_TYLIST_TAB     = 3
SK_ARB_TAB        = 4
SK_ST_GLOBAL      = 5
SK_LABEL_GLOBAL   = 6
SK_PREG_GLOBAL    = 7
SK_INITO_GLOBAL   = 8
SK_PU_TAB         = 9
SK_PU_INFO_TAB    = 10
SK_FUNC_TAB       = 11   // (pu_info_idx, sub_offset, sub_size) 数组
SK_FUNC_BLOB      = 12   // 所有函数 sub-image 的连续区域

// Phase 2 预留 (本次未实现)
SK_TCON_TAB       = 16
SK_FILE_INFO_TAB  = 17
SK_SSA_GLOBAL     = 32
SK_CGIR_GLOBAL    = 48
SK_DEBUG_INFO     = 64
```

### 2.5 函数 Sub-image (居于 `SK_FUNC_BLOB`)

每个函数占用一个 sub-image，结构和顶层文件类似：尾部 16B trailer 指向该 sub-image 的内部 section table。

```
+---------------------------------+ sub-image start
|  SUBSK_LOCAL_ST                 |
|  SUBSK_LOCAL_LABEL              |
|  SUBSK_LOCAL_PREG               |
|  SUBSK_LOCAL_INITO              |
|  SUBSK_IRNODES                  |
|  SUBSK_TREE_TOPO                |
+---------------------------------+
|  N × SUB SECTION_ENTRY          |
+---------------------------------+
|  16B trailer:                   |
|     u64 sub_table_offset        |
|     u32 sub_section_count       |
|     u32 magic_check             |
+---------------------------------+ sub-image end
```

```
SUBSK_LOCAL_ST    = 1
SUBSK_LOCAL_LABEL = 2
SUBSK_LOCAL_PREG  = 3
SUBSK_LOCAL_INITO = 4
SUBSK_IRNODES     = 5   // vector<IRNODE>
SUBSK_TREE_TOPO   = 6   // (parent_pos, node_idx) 列表

// Phase 2 预留
SUBSK_LOCAL_SSA   = 16
SUBSK_LOCAL_CGIR  = 17
```

`SK_FUNC_TAB` 里给每个函数记 `(pu_info_idx, abs_offset_into_file, sub_image_size)`，loader
直接 `Enter_subimage(off, size)` 切到对应 sub-image。

### 2.6 字段编码 — 各结构 v1 字节布局

> 全部字段固定宽度 + little-endian。**两级 IDX (`ST_IDX` 等 = `(raw<<8) | level`) 一律按 32-bit 完整值写入**，
> 不拆 level；决定写到哪张 section / sub-section 由"遍历哪张表"决定，表内位置就是 raw idx。

#### `TY` (`common/symtab.h:40-123`)

```
u64 size;
u8  kind;
u8  mtype;
u16 flags;
i32 align;
u32 u1_view;        // 4B union 整体视为 u32
u32 name_idx;
u32 u2_view;        // 4B union
u32 vtable;
```

#### `ST` (`common/symtab.h:195+`)

```
u32 name_idx;
u32 packed_bitfields;   // attr/sym_class/storage_class/export_class/tls_model
                        // 显式 pack/unpack, 不依赖编译器的位字段布局
u32 type;
u32 pu;
u32 pad;
u32 offset;
u32 base_idx;
u32 inito_idx;
```

#### `LABEL`

```
u32 name_idx;
u32 (kind & 0xff) | (flags << 8);
u32 temp_sym;
```

#### `PREG`

```
u32 name_idx;
u32 desire_reg_num;
```

#### `ARB` / `TYLIST` / `PU` / `PU_INFO`

逐字段写，全部 u32/u64，详见 `common/ir_serial.cxx`。`PU_INFO` 里 SCOPE 不在此 section dump，
而是 sub-image 里的 4 张 local 表。

#### `INITV` (`common/symtab.h:563+`)

```
u16 kind;
u16 repeat1;
// 按 kind dispatch 写 union body，所有 variant 一律 pad 到最大尺寸 (16B)，
// 让 INITO 数组步长固定。
```

#### `INITO`

```
u32 st_idx;
u32 count;
{ Encode_INITV ... }
```

#### `IRNODE` (`common/tree.h:84-200`, 72B)

v1 直接写 union 视图，不按 OPCODE 分支：

```
u32 opcode;
u64 extra1_view;       // combined u32x2 union 当 u64 写
u64 extra3_first_8B;   // kids[2] / const_val / ...
u64 extra3_next_8B;    // pragma 等
```

v1 的"等同 POD 直冗"换得简洁，未来想精简（按 OPERATOR 只写当前用到的字段）只需 bump
`SUBSK_IRNODES` version，不破坏 file format。

### 2.7 TREE 拓扑编码（核心）

唯一的 pointer 重建场景。`tree<IRNODE_IDX>`（`lib/tree_util.h`）每节点有 5 个原生指针。

#### `SUBSK_IRNODES`

```
u32 node_count
for i in [0, node_count):
    Encode_IRNODE(w, _ir_elem_tab[i])
```

#### `SUBSK_TREE_TOPO`

按 pre-order 遍历 `irtree`，每访问一个节点记录其遍历位置 `topo_pos`：

```
u32 edge_count          // = irtree.size()
u32 root_node_idx       // *(tree->Get_root())
for each pre-order visit (按 topo_pos 顺序):
    u32 parent_topo_pos     // root 用 0xFFFFFFFF
    u32 node_idx            // IRNODE_IDX，索引到 _ir_elem_tab
```

#### Decode 重建

```
1. 全部 IRNODES 读进 _ir_elem_tab
2. 维护 vector<IR_ITER> by_topo_pos (容量 = edge_count)
3. 顺序读 (parent_topo_pos, node_idx):
   - 若 parent_topo_pos == 0xFFFFFFFF -> tree.Set_root(node_idx)
   - 否则 parent_iter = by_topo_pos[parent_topo_pos];
          new_iter = irtree.append_child(parent_iter, node_idx);
   - by_topo_pos.push_back(new_iter)
4. pre-order 保证父先于子出现 -> 永远能取到 parent_iter
5. 校验 irtree.size() == edge_count
```

成本 8B/edge，简单且无歧义。

---

## 3. Decode 顺序约束

```
1. fm = File()  -> singleton 首次访问已自动跑过 FILE_SYMTAB::Initialize()
                   (symtab.cxx:351)，此时 strtab 与 _ty_tab 已种入内置 MTYPE。

2. 读 SK_STRTAB -> Strtab_replace(buf, used)
   * Strtab_replace 必须 malloc(used + STR_TABLE_BLOCK_SIZE) 留 headroom，
     否则 BE/CG 后续 Save_string 会立刻命中 line 125 的边界 assert。

3. 读各全局 section (顺序无强约束，但建议):
   TY -> TYLIST -> ARB -> PU -> PU_INFO -> ST -> LABEL -> PREG -> INITO
   * 每张表用 while (Length() < n) Add() 把空位 fill 到 dump 时的长度，
     再用 Set/iterator 覆盖每一项。
   * Initialize 写过的 idx 会被同步骤决定性产物等值覆盖。

4. 读 SK_FUNC_TAB, 每条 entry:
   - Enter_subimage(off, size)
   - 创建 PU_INFO; pu->scope.Init(proc_sym) 创建空的 4 张 local 表
   - 读 SUBSK_LOCAL_* 覆盖到 scope.*_tab
   - 读 SUBSK_IRNODES -> pu->entry->_ir_elem_tab
     * 注意: Decode 路径不能调 pu->entry->Initialize()，否则会预创建
       FUNC_ENTRY/BLOCK 三个 placeholder IRNODES, 与 Decode_irnodes 的 Create_node
       追加冲突, 让 verify 抱怨 "OPC_FUNC_ENTRY should not be in the body".
   - 读 SUBSK_TREE_TOPO -> 重建 pu->entry->Internal_tree()
   - _pu_info_tab->Set(pu_info_idx, pu_info)
   - _scope_manager->Finish_function(proc_sym, pu_info_idx) 注册映射
   - Leave_subimage()
```

---

## 4. 代码组织

### 4.1 新建文件（4 个）

| 文件                    | 行数 | 职责                                                                      |
| ----------------------- | ---- | ------------------------------------------------------------------------- |
| `common/ir_io.h`        | 243  | `IR_WRITER`/`IR_READER`、`SECTION_KIND`/`SUBSK_*`、`Dump_ir_file` 等入口  |
| `common/ir_io.cxx`      | 675  | I/O 层：section 表延后写尾、sub-image 嵌套、stdio FILE*                   |
| `common/ir_serial.h`    | 116  | 每个数据结构的 `Encode_X` / `Decode_X` 声明 + `SECVER_*` 常量             |
| `common/ir_serial.cxx`  | 832  | field-by-field encoder/decoder 实现                                       |

I/O 层（writer/reader + section bookkeeping）和 encoder 层（struct ↔ bytes）解耦，
v2 字段调整只影响 `ir_serial.cxx` 一个文件。

### 4.2 修改文件（6 个）

| 文件                       | 改动                                                                                                       |
| -------------------------- | ---------------------------------------------------------------------------------------------------------- |
| `common/symtab.h`          | 加 `Strtab_buffer()` / `Strtab_used()` 访问器                                                              |
| `common/symtab.cxx`        | `Strtab_replace` 加 headroom；三个 stub `Open_ir_file/Create_ir_file/Write_data_to_file` 接通到 `ir_io`    |
| `common/symtab_test.cxx`   | 新增 264 行 round-trip 单测（u32 写读 / strtab / 全局 ST/LABEL/PREG/INITO / mini-tree / 完整 sub-image）   |
| `common/options.h`         | `COMPILER_CONFIG` 加 `dump_ir_stages` / `load_ir_path` / `dump_textual_after_dump`                         |
| `common/CMakeLists.txt`    | 注册 `ir_io.cxx` / `ir_serial.cxx`                                                                         |
| `driver/main.cxx`          | 加 args 项 + `Run_component(COMPONENT_FE,...)` 中 `--load-ir` 短路 + 各阶段 `Maybe_dump_ir(...)` 注入       |
| `opt/opt_main.cxx`         | `BE_EXTERNAL_MAIN_NAME` 各 lower/verify 后插 `Maybe_dump_ir(...)`                                          |
| `test.sh`                  | 加 IRB round-trip 测试块                                                                                   |

### 4.3 接口入口

```cpp
// common/ir_io.h
void Maybe_dump_ir(const char *stage, COMPILER_CONFIG &conf, FILE_MANAGER *fm);
void Dump_ir_file(FILE_MANAGER *fm, const char *path);
void Load_ir_file(FILE_MANAGER *fm, const char *path);
```

`Maybe_dump_ir` 是命令行驱动用的 dispatcher：扫 `conf.dump_ir_stages`，命中 stage 名就调
`Dump_ir_file`，并在 `dump_textual_after_dump` 为真时同时打开 `path + ".txt"` 调 `fm->Print(txt_fp)`
作为 oracle。

---

## 5. 命令行使用

### 5.1 新增 flags（`compiler`）

| Flag                                  | 含义                                                                            |
| ------------------------------------- | ------------------------------------------------------------------------------- |
| `--dump-ir-after=<stage>=<path>`      | 在 `<stage>` 之后把 IR 写到 `<path>`；可重复指定多个 stage                       |
| `--load-ir=<path>`                    | 从 `<path>` 加载二进制 IR，跳过 FE                                              |
| `--dump-textual`                      | 每次 `--dump-ir-after` 同时写 `<path>.txt`（内容 = `FILE_MANAGER::Print` 输出）  |

### 5.2 支持的 stage 名（共 8 个注入点）

| stage 名         | 注入位置                                                              |
| ---------------- | --------------------------------------------------------------------- |
| `fe`             | `Run_component(COMPONENT_FE,...)` 内，`femain` 之后                    |
| `opt-high`       | `BE_EXTERNAL_MAIN_NAME` 中 `Opt_verify(LEVEL_HIGH, ...)` 之后          |
| `opt-mid`        | `Opt_verify(LEVEL_MID, ...)` 之后                                      |
| `opt-after-ssa`  | `Opt_destruct_ssa_all(...)` 之后                                      |
| `opt-low`        | `Opt_verify(LEVEL_LOW, ...)` 之后                                      |
| `opt-vlow`       | `Opt_verify(LEVEL_VLOW, ...)` 之后                                     |
| `opt-cgir`       | `Opt_verify(LEVEL_CGIR, ...)` 之后                                     |
| `pre-cg`         | `CG_full_process(config)` 之前                                        |

### 5.3 典型用法

#### 5.3.1 dump 一个阶段，再 reload 接着跑

```bash
# 1) 跑 FE，dump 到 /tmp/foo.fe.irb
compiler --feonly --dump-ir-after=fe=/tmp/foo.fe.irb foo.sy

# 2) 跳过 FE，从 IRB 出发跑 OPT + CG
compiler --load-ir=/tmp/foo.fe.irb -S foo.sy -o foo.s
```

#### 5.3.2 多阶段 snapshot + textual oracle

```bash
compiler -S foo.sy -o foo.s \
    --dump-ir-after=fe=/tmp/foo.fe.irb \
    --dump-ir-after=opt-after-ssa=/tmp/foo.ssa.irb \
    --dump-ir-after=pre-cg=/tmp/foo.precg.irb \
    --dump-textual
```

会同时产出 6 个文件：3 个 `.irb` + 3 个 `.irb.txt`。

#### 5.3.3 Round-trip 自检

```bash
compiler --feonly --dump-ir-after=fe=foo.irb --dump-textual foo.sy
compiler --load-ir=foo.irb --feonly --dump-ir-after=fe=foo.reload.irb \
         --dump-textual foo.sy
diff foo.irb.txt foo.reload.irb.txt    # 必须 empty
```

### 5.4 `irprint` — 离线打印工具

`compiler` 旁边还有一个独立可执行 `irprint`，专门用来事后解析 `.irb` 文件并打印内容。
不需要原始 `.sy` 源码，也不会触发 FE/OPT/CG。安装路径同 `compiler`：`build/driver/irprint`
（CMake 安装后位于 `bin/irprint`）。

#### 5.4.1 用法

```
irprint <input.irb>                    # 打印 FILE_MANAGER 全部内容到 stdout
irprint <input.irb> -o <out.txt>       # 写到文件
irprint <input.irb> --header-only      # 只打印 file header + section table
irprint <input.irb> -v                 # verbose tracing (与 driver -v 等价)
irprint -h                             # help
```

#### 5.4.2 典型场景

**(A) 看 .irb 长啥样、有哪些 section**：

```bash
$ irprint --header-only /tmp/foo.fe.irb
=== IRB File: /tmp/foo.fe.irb ===
file_format_version  = 1
flags                = 0x0003

Sections:
  kind   name           offset    size      version
  1      STRTAB         40        290       1
  2      TY_TAB         330       804       1
  3      TYLIST_TAB     1134      88        1
  ...
  11     FUNC_TAB       2490      24        1
  12     FUNC_BLOB      1750      740       1
```

**(B) 把 dump 的 IR 内容打到屏幕（等价于 `compiler --dump-textual`，但只对已有文件离线跑）**：

```bash
$ irprint /tmp/foo.fe.irb | less
=== IRB File: /tmp/foo.fe.irb ===
=======================================================
+ Dumping scope manager
=======================================================
...
```

**(C) bisect 流程：先 dump 多个阶段，再用 irprint 逐个比对**：

```bash
compiler -S foo.sy -o foo.s \
    --dump-ir-after=fe=foo.fe.irb \
    --dump-ir-after=opt-mid=foo.mid.irb \
    --dump-ir-after=opt-after-ssa=foo.ssa.irb \
    --dump-ir-after=pre-cg=foo.precg.irb

irprint foo.fe.irb     -o foo.fe.txt
irprint foo.mid.irb    -o foo.mid.txt
irprint foo.ssa.irb    -o foo.ssa.txt
irprint foo.precg.irb  -o foo.precg.txt
diff foo.fe.txt foo.mid.txt | less       # 看 OPT 改了什么
diff foo.mid.txt foo.ssa.txt | less      # 看 SSA 阶段改了什么
```

**(D) 验证 round-trip 是否字节相同（含工具自身的 oracle）**：

```bash
compiler --feonly --dump-ir-after=fe=foo.irb foo.sy
compiler --load-ir=foo.irb --feonly --dump-ir-after=fe=foo.reload.irb foo.sy
irprint foo.irb        -o foo.txt
irprint foo.reload.irb -o foo.reload.txt
diff foo.txt foo.reload.txt   # 应当 empty
```

#### 5.4.3 实现要点

`driver/irprint_main.cxx` 是个大约 130 行的小工具：

```cpp
FILE_MANAGER *fm = File();           // 单例首次访问 -> 自动 Initialize
Load_ir_file(fm, path.c_str());      // 复用 ir_io 的 loader
fm->Print(out);                      // 复用 FILE_MANAGER::Print
```

只链接 `common`（含 `ir_io` / `ir_serial` / `FILE_MANAGER`），**不**链接 `fe / opt / cg`。
`--header-only` 模式更轻：只跑 `IR_READER::Read_header()` 不解析任何 section 内容，
适合检查损坏文件 / 看 layout。

---

## 6. 测试体系

### 6.1 三层验证

| 层 | 内容 | 在哪儿 |
| --- | --- | --- |
| **L1 单元** | 每个 struct + 容器 round-trip：写一份 → 读回 → 字段逐项 `assert_eq`. 包括 TY/ST/LABEL/PREG/TYLIST/ARB、INITV 各 kind、IRNODE 几种 opcode、mini TREE 拓扑 | `common/symtab_test.cxx` |
| **L2 textual round-trip** | 对 `testcase/simple/*.sy`，第一次 dump 后 reload 再 dump，两份 `.txt` 必须一致 | `test.sh` 内的 `run_irb_test`（textual 段） |
| **L3 端到端 .s diff** | 直接编译 vs. dump-then-load 编译，`.s` 字节相同 | `test.sh` 内的 `run_irb_test`（s 段） |

### 6.2 在 `test.sh` 里跑

```bash
./test.sh nobuild        # 跳过 build, 直接跑测试 (前提 build/install/bin 已存在)
```

新增的 IRB 测试块独立计数；原有 11 个 sanity test 跑完后才进入。每个 .sy 用例输出形如：

```
[IRB][add1] OK
[IRB][b]    OK
...
[IRB] Summary: 13/14 passed, 1 failed
```

非零退出码（`exit 3`）表示有用例 fail。

### 6.3 当前测试结果（`testcase/simple/*.sy`）

| 类别 | 用例 |
| --- | --- |
| **PASS (textual + .s)** | add1 / add2_main / b / hex / if1-reduce1 / if1 / if2 / if3 / if4 / if5 / mul1 / not1 / sub1 |
| **FAIL** | array.sy（直接编译就 assert：`opt_main.cxx:680 not an even number of exprs for dimension/ofst in ARRAY` — 与 dump/load 无关，OPT 已存在 bug） |
| **SKIP** | add2.sy / and1.sy / tree.sy（FE 直接报错，与本工作无关） |

### 6.4 调试 tips

- 单测优先： `./build/common/symtab_test` 跑完 L1，定位字段级 mismatch；
- L1 全绿但 L2 红 → 多半是 strtab / 全局表的 Length / Add 顺序问题；
  打开 `--dump-textual` 看两份 `.txt` 有什么差异；
- L2 全绿但 L3 红 → BE/CG 在加载 IR 之上还会 `Save_string`，确认 `Strtab_replace`
  留了 headroom（line 142 附近 `new_used + STR_TABLE_BLOCK_SIZE`）；
- 加 stage 时：在 `BE_EXTERNAL_MAIN_NAME` 任何 `Opt_verify` 之后插 `Maybe_dump_ir("xxx", conf, File())`
  即可。reader 端不需要改。

---

## 7. 实现踩坑记录

### 7.1 重复 `Initialize` 留下幽灵 TY 项

**症状**：Reload 后再 dump 时 `STR_str` 报 `Invalid string idx = 284, allocated = 282`。

**根因**：`File()` 单例首次访问时已自动跑 `Initialize()`（`symtab.cxx:351`）：把 19 个 MTYPE
项种入 `_ty_tab` + 填充 strtab。`Load_ir_file` 又调一次 `fm->Initialize()` →
`_ty_tab.Length()` 从 20 涨到 39。`Strtab_replace` 紧接着把 strtab 截回 282 字节，
但多出来的 TY 项的 `name_idx` 已指向 282+ 区域。`Decode_global_TY` 读到 count=25 后发现
`Length()=39 > 25`，`while(... < n) Add()` 不触发，留下脏项 25..38；
`Print()` 一访问就触发 `Get_string` 边界 assert。

**修复**：删掉 `Load_ir_file` 中冗余的 `fm->Initialize()` 调用。

### 7.2 `Strtab_replace` 不留 headroom

**症状**：load 之后 BE/CG 第一次 `Save_string` 立刻 fatal `not enough space in the buffer`。

**根因**：原实现 `allocated_size_of_buffer = new_used`。`Save_string` 在
`used + needed < allocated` 失败时 realloc，但 `==` 时不 realloc，下一行的
`AssertThat(used + needed < allocated)` 直接挂。

**修复**：`Strtab_replace` 改成 `malloc(new_used + STR_TABLE_BLOCK_SIZE)` 并 memcpy，
释放 reader 临时 buffer。

### 7.3 `RELATED_SYMTAB_ACCESS::Get(i)` 把 i 当 IDX 解码

**症状**：load 时 SIGSEGV，stack 落在 `SCOPE::getStTab` 经 `Scoped_table()`。

**根因**：`Get(IDX)` 的 IDX 含义是 `(raw<<8) | level`，传 raw 计数 `i=0..n` 的 level
落进 0/1，再分支到 `tab[second]`，`second = (i>>8) & 0xffffff = 0` —— 看似没崩，但当
`i ≥ 256` 时 second 不再是 0 而是真正"第 second 个 ST"，导致越界或走错路径。

**修复**：global 编/解全用 `Begin()/End()` 迭代器（`std::vector<void*>::iterator`），
跳过 IDX 编码层。Local 表保持 `Get(i)`，因为那是直接的 `GROWING_TABLE`。

### 7.4 `TREE::Initialize()` 污染 IRNODE 表

**症状**：load 后 verify 报 `OPC_FUNC_ENTRY should not be in the body`。

**根因**：`Decode_function_subimage` 之前为每个 PU_INFO 调过 `pu->entry->Initialize()`，
该函数会预创建 `FUNC_ENTRY + BLOCK + BLOCK` 三个节点占 IRNODE 索引 0..2。然后
`Decode_irnodes` 走 `Create_node` 又 append 一遍 dump 出来的 0..2，让 0..2 变成两份。

**修复**：`Decode_function_subimage` 的两个 `Initialize()` 调用全部去掉，让 tree 从空开始
完全由 dump 数据重建。

---

## 8. 前向扩展点（Phase 2 预留，未实现）

| Hook                  | v1 预留                                                                 |
| --------------------- | ----------------------------------------------------------------------- |
| **SSA dump**          | `SUBSK_LOCAL_SSA = 16` 已留位；v2 加 `Encode_SSA(IR_WRITER&, SSAIR&)`，**不用 bump file_format_version**——sub-image 走 mini section table，旧 reader 见到未知 kind 用 `size` 跳过 |
| **CGIR dump**         | `SUBSK_LOCAL_CGIR = 17` 已留位                                          |
| **TCON / FILE_INFO**  | `SK_TCON_TAB = 16` / `SK_FILE_INFO_TAB = 17` 已留位                     |
| **DEBUG INFO**        | `SK_DEBUG_INFO = 64`                                                    |
| **Per-section ver**   | 每个 section 的 `version` 独立 bump；`Decode_X` 里 `switch (Section_version(kind))` 路由到老格式 reader |
| **mmap 友好**         | section 偏移绝对、表在尾部；mmap-based loader 可 O(1) 索引              |
| **跨架构**            | header `flags` bit0 标记 little-endian、`target_arch_id` 记目标架构；v2 可加 byte-swap reader 路径 |

---

## 9. Critical Files 索引

```
common/ir_io.h                  # 类 + section/sub-section kind + 高层入口
common/ir_io.cxx                # I/O 层实现
common/ir_serial.h              # 每元素 encoder/decoder 声明 + SECVER_*
common/ir_serial.cxx            # 编解码实现
common/symtab.h                 # FILE_SYMTAB; 新增 Strtab_buffer/Strtab_used
common/symtab.cxx               # Strtab_replace headroom 修复 + 三个 stub 接通
common/symtab_test.cxx          # L1 单测
common/options.h                # COMPILER_CONFIG.dump_ir_stages 等
driver/main.cxx                 # CLI args + Run_component 注入
driver/irprint_main.cxx         # 独立工具 `irprint`：load + Print
driver/CMakeLists.txt           # 注册 compiler / irprint 两个可执行
opt/opt_main.cxx                # BE 各 verify 后插 Maybe_dump_ir
test.sh                         # L2/L3 自动化测试
docs/ir-serialization.spec.md   # 本文件
```

---

## 10. 一图流总结

```
                   COMPILER_CONFIG.dump_ir_stages
                                |
   FE  --(femain)-->            v
   ┌─────────────┐  Maybe_dump_ir("fe",...)         ┌───────────┐
   │ FILE_MANAGER│ ───────────────────────────────► │ foo.fe.irb│
   └─────────────┘  (Dump_ir_file)                  └───────────┘
                                |
   OPT (BE_EXTERNAL_MAIN_NAME):  v
   verify(VHIGH) → lower(HIGH) → verify(HIGH) → Maybe_dump_ir("opt-high",...)
                 → lower(MID)  → verify(MID)  → Maybe_dump_ir("opt-mid",...)
                 → build_ssa → cprop → dce → destruct_ssa
                                            → Maybe_dump_ir("opt-after-ssa",...)
                 → lower(LOW) → verify(LOW) → Maybe_dump_ir("opt-low",...)
                 → lower(VLOW)→ verify(VLOW)→ Maybe_dump_ir("opt-vlow",...)
                 → lower(CGIR)→ verify(CGIR)→ Maybe_dump_ir("opt-cgir",...)

   CG: Maybe_dump_ir("pre-cg",...) → CG_full_process

                  --load-ir=PATH 短路:
                                FE 调用整个被替换为
                                Load_ir_file(File(), PATH)
```

---

## 11. 下一步建议

1. **加更多 .sy 用例进 round-trip** — 当前只对 simple 子集做 .s diff，可扩到 functional/。
2. **支持 dump 中间 OPT 阶段后 reload** — 现在 `--load-ir` 只在 FE 之后短路；要让
   `--load-ir-stage=opt-mid=path` 能从 OPT 中段切入，需要在 `BE_EXTERNAL_MAIN_NAME`
   开头加同样的短路逻辑 + skip 已经做过的 lower。
3. **SSAIR / CGIR 序列化** — Phase 2 预留 sub-section kind 已留好，加对应 encoder 即可。
4. **跨平台 little-endian 安全** — 当前依赖 host LE。`flags` bit0 已记，未来 reader 加
   byte-swap fallback。
