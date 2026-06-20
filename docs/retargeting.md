# Multi-Target CG Refactor Plan

  ## Summary

  Refactor CG so target choice is explicit and centralized, while preserving the current ARMv7 path as the default/status quo. V1 guarantees target-specific assembly output only, with
  external assembler/link commands documented for validation. Supported v1 targets: armv7-linux-gnueabihf, aarch64-linux-gnu, arm64-apple-darwin, i386-linux-gnu, x86_64-linux-gnu, and
  x86_64-apple-darwin.

  ## Key Interface Changes

  - Add TARGET_INFO / TARGET_ABI concepts under cg/, selected from CLI aliases:
      - default: armv7-linux-gnueabihf
      - aliases: armv7, arm32, aarch64-linux, arm64-linux, arm64-macos, x86-linux, x64-linux, x64-macos

  - Wire existing --march/--arch to target selection; optionally add --target=<triple> as the canonical spelling.
  - Extend COMPILER_CONFIG with resolved target triple, arch, OS, pointer size, ABI, symbol prefix, section syntax, stack alignment, register set, and external assembler hint.
  - Keep CGIR mostly target-neutral: CGOPC_ADD, CGOPC_LDR, CGOPC_STR, CGOPC_CALL, branches, etc. remain logical operations; target emitters translate them to concrete assembly.

  ## Implementation Changes

  - Introduce a small target backend interface used by CG_COMPOSITE, CGIR_BUILDER, DATA_LAYOUT, TN setup, and CG_EMITTER:
      - register naming and dedicated registers: SP, FP/BP, LR/RA, return value, integer argument registers, caller/callee-save sets
      - ABI layout rules: pointer size, stack alignment, first register arguments, stack argument area, return register
      - instruction lowering helpers: load/store, load immediate, load symbol address, move, arithmetic, compare/branch, call, return
      - assembly syntax helpers: global label, function label/type, data directives, symbol prefix, comment prefix

  - Preserve ARMv7 behavior first by moving today’s hardcoded ARMv7 logic into Armv7LinuxTarget; do not change its emitted assembly except where necessary to remove hardcoded globals.
  - Add three new emitter/lowering families:
      - X86LinuxTarget: i386 SysV, 32-bit pointers, AT&T or GNU-compatible syntax, stack-passed args with simple frame-based lowering
      - X64Target: SysV for Linux and Apple x86_64 for macOS, including Apple _symbol prefix and Mach-O directives
      - Arm64Target: AArch64 Linux and Apple arm64, including platform-specific symbol prefix/directives

  - Keep LRA, interference graph, and full register allocation out of scope:
      - reuse existing simple register assignment where possible
      - backend supplies allocatable physical register lists
      - builder uses dedicated/preallocated TNs for ABI-sensitive values like args, return, SP/FP/LR

  - Refactor DATA_LAYOUT to take ABI parameters instead of ARM constants:
      - stack alignment from target
      - register-formal count from target
      - pointer size from target
      - prologue save area described by target, not fixed 28 + 8

  - Refactor current ARM-specific code in CGIR_BUILDER::Handle_*:
      - keep IR walking and CGOP creation shared
      - delegate target-specific pieces: formal storing, call arg placement, return value register, address materialization, immediate materialization, branch condition mapping

  - Refactor CG_EMITTER::Emit_*:
      - split generic CFG traversal from target assembly printing
      - target owns prologue/epilogue, operand spelling, instruction spelling, data section emission, function metadata, and label spelling

  - Update docs and examples:
      - document supported triples, default target, and sample compiler -S --target=...
      - document external validation commands for each target; no integrated binary/link guarantee in v1

  ## Test Plan

  - Add assembly golden tests for a small shared set of programs: constant return, local variable, add/sub/mul/div, if/else, loop, function call, globals, arrays.
  - For ARMv7, preserve existing OpenCTI/qemu path and compare current known-good output behavior before/after refactor.
  - For assembly-only validation:
      - ARMv7 Linux: arm-linux-gnueabihf-gcc -marm -march=armv7-a ... -c
      - ARM64 Linux: aarch64-linux-gnu-gcc -c
      - ARM64 macOS: clang -target arm64-apple-macos -c
      - x86 Linux: gcc -m32 -c or container toolchain
      - x64 Linux: gcc -m64 -c
      - x64 macOS: clang -target x86_64-apple-macos -c

  - Run existing test.sh with default target to ensure no ARMv7 regression.
  - Add negative tests for unknown target names and unsupported -c integrated assembler behavior.

  ## Assumptions

  - V1 output contract is assembly-first: -S must work for all targets; -c may remain target-wrapper/toolchain-dependent.
  - ARMv7 remains first-class and default because current benchmarks depend on it.
  - x86 means 32-bit i386 Linux only; no runnable 32-bit macOS target in v1.
  - The implementation should favor clear learning-oriented target abstractions over a full production backend framework.

## Implementation Status (2026-06-20)

Phase 1 is implemented:

- `TARGET_INFO` and `TARGET_ABI` resolve all six canonical triples and the
  documented aliases. The default remains `armv7-linux-gnueabihf`.
- `--target`, `--arch`, and `--march` populate resolved target metadata in
  `COMPILER_CONFIG`. Conflicting or unknown names are rejected.
- `DATA_LAYOUT` receives ABI pointer size, stack alignment, register-formal
  count, and prologue save area from the selected target.
- `Armv7LinuxTarget` owns current section, symbol, register, immediate,
  prologue, and epilogue assembly syntax. Default output is byte-for-byte
  unchanged in the phase-1 baseline test.
- The five non-ARMv7 targets are metadata-complete but their emitters are not
  implemented yet. `-S` fails explicitly instead of emitting incorrect ARM
  assembly. Non-ARMv7 `-c` reports the external assembler hint.

Canonical selection:

```sh
compiler -S --target=armv7-linux-gnueabihf input.c -o input.s
```

External assembly validation commands for the completed V1 emitters are:

```sh
arm-linux-gnueabihf-gcc -marm -march=armv7-a -c input.s
aarch64-linux-gnu-gcc -c input.s
clang -target arm64-apple-macos -c input.s
gcc -m32 -c input.s
gcc -m64 -c input.s
clang -target x86_64-apple-macos -c input.s
```

## Phase 2 Status (2026-06-21)

- `aarch64-linux-gnu` and `arm64-apple-darwin` now have a shared Arm64 target
  emitter with ELF/Mach-O section, symbol, relocation-pointer, and function
  syntax variants.
- Full CGOP emission is target-owned; generic emission only traverses CFG/BBs.
- `CGIR_BUILDER` still owns Very Low IR to CGIR conversion. It now emits
  logical load-immediate, load-address, return, and stack-adjust CGOPs instead
  of encoding these intents as ARMv7 `MOVT`, `BX lr`, or SP arithmetic.
- Arm64 ABI handling uses eight integer argument registers, 64-bit pointers,
  eight-byte argument slots, and 16-byte stack alignment. Calls stage argument
  values in aligned outgoing stack storage before loading `w0-w7`/`x0-x7`.
- Apple and Linux Arm64 smoke programs are assembled in CTest with Clang target
  assemblers. Integrated `-c` remains intentionally unavailable.
- Array compilation is not claimed in this phase because the existing array
  lowering fails before target emission; it remains a separate shared-CG/OPT
  issue rather than an Arm64 emitter issue.

## Phase 3 Status (2026-06-21)

- Arm64 large frame and large stack adjustments materialize out-of-range
  immediates in `x16`; logical `LIMM`, `LADDR`, `RET`, `ADJSP`, memory,
  arithmetic, compare, branch, and call paths are target-assembler tested.
- `i386-linux-gnu`, `x86_64-linux-gnu`, and `x86_64-apple-darwin` use a shared
  AT&T-syntax x86 emitter with platform-specific frames, register widths,
  symbols, sections, calls, and pointer directives.
- i386 uses stack-passed arguments and accounts for saved BP plus return
  address. x86_64 uses six SysV register arguments and aligned stack staging
  for additional arguments.
- Return-value TNs are ABI-aware: AAPCS reuses argument register zero, while
  x86 uses the dedicated accumulator independently of its first argument.
- CTest assembles non-array constant/local/arithmetic/division/branch/call/
  globals/eight-argument programs for all five non-default V1 targets.
- Array lowering remains explicitly outside these target-emitter phases.
