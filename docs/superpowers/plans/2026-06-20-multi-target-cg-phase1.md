# Multi-Target CG Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish an explicit target/ABI boundary while preserving ARMv7 assembly output as the default.

**Architecture:** A target registry under `cg/` resolves canonical triples and aliases into immutable target metadata. `COMPILER_CONFIG` carries the resolved metadata into CG; `CG_COMPOSITE`, `DATA_LAYOUT`, and `CG_EMITTER` consume it instead of ARM constants. Phase 1 implements the ARMv7 backend and rejects code generation for the other recognized-but-not-yet-implemented targets without claiming assembly support.

**Tech Stack:** C++14, CMake/CTest, existing CGIR and driver argument parser.

---

### Task 1: Target registry and aliases

**Files:**
- Create: `cg/target_info.h`
- Create: `cg/target_info.cxx`
- Create: `cg/target_info_test.cxx`
- Modify: `cg/CMakeLists.txt`

- [ ] Write resolver tests for the default, all canonical triples, all aliases, and an unknown name.
- [ ] Run `ctest` and verify the new test fails because the target registry does not exist.
- [ ] Implement target enums, ABI/layout metadata, canonicalization, and lookup.
- [ ] Run the target registry test and verify it passes.

### Task 2: CLI and compiler configuration

**Files:**
- Modify: `common/options.h`
- Modify: `driver/main.cxx`
- Modify: `driver/CMakeLists.txt`
- Modify: `cg/target_info_test.cxx`

- [ ] Add tests that copy resolved target metadata into `COMPILER_CONFIG` and reject conflicting/unknown target names.
- [ ] Verify the tests fail for the missing configuration API.
- [ ] Add `--target`, connect `--march`/`--arch`, default to ARMv7, and populate triple, arch, OS, ABI, pointer size, symbol prefix, section syntax, stack alignment, register description, and assembler hint.
- [ ] Make `-c` fail explicitly for non-default targets because integrated target assemblers are outside the V1 contract.
- [ ] Re-run unit tests and CLI smoke tests.

### Task 3: ABI-driven data layout

**Files:**
- Modify: `cg/data_layout.h`
- Modify: `cg/data_layout.cxx`
- Modify: `cg/cg_builder.cxx`
- Create: `cg/data_layout_target_test.cxx`
- Modify: `cg/CMakeLists.txt`

- [ ] Add focused tests for register-formal count, pointer-formal size, stack alignment, and prologue save area.
- [ ] Verify failures expose the current constants (`4`, `8`, and `36`).
- [ ] Inject `TARGET_ABI` into `DATA_LAYOUT` and replace those constants while retaining ARMv7 values.
- [ ] Run layout and registry tests.

### Task 4: ARMv7 backend boundary

**Files:**
- Create: `cg/target_backend.h`
- Create: `cg/armv7_linux_target.h`
- Create: `cg/armv7_linux_target.cxx`
- Modify: `cg/cg_composite.h`
- Modify: `cg/cgir.h`
- Modify: `cg/cg_main.cxx`
- Modify: `cg/cg_pass.cxx`
- Modify: `cg/cgir.cxx`
- Modify: `cg/CMakeLists.txt`

- [ ] Capture assembly for `testcase/simple/simple.c` with the current compiler as a baseline.
- [ ] Add backend unit tests for section/function syntax, prologue/epilogue, operands, and symbol spelling.
- [ ] Verify tests fail before the backend exists.
- [ ] Move ARMv7-specific assembly spelling behind `TARGET_BACKEND`; leave CFG traversal generic.
- [ ] Pass the selected backend through `CG_COMPOSITE`, builder, layout, and emitter.
- [ ] Rebuild and compare default-target assembly with the baseline.

### Task 5: Documentation and verification

**Files:**
- Modify: `docs/retargeting.md`
- Modify: `cg/README.md`

- [ ] Document supported names, the Phase 1 implementation status, default target, `-S --target=...`, explicit non-ARM emitter rejection, and external assembler commands.
- [ ] Run all CTest tests.
- [ ] Run `test.sh` or the repository's equivalent ARMv7 regression command when its external toolchain is available.
- [ ] Verify no unrelated user changes were modified.
