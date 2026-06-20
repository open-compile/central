# Central 编译器 Bug 跟踪

> **创建时间**: 2026-06-20
> **维护者**: claude
> **用途**: 跟踪 central 编译器已知缺陷,优先记录通过 opencti/qemu ARMv7 回归
>          (`szu.sanity.sched`)发现的问题。详细的逐用例分析见
>          [`case-analysis.md`](case-analysis.md)。

**状态图例**: 🔴 未修复 / 🟡 已定位待修 / 🟢 已修复 / ⚪ 待确认

---

## Bug 清单

| ID | 标题 | 位置 | 严重度 | 状态 | 关联用例 |
|----|------|------|--------|------|----------|
| BUG-001 | LRA: 空体非void带参函数 stmt/BB live_in 不一致断言 | `cg/lra.cxx:269` | 高(**回归**) | 🟡 | array_param01/02/03, call02 |
| BUG-002 | LRA: 断言诊断信息把 `CGBB*` 用 `%d` 打印 | `cg/lra.cxx:269` | 低 | 🟡 | (同上,诊断噪音) |
| BUG-003 | 返回函数调用结果时返回值丢失:spill/reload 排到无条件跳转之后(死代码) | `cg/lra.cxx`(spill 重写) | 高 | 🟡 | `return add(...)` 类 |

---

## BUG-001 — LRA stmt/BB live_in 一致性断言失败(回归)

**状态**: 🟡 已定位待修　**严重度**: 高　**类型**: 代码生成回归

### 现象
```
### In file /home/cti/central-src/cg/lra.cxx:269
occ: fatal error: ### Assertion Failure in Phase CODE_GEN:
Stmt-level live_in doesn't match BB-level, bb_idx = 1433441088
```
occ 异常退出,只产生 3 行空壳 `.s`,在 `szu.gcc.core` 阶段记为 CompileErr。

### 回归确认
2026-06-18 从当前源码重新构建 compiler 后,`szu.gcc.core` 从 **99/107 → 95/107**,
新增 4 个失败用例:`array_param01/02/03`、`call02`。这 4 个在旧 binary(2026-06-09 构建)
上可以通过 —— 即当前 `cg/` 源码相对旧二进制对应的源码引入了此回归。

### 触发条件(对照实验锁定)
| 变体 | 触发 |
|------|------|
| 非 void 返回 + **有参** + 空体(无 return) | ❌ 触发 |
| 非 void 返回 + **无参** + 空体 | ✅ 不触发 |
| **void** 返回 + 空体 | ✅ 不触发 |
| 非 void + 有参 + 空体 + **不被调用** | ❌ 触发 |
| 非 void + 有参 + **有 return** | ✅ 不触发 |

结论:**「非 void 返回类型 + 带参数 + 函数体为空(无 return 语句)」** 三者同时满足即触发,
且与该函数是否被调用无关 —— 出错的是被编译的那个空体函数自身的 liveness 计算。

### 最小复现
```c
int f(int x) {}      // 单独这一个函数即可触发,main 可有可无
```

### 代码定位
`cg/lra.cxx` 的 def-use builder(Phase 2,逐语句反向遍历算 stmt-level live_in)末尾的
一致性检查:
```cpp
// 265  Sanity check: live_in of first stmt should ⊆ BB live_in
266  if (stmt_count > 0) {
267    BOOL is_subset = std::includes(_live_ins[bb].begin(), _live_ins[bb].end(),
268                                   _stmt_live_ins[bb][0].begin(), _stmt_live_ins[bb][0].end());
269    AssertThat(is_subset, ("Stmt-level live_in doesn't match BB-level, bb_idx = %d", cur_bb));
270  }
```
即:第一条语句的 stmt-level live_in 不再是 BB-level live_in(Phase 1 `_live_ins[bb]`)的子集。

### 根因推断(待 central 团队确认)
非 void 但无 return 的带参函数,参数 TN 进入了某 BB 的 stmt-level live_in,而 Phase 1 算的
BB-level live_in 没有包含它,导致 `std::includes` 失败。可疑方向:
- 空体函数缺少正常的 return/epilogue,参数 TN 的 live range 在两个 Phase 间建立方式不一致;
- 若 `TN_SET` 底层非有序容器(`std::includes` 要求两个区间均已按相同序排序),会直接导致
  子集判断错误 —— 需核对 `_live_ins` / `_stmt_live_ins` 的容器类型与排序保证。

---

## BUG-002 — 断言诊断把 `CGBB*` 当 `%d` 打印

**状态**: 🟡　**严重度**: 低

`cg/lra.cxx:269` 的 `AssertThat(..., ("... bb_idx = %d", cur_bb))` 中 `cur_bb` 是 `CGBB*`
指针,用 `%d` 格式化 → 打印出 `1433441088` 这类「指针地址被截断成 int」的值,**并非未初始化
内存**。修复:打印真正的 BB 序号(循环变量 `bb`),或用 `%p` 打印指针。属诊断质量问题,不影响
正确性,但会误导排查(曾被误判为脏内存读取)。

---

## BUG-003 — 返回函数调用结果时返回值丢失(spill/reload 排到无条件跳转之后)

**状态**: 🟡 已定位待修　**严重度**: 高　**类型**: 代码生成(寄存器分配 / spill 重写)

### 现象
```c
int add(int a, int b) { return a + b; }
int main() { return add(20, 22); }     // 期望退出码 42
```
经 central 编译 → 交叉汇编/链接 → qemu-arm 运行,**退出码 196**。
纯 `arm-linux-gnueabihf-gcc` 编译同一源码退出码为 **42**,确认是 central codegen 问题。

### central 生成的 `main`(关键片段)
```asm
	bl	add               # add(20,22) 返回值在 r0 = 42
	mov	r8 , r0           # 把返回值存进 r8
	mov	r0 , r10          # ✗ 用 r10(垃圾)覆盖 r0,然后就返回了
	b	.L2817_endfunc
	str	r8 , [sp, #4 ]    # ✗ 这两条「把 r8 倒给 r10」的指令排在无条件 b 之后
	ldr	r10 , [sp, #4 ]   #    → 死代码,永不执行
.L2817_endfunc:
	sub	sp, fp, #32
	pop	{r4-r10}
	pop	{fp, pc}
```
本应让返回值留在 `r0`(=42)。实际寄存器分配把返回值分到了 `r8`,再试图通过栈槽
(`str r8 ; ldr r10`)把它倒进 `r10` 后 `mov r0, r10` —— 但 **spill/reload 这两条指令被
排到了无条件跳转 `b` 之后**,根本执行不到,于是 `r0` 拿到的是未定义的 `r10`,退出码变成垃圾值。

### 根因推断(待 central 团队确认)
寄存器分配的 spill 重写阶段在「插入 reload 指令」与「基本块终结的无条件跳转 / 函数返回序列」
之间的**顺序处理有误**:reload(`ldr r10`)及配套 `str` 被追加到了块尾跳转之后,而不是插入到
使用点(`mov r0, r10`)之前。属 `cg/lra.cxx` 寄存器分配/spill 逻辑,与 [BUG-001](#bug-001)
同一模块,建议一并排查。

### 最小复现
```c
int add(int a, int b) { return a + b; }
int main() { return add(20, 22); }
```
快测:`scripts/central-run-case.sh <该文件>`(退出码应为 42,实际 196)。

### 备注
- 是否相对 2026-06-09 旧 binary 为回归:**待确认**(旧二进制已被覆盖,需 checkout 旧源码重建对比)。
- 影响面可能较大:凡「`return <函数调用>`」或返回值需经 spill 的场景都可能中招。

---

## 附:`szu.gcc.core` 12 个失败用例归类(2026-06-18,新 compiler)

| 用例 | 归类 | 备注 |
|------|------|------|
| array_param01 / array_param02 / array_param03 | **BUG-001(回归)** | 空体非void带参函数 |
| call02 | **BUG-001(回归)** | 同上(`int far(int m){}`) |
| array_def06 / array_def07 / array_def08 | 旧有 CG 断言 | 见 [case-analysis.md](case-analysis.md) / 历史 cg.diagnose |
| var04 | 旧有 IR_GEN 断言 | |
| add2 / and1 / expr02 / comment04 | 旧有 parser 不支持 | |

> 旧有 8 项在 2026-06-09 与 2026-06-18 两版均失败,非本次回归;此处仅登记,详分析待补。
