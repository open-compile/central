# Arm64 CG Backend Design

## Scope

Phase 2 adds assembly output for `aarch64-linux-gnu` and
`arm64-apple-darwin`. `CGIR_BUILDER` remains the owner of Very Low IR to CGIR
conversion. Target objects provide ABI policy and final instruction emission;
they do not walk Very Low IR.

The existing ARMv7 output remains byte-for-byte stable. i386 and x86_64 remain
recognized but unavailable until later phases.

## Responsibility Boundary

`CGIR_BUILDER` owns CFG construction, expression traversal, statement ordering,
def/use relationships, and creation of logical CGOPs. It asks `TARGET_ABI` for
pointer width, argument locations, return register, stack alignment, and call
stack requirements.

Target-neutral CGOPs describe intent rather than an ARM encoding. Phase 2 adds
logical load-immediate, load-address, return, and stack-adjust operations. The
builder stops constructing `MOVT`, `BX lr`, and ARM push/pop sequences for
these intents.

`TARGET_BACKEND` owns CGOP-to-assembly lowering. One logical CGOP may produce
multiple assembly instructions. It also owns register spelling, symbol
prefixes, function metadata, sections, prologue/epilogue, relocations, and
data pointer directives.

## ABI Rules

Both Arm64 targets use eight integer argument registers (`x0` through `x7`),
`x0` for scalar integer returns, `sp` as stack pointer, `x29` as frame pointer,
`x30` as link register, 64-bit pointers, and 16-byte stack alignment.

Four-byte integer values use `wN`; addresses and dedicated frame registers use
`xN`/`sp`. Stack argument slots are eight bytes. The builder uses preallocated
TNs for register arguments and stack locations for remaining arguments.

## Platform Differences

Linux uses ELF sections and unprefixed global symbols. Address materialization
uses ELF AArch64 relocations.

Apple uses Mach-O sections/directives and prefixes external/global symbols
with `_`. Address materialization uses Apple page/pageoff relocations. Local
compiler labels remain unprefixed.

## Error Handling

Selecting either Arm64 target must create an Arm64 backend. Unsupported CGOPs
fail with a diagnostic containing the target triple and logical opcode; the
compiler must never silently print ARMv7 syntax.

Integrated `-c` remains unsupported for non-default targets. Users validate
the emitted `.s` with the documented external compiler commands.

## Tests

Backend unit tests cover Linux and Apple sections, symbols, prologue/epilogue,
register widths, immediate/address materialization, calls, returns, memory
operations, arithmetic, and conditional branches.

CLI integration tests compile the shared program set for both Arm64 targets.
Generated Apple assembly is assembled with
`clang -target arm64-apple-macos -c`. Linux assembly is assembled with
`aarch64-linux-gnu-gcc -c` when that toolchain is installed. Existing ARMv7
tests and byte comparison remain mandatory.
