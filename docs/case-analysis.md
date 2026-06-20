# 测试用例分析

> **创建时间**: 2026-06-20
> **维护者**: claude
> **范围**: opencti/qemu ARMv7 回归(`szu.sanity.sched`)中失败用例的逐例分析。
> **配套**: bug 索引见 [`bug-tracking.md`](bug-tracking.md);测试环境见
>          [`how-to-run-opencti-for-central.md`](how-to-run-opencti-for-central.md)。

---

## 2026-06-18 回归:`szu.gcc.core` 99/107 → 95/107

新构建的 compiler(2026-06-18)相对旧二进制(2026-06-09)在 `szu.gcc.core` 多挂 4 个用例。
经 `diff` 旧日志(`log.sun`,8 fail)与新日志(`log.thu`,12 fail)定位,新增的恰好是下面 4 个,
且 **全部命中同一根因 [BUG-001](bug-tracking.md#bug-001)**(`cg/lra.cxx:269` LRA live_in 断言)。

### 4 个回归用例的共同点

注意:**与 `array_param` / 多维数组无关**。这 4 个用例真正的共同特征是
**「声明非 void 返回 + 带参数 + 函数体为空(没有 return)」**——参数和空体的组合才是触发点。

| 用例 | 关键代码 | 触发的空体函数 |
|------|----------|----------------|
| `array_param01` | `int foo(int a[][10]) {}` | `foo`(数组参数,空体) |
| `array_param02` | `int foo(int n) {}` | `foo`(标量参数,空体) |
| `array_param03` | `int foo(int m, int n) {}` | `foo`(双参数,空体) |
| `call02` | `int far(int m) {}` `int foo(){ far(30); }` | `far`(标量参数,空体) |

所有用例报错完全一致:
```
### In file /home/cti/central-src/cg/lra.cxx:269
occ: fatal error: ### Assertion Failure in Phase CODE_GEN:
Stmt-level live_in doesn't match BB-level, bb_idx = <CGBB指针被%d截断的值>
```

---

### 用例源码

**array_param01.sy**
```c
int foo(int a[][10]) {

}
int main() {
  int a[3][10];
  foo(a);
}
```

**array_param02.sy**
```c
int foo(int n) {

}
int main() {
  int a[3][10];
  int i;
  foo(a[1][5]);
}
```

**array_param03.sy**
```c
int foo(int m, int n) {

}
int main() {
  int a[3][10];
  int i = 2;
  foo(a[1][5], a[i][6]);
}
```

**call02.sy**
```c
int far(int m) {
}

int foo() {
  far(30);
}
```

---

### 触发边界(对照实验)

用最小变体逐项隔离变量,确认触发条件:

| # | 代码 | 结果 |
|---|------|------|
| A | `int f(int x){} int main(){f(0);}` | ❌ 触发 `lra.cxx:269` |
| B | `int f(){} int main(){f();}` | ✅ OK(无参) |
| C | `void f(){} int main(){f();}` | ✅ OK(void) |
| D | `int f(int x){} int main(){return 0;}` | ❌ 触发(`f` 未被调用也触发) |
| E | `int f(int x){return x;} int main(){f(0);}` | ✅ OK(有 return) |

**判定**:
- A vs B → **参数** 是必要条件;
- A vs C → **非 void 返回** 是必要条件;
- A vs E → **空体(缺 return)** 是必要条件;
- D → 与调用点无关,问题在被编译的空体函数自身。

**最小复现**:`int f(int x) {}`

### 归因
见 [BUG-001 根因推断](bug-tracking.md#根因推断待-central-团队确认):非 void 无 return 的带参函数,
参数 TN 进入了 stmt-level live_in,但 Phase 1 的 BB-level live_in 未包含 → `std::includes`
子集检查失败。建议从「空体函数的 return/epilogue 处理」与「`TN_SET` 的有序性」两处入手。

---

## 2026-06-21 发现:`return <函数调用>` 返回值丢失([BUG-003](bug-tracking.md#bug-003))

开发脚本 `central-run-case.sh` 快测时撞出。**不在 szu.gcc.core 用例集里**,是用最小手写
case 暴露的运行期错误(编译/汇编/链接全部成功,运行结果错)。

### 用例与对照
```c
int add(int a, int b) { return a + b; }
int main() { return add(20, 22); }
```
| 编译器 | 退出码 |
|--------|--------|
| central | **196**(错) |
| arm-linux-gnueabihf-gcc(对照) | 42(对) |

### 关键汇编(central 的 main)
```asm
	bl	add               # 返回值 r0 = 42
	mov	r8 , r0           # 存进 r8
	mov	r0 , r10          # ✗ 用垃圾 r10 覆盖 r0
	b	.L2817_endfunc
	str	r8 , [sp, #4 ]    # ✗ 倒腾 r8->r10 的 spill/reload 在 b 之后 = 死代码
	ldr	r10 , [sp, #4 ]
.L2817_endfunc:
	...
```
spill/reload 指令排到了无条件跳转之后,执行不到,`r0` 最终是未定义的 `r10`。详见
[BUG-003](bug-tracking.md#bug-003)。

---

## 旧有失败用例(2026-06-09 与 2026-06-18 两版均失败,非回归)

仅登记,详细分析待补:

| 用例 | 初判归类 |
|------|----------|
| array_def06 / array_def07 / array_def08 | CG 断言崩溃 |
| var04 | IR_GEN 断言 |
| add2 / and1 / expr02 | parser 语法不支持 |
| comment04 | parser(注释处理) |
