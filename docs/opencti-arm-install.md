spec-arm32-passing.md 已写入 central-opencti/doc/。文档涵盖:

1. 总体流水线 + 为什么必须用外部交叉工具链(central 内部 asm stage 禁用且有 bug)
2. 环境准备(host 下 deb→容器 dpkg 绕过 Rosetta apt)
3. CC wrapper(central-gcc:central -S → 交叉汇编真 .o,feonly 优先级)
4. 两个交叉链接脚本(ssh_link / ssh_link_so,-marm 避 SIGILL)
5. hard-float libsysy 重编(sylib.c,计时走 stderr)
6. SIMULATOR + timeout(必须,防死循环堵塞)
7. opt 配置改动(只改 szu 专用,不碰 common.run.opt 保护 open64)
8. 复现步骤 / 当前结果 / central bug 清单(定位到 case) / 文件清单

顺便把这次成果的指针更新进 memory(跨会话可复用):

  Wrote 1 memory

文档和 memory 都更新好了。

总结

central-opencti/doc/spec-arm32-passing.md 已写入——这是一份完整的 ARMv7-EABI 回归通过方案 spec,任何人(或下次会话)照着就能复现整套流水线:容器环境 → CC wrapper → 交叉链接 → hard-float libsysy → qemu 运行 → DTM 配置 → 复现命令 → 当前结果 → central bug 清单。

memory 也加了指针(指向这份 spec)和关键事实,跨会话可直接复用。

这次从"环境都没有"到"完整 DTM 回归跑通"的全过程:

1. ✅ 容器内装好 qemu + 交叉工具链(绕过 Rosetta apt 龟速)
2. ✅ central 编译器接入 CTI(wrapper 补齐 central 缺失的 .s→.o)
3. ✅ 交叉链接(ssh_link/.so,-marm 避 SIGILL)
4. ✅ 重编 hard-float libsysy(LinkErr 37→0)
5. ✅ qemu 运行 + timeout(防死循环堵塞)
6. ✅ 完整 szu.sanity 回归跑通,失败全部收敛到 central 自身的编译期/运行期 bug,并定位到 case 级别

如果之后要继续,opt.run 已配好 libsysy 待重跑,以及 application 的 Makefile.CTI 可补——随时叫我。
