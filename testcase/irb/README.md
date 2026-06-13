# Binary IR Test Fixtures

本目录存放二进制 IR (`.irb`) + 对应的文本 dump (`.txt`) 测试样例，由
`testcase/simple/*.sy` 通过 `compiler --dump-ir-after=...` 产生，再用 `irprint` 把
内容拍平到 `.txt`。

## 文件命名

```
<base>.<stage>.irb        # 二进制 IR 文件
<base>.<stage>.txt        # irprint 输出 (FILE_MANAGER::Print 内容)
```

`<base>` 是源文件去掉 `.sy` 后的 basename；`<stage>` 是 dump 阶段（见 `docs/ir-serialization.spec.md`）：
`fe` / `opt-high` / `opt-mid` / `opt-after-ssa` / `opt-low` / `opt-vlow` / `opt-cgir` / `pre-cg`。

## 当前快照

| 用例 | 阶段 | 备注 |
| --- | --- | --- |
| add1 / add2_main / array / b / hex / if1 / if1-reduce1 / if2 / if3 / if4 / if5 / mul1 / not1 / sub1 | `fe` | FE 之后；最干净的中间表示 |
| add1 / if2 / mul1 | `opt-after-ssa` | SSA build / cprop / dce / destruct 之后 |
| add1 / if2 / mul1 | `pre-cg` | 全部 OPT 完成、CG 之前 |

跳过的源文件：

- `add2.sy` / `and1.sy` / `tree.sy` — FE 直接报错
- `array.sy` — FE 通过（所以 `array.fe.irb` 可生成）；但 OPT 后续会报
  `not an even number of exprs for dimension/ofst in ARRAY`（pre-existing bug），
  因此没有 `array.opt-*.irb` / `array.pre-cg.irb`

## 用途

这些文件作为 dump/load 工具链的回归基线：

1. **格式稳定性回归**：未来改 `ir_io` / `ir_serial` 时，重新生成一份 `.irb`，用
   `diff <new>.irb <fixtures>/<old>.irb` 看 wire format 是否变；或者保留 `.txt`
   作 oracle 比对（IRB 改了字段顺序但语义不变时，`.txt` 应保持一致）。

2. **`irprint` 自检**：回归测试可以直接 `diff <(irprint x.fe.irb) x.fe.txt`
   验证打印路径仍然正确。

3. **离线开发**：开发 OPT / CG pass 时不需要每次跑完整 FE，可以用
   `compiler --load-ir=testcase/irb/<base>.fe.irb -S <base>.sy -o /tmp/x.s` 直接
   从这里开始跑。

## 重新生成

新增 / 修改了 .sy 用例之后：

```bash
# 在仓库根目录
./build/driver/compiler --feonly --dump-ir-after=fe=testcase/irb/<base>.fe.irb \
    testcase/simple/<base>.sy
./build/driver/irprint testcase/irb/<base>.fe.irb -o testcase/irb/<base>.fe.txt
```

或者批量重建：

```bash
for f in testcase/simple/*.sy; do
  base=$(basename "$f" .sy)
  ./build/driver/compiler --feonly \
      --dump-ir-after=fe=testcase/irb/$base.fe.irb "$f" 2>/dev/null
  [ -s testcase/irb/$base.fe.irb ] && \
      ./build/driver/irprint testcase/irb/$base.fe.irb \
      -o testcase/irb/$base.fe.txt
done
```

## 大小参考

| 文件类型 | 单文件 | 备注 |
| --- | --- | --- |
| `.fe.irb` | 1.9–8.4 KB | 紧凑的二进制；`add1` 这类小例子 ~2.8KB |
| `.fe.txt` | 5–8 KB | 全部 symtab + IRNODE 树、可读 |

总目录大小当前约 260KB，所有文件都进 git。

## 看看里面长啥样

```bash
# header + section table（最快）
./build/driver/irprint --header-only testcase/irb/add1.fe.irb

# 全打印
./build/driver/irprint testcase/irb/add1.fe.irb | less

# 直接看预生成的 txt
less testcase/irb/add1.fe.txt
```

## 跨阶段 diff 示例

```bash
diff testcase/irb/add1.fe.txt testcase/irb/add1.opt-after-ssa.txt | less
diff testcase/irb/add1.opt-after-ssa.txt testcase/irb/add1.pre-cg.txt | less
```

第二个 diff 应当显示 destruct SSA 之后到 CG 之前 lowering 阶段对 IR 的修改。
