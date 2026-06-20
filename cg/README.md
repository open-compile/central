# Code Generator

CG target selection is centralized in `target_info.{h,cxx}`. The default is
`armv7-linux-gnueabihf`; `--target=<triple>` is canonical and the existing
`--arch`/`--march` options accept the same triples and aliases.

Assembly emission is implemented for all V1 targets: ARMv7 Linux, AArch64
Linux, Apple arm64, i386 Linux, x86_64 Linux, and Apple x86_64. Each target is
assembler-validated in CTest; integrated object generation remains separate.

Examples:

```sh
compiler -S input.c -o input.s
compiler -S --target=armv7-linux-gnueabihf input.c -o input.s
compiler -S --arch=arm32 input.c -o input.s
arm-linux-gnueabihf-gcc -marm -march=armv7-a -c input.s
compiler -S --target=aarch64-linux-gnu input.c -o input.s
aarch64-linux-gnu-gcc -c input.s
compiler -S --target=arm64-apple-darwin input.c -o input.s
clang -target arm64-apple-macos -c input.s
compiler -S --target=i386-linux-gnu input.c -o input.s
clang -target i386-linux-gnu -c input.s
compiler -S --target=x86_64-linux-gnu input.c -o input.s
clang -target x86_64-linux-gnu -c input.s
compiler -S --target=x86_64-apple-darwin input.c -o input.s
clang -target x86_64-apple-macos -c input.s
```

`-c` is currently integrated only for the default ARMv7 toolchain. For all
other targets, emit with `-S`, then invoke the external assembler documented
above.
