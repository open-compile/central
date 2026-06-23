# Code Generator

CG target selection is centralized in `target.{h,cxx}`. The default target is
`armv7-linux-gnueabihf`; `--target=<triple>` is canonical, while `--arch` and
`--march` accept the same triples and aliases. `--target=native` detects the
host OS and architecture and maps them to a supported triple.

The compiler emits assembly itself. Object and executable output then flow
through an external compiler driver: `-S` stops after assembly, `-c` invokes
the selected driver to assemble, and the default mode invokes it again to
link. Compiler drivers are used instead of calling `as` or `ld` directly
because they provide the target's CRT objects, SDK selection, libc, and other
platform link defaults.

## Toolchain selection

`--toolchain=auto` is the default. On macOS, auto selection prefers Clang. On
Linux, including Ubuntu 20.04, 22.04, and 26.04, it prefers GCC and then falls
back to Clang. ARM Linux cross targets first try the target-prefixed GCC driver
(`arm-linux-gnueabihf-gcc` or `aarch64-linux-gnu-gcc`) and then Clang.

Use `-clang` or `-gcc` to force a family. These options are mutually exclusive,
and a forced family never falls back to the other one. Apple targets have no
assumed GNU GCC candidate: `-gcc` fails clearly when no real, compatible GNU
GCC driver is available (Apple's `gcc` command is commonly Clang).

`-Wa,<args>` passes a repeatable argument group to the assembler;
`-Ws,<args>` is an alias. `-Wl,<args>` passes a repeatable group to the linker.
`--keep` publishes successful assembly and object intermediates next to the
requested output.

## Common workflows

```sh
compiler -S --target=native input.c -o input.s
compiler -c --target=native input.c -o input.o
compiler --target=native input.c -o app
compiler --target=x64-linux -gcc -Wa,-g -Wl,-Map,app.map input.c -o app
compiler --target=arm64-macos -clang --keep input.c -o app
```

## Target validation

Object generation is a convenient external validation of each supported
triple. These commands require the corresponding compiler driver, target
multilib, SDK, or sysroot to be installed:

```sh
compiler -c --target=armv7-linux-gnueabihf -gcc input.c -o armv7.o
file armv7.o
compiler -c --target=aarch64-linux-gnu -gcc input.c -o arm64-linux.o
file arm64-linux.o
compiler -c --target=arm64-apple-darwin -clang input.c -o arm64-macos.o
file arm64-macos.o
compiler -c --target=i386-linux-gnu -gcc input.c -o i386.o
file i386.o
compiler -c --target=x86_64-linux-gnu -gcc input.c -o x64-linux.o
file x64-linux.o
compiler -c --target=x86_64-apple-darwin -clang input.c -o x64-macos.o
file x64-macos.o
```
