# Arm64 CG Backend Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Generate valid AArch64 Linux and Apple arm64 assembly while keeping Very Low IR to CGIR conversion shared and preserving ARMv7 output.

**Architecture:** `CGIR_BUILDER` emits target-neutral operations and queries ABI location rules. `TARGET_BACKEND` lowers each logical CGOP to one or more target instructions. A shared Arm64 backend handles A64 instructions while small platform branches handle ELF versus Mach-O symbols and relocations.

**Tech Stack:** C++14, CMake/CTest, Clang cross-target assembler, existing CGIR.

---

### Task 1: Make CGOP emission target-owned

**Files:**
- Modify: `cg/target_backend.h`
- Modify: `cg/armv7_linux_target.{h,cxx}`
- Modify: `cg/cg_main.cxx`
- Modify: `cg/cgir.{h,cxx}`
- Modify: `cg/armv7_linux_target_test.cxx`

- [ ] Add a failing unit test requiring the ARMv7 backend to emit a complete logical operation.
- [ ] Add an emission context and `Emit_op` backend entry point.
- [ ] Move current operand/instruction printing into `Armv7LinuxTarget` without changing output.
- [ ] Compare generated ARMv7 assembly with the Phase 1 baseline.

### Task 2: Add target-neutral CGOP intents

**Files:**
- Modify: `cg/cg_opc.h`
- Modify: `cg/cg_builder.cxx`
- Modify: `cg/cgir.cxx`
- Modify: `cg/target_backend.h`
- Modify: `cg/armv7_linux_target.cxx`

- [ ] Add failing tests for logical load-immediate, load-address, return, and stack-adjust emission.
- [ ] Add `CGOPC_LIMM`, `CGOPC_LADDR`, `CGOPC_RET`, and `CGOPC_ADJSP`.
- [ ] Change constant, address, return, and call-stack builder paths to emit these intents.
- [ ] Lower the new intents back to the exact existing ARMv7 sequences.
- [ ] Re-run ARMv7 byte comparison.

### Task 3: Parameterize call/formal ABI placement

**Files:**
- Modify: `cg/target_info.{h,cxx}`
- Modify: `cg/cg_builder.cxx`
- Modify: `cg/tn.cxx`
- Modify: `cg/target_info_test.cxx`

- [ ] Add failing tests for Arm64 argument count, slot size, pointer TN width, and return register.
- [ ] Replace four-register and four-byte call/formal constants with `TARGET_ABI` values.
- [ ] Ensure preallocated TNs use ABI widths and argument register indices.
- [ ] Verify ARMv7 and ABI tests.

### Task 4: Implement Arm64 target emission

**Files:**
- Create: `cg/arm64_target.{h,cxx}`
- Create: `cg/arm64_target_test.cxx`
- Modify: `cg/armv7_linux_target.cxx`
- Modify: `cg/CMakeLists.txt`

- [ ] Add failing tests for A64 file/function syntax, prologue/epilogue, register widths, logical operations, memory operations, calls, returns, globals, and branches.
- [ ] Implement shared Arm64 lowering.
- [ ] Add ELF and Mach-O symbol, section, and relocation variants.
- [ ] Enable both Arm64 target factories.
- [ ] Run backend tests.

### Task 5: Assembly integration and documentation

**Files:**
- Create: `driver/arm64_cli_test.cmake`
- Modify: `driver/CMakeLists.txt`
- Modify: `cg/README.md`
- Modify: `docs/retargeting.md`

- [ ] Compile constant, local, arithmetic, branch, loop, call, global, and array inputs for both Arm64 triples.
- [ ] Assemble Apple output with `clang -target arm64-apple-macos -c`.
- [ ] Assemble Linux output when `aarch64-linux-gnu-gcc` is available.
- [ ] Run all CTest and existing default-target tests.
- [ ] Document Phase 2 status and remaining x86 work.
