# External Toolchain Driver Design

## Scope

Integrate external assembler and linker execution into the central compiler
driver for macOS and Linux. Central continues to own source-to-assembly code
generation. A discovered Clang, GCC, or cross-GCC driver owns assembly-to-object
and object-to-executable processing.

The default target remains `armv7-linux-gnueabihf`. The new
`--target=native` spelling selects a target from the build host OS and CPU.

The first version supports one source input compiled by central. Existing
object files, archives, `-l` options, and `-L` options may be forwarded to the
link step. Compiling several source files in one invocation is outside this
version.

## Command-Line Contract

Output modes follow GCC conventions:

- `-S` runs central through assembly emission and writes a `.s` file. It never
  invokes an external assembler or linker.
- `-c` emits intermediate assembly, invokes the selected external driver to
  create an object file, and does not link.
- With neither `-S` nor `-c`, central emits assembly, creates an object, and
  invokes the external driver to create an executable.
- `-o` names the final output of the selected mode. Default names are
  `<base>.s`, `<base>.o`, and `a.out`, respectively.
- `--keep` retains intermediate assembly and object files. Otherwise successful
  compilation removes driver-created intermediate files.

Toolchain control options are:

- `--toolchain=auto` is the default. It searches target-appropriate candidates
  in preference order.
- `--toolchain=required` uses the same search order but treats missing tools as
  a direct toolchain diagnostic. This is primarily useful in scripts; `-c` and
  link mode also necessarily fail when `auto` finds no usable tool.
- `--toolchain=off` prevents all external tool execution. It is valid with
  `-S`; using it with `-c` or link mode is an option error.
- `-clang` forces the Clang family and disables GCC fallback.
- `-gcc` forces the GCC family and disables Clang fallback.
- `-clang` and `-gcc` are mutually exclusive.

Pass-through options are repeatable and preserve command-line order within
their destination phase:

- `-Wa,<arg1>,<arg2>` passes assembler arguments through the compiler driver.
- `-Ws,<arg1>,<arg2>` is a central compatibility alias for `-Wa`.
- `-Wl,<arg1>,<arg2>` passes linker arguments through the compiler driver.

Empty comma-separated elements are rejected. Central passes arguments as an
argv vector and never evaluates them with a shell.

## Native Target Resolution

`--target=native` resolves from compile-time host macros:

- Darwin AArch64: `arm64-apple-darwin`
- Darwin x86-64: `x86_64-apple-darwin`
- Linux AArch64: `aarch64-linux-gnu`
- Linux ARM 32-bit: `armv7-linux-gnueabihf`
- Linux x86-64: `x86_64-linux-gnu`
- Linux i386: `i386-linux-gnu`

An unsupported host OS or CPU produces an option error. An omitted target still
resolves to `armv7-linux-gnueabihf`; native selection is never implicit.

## Toolchain Selection

Toolchain selection is target-driven and uses executable discovery through
`PATH`. The driver invokes a candidate with `--version` to distinguish an
executable tool from an unusable entry. Actual assemble and link invocations
remain the authoritative capability tests because, for example, `gcc -m32`
can exist while 32-bit startup objects are missing.

Candidate preferences are:

- Apple targets: `clang -target <apple-target>` first; GCC is considered only
  when explicitly forced and a compatible GCC executable exists.
- Native Linux x86-64/i386: `gcc -m64` or `gcc -m32`, then
  `clang -target <linux-target>`.
- ARM Linux: target-prefixed GCC (`arm-linux-gnueabihf-gcc` or
  `aarch64-linux-gnu-gcc`), then Clang with the matching `-target` triple.

Forcing `-clang` or `-gcc` filters this candidate list before probing. A forced
family never falls back to the other family.

The selected compiler driver is used for both phases. This deliberately avoids
direct `ld` use so the platform driver supplies CRT objects, SDK paths, dynamic
linker settings, libc, and platform defaults.

## Driver Architecture

Create a focused toolchain module under `driver/`:

- `toolchain.h/.cxx` defines output mode, toolchain family, candidate discovery,
  argv construction, subprocess execution, and intermediate cleanup.
- Target metadata continues to describe code-generation ABI properties. The
  toolchain module derives external invocation flags from the resolved target
  triple rather than embedding process execution in CG backends.
- `main.cxx` parses command-line intent, resolves the target and output paths,
  runs FE/OPT/CG to an assembly path, then delegates object/link stages to the
  toolchain module.

The CG emitter always receives the actual assembly output path. For `-c` and
link mode this is a deterministic sibling temporary path that cannot overwrite
the requested final output. Cleanup occurs only for files created by the
current invocation.

## External Commands

Object mode has this conceptual shape:

```text
<driver> <target-flags> -c <assembler-pass-through> input.s -o output.o
```

Link mode has this conceptual shape:

```text
<driver> <target-flags> input.o <link-inputs> <linker-pass-through> -o output
```

Assembler pass-through uses the selected driver's `-Wa,<joined-args>` form.
Linker pass-through uses `-Wl,<joined-args>`. This preserves GCC/Clang driver
semantics instead of interpreting raw assembler or linker flags inside central.

## Diagnostics and Failure Handling

Diagnostics identify the target, selected or requested family, phase, command
argv, and child exit status. Missing tools, mutually exclusive options,
`--toolchain=off` conflicts, invalid pass-through syntax, assembly failure, and
link failure return non-zero driver status.

On failure, intermediate files are retained to support diagnosis. On success,
they are removed unless `--keep` was specified. The final output is never
reported as successful unless the child process exits successfully.

## Tests

Unit tests cover:

- native target mapping for supported compile-time hosts;
- target/family candidate ordering;
- `-clang`/`-gcc` mutual exclusion and forced-family behavior;
- parsing `-Wa`, `-Ws`, and `-Wl` values;
- `-S`, `-c`, and link output-path decisions;
- argv construction without shell quoting;
- cleanup policy on success and failure.

Driver integration tests use fake Clang/GCC executables placed first in `PATH`
to record argv and create requested outputs. They verify that `-S` runs no
external process, `-c` runs only the object command, link mode runs object then
link commands, pass-through arguments reach only their intended phase, and
forced family selection does not fall back.

Platform validation additionally performs real Clang assembly and linking for
Apple arm64 and x86-64 on macOS. Linux CI performs native GCC validation and,
when installed, cross-GCC validation for ARM targets. Tests skip only optional
cross-toolchains; native toolchain validation is required on supported CI
hosts.
