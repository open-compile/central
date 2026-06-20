# Arm64 Audit and x86 Backends Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish and assembler-verify Arm64 logical-operation lowering, then add i386 Linux and x86_64 Linux/macOS assembly backends without changing array lowering.

**Architecture:** Very Low IR traversal stays in `CGIR_BUILDER`, which emits logical CGOPs and uses `TARGET_ABI` for locations. Arm64 and x86 target emitters own instruction expansion; the x86 family shares AT&T operand formatting while separate ABI/platform policy handles 32/64-bit registers, symbols, calls, frames, and directives.

**Tech Stack:** C++14, CMake/CTest, Clang target assemblers, existing CGIR.

---

### Task 1: Audit Arm64 logical operations

**Files:**
- Modify: `cg/arm64_target.{h,cxx}`
- Modify: `cg/arm64_target_test.cxx`
- Modify: `driver/arm64_cli_test.cmake`

- [ ] Add failing tests for 32-bit positive/negative `LIMM`, `LADDR`, signed `ADJSP`, `RET`, large frames, memory offsets, calls, arithmetic, compare, and branches.
- [ ] Add reusable immediate/frame helpers that emit legal A64 sequences beyond single-instruction ranges.
- [ ] Assemble Linux and Apple outputs for constants, globals, locals, arithmetic, branches, loops, calls, and eight arguments.
- [ ] Preserve ARMv7 byte output.

### Task 2: Introduce shared x86 target emitter

**Files:**
- Create: `cg/x86_target.{h,cxx}`
- Create: `cg/x86_target_test.cxx`
- Modify: `cg/target_backend.cxx`
- Modify: `cg/CMakeLists.txt`

- [ ] Add failing unit tests for i386, x86_64 Linux, and x86_64 Apple sections, symbols, registers, frames, and pointer directives.
- [ ] Implement shared AT&T formatting and target factory selection.
- [ ] Lower move/immediate/address/load/store/arithmetic/compare/branch/call/return/stack-adjust CGOPs.
- [ ] Expand signed division through accumulator/high-register conventions.

### Task 3: Complete x86 ABI integration

**Files:**
- Modify: `cg/target_info.{h,cxx}`
- Modify: `cg/cg_builder.cxx`
- Modify: `cg/data_layout.cxx`
- Modify: `cg/target_info_test.cxx`

- [ ] Verify i386 uses stack arguments and 32-bit pointers.
- [ ] Verify x86_64 uses six integer argument registers, 64-bit pointers, and 16-byte call alignment.
- [ ] Ensure Darwin external symbols receive `_` while compiler-local labels do not.
- [ ] Keep integrated `-c` disabled for all new targets.

### Task 4: Add x86 assembler integration tests

**Files:**
- Create: `driver/x86_cli_test.cmake`
- Modify: `driver/CMakeLists.txt`
- Modify: `cg/README.md`
- Modify: `docs/retargeting.md`

- [ ] Generate shared non-array programs for all three x86 targets.
- [ ] Assemble x86_64 Linux and Apple with Clang.
- [ ] Assemble i386 Linux with `clang -target i386-linux-gnu`.
- [ ] Run all CTest, `test.sh`, ARMv7 byte comparison, and `git diff --check`.
