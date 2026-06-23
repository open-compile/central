# External Toolchain Driver Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make central select a native or cross Clang/GCC driver and implement GCC-style `-S`, `-c`, and executable-link workflows on macOS and Linux.

**Architecture:** Keep CG responsible only for producing target assembly. Add a focused `driver/toolchain` module that resolves native targets, discovers a target-compatible compiler driver, builds argv vectors for assembly and linking, executes them without a shell, and owns cleanup of intermediates. `driver/main.cxx` parses user intent and orchestrates FE/OPT/CG followed by the external stages.

**Tech Stack:** C++14, CMake/CTest, existing `subprocess.h`, Clang/GCC compiler drivers, Mach-O and ELF targets.

---

## File Structure

- Create `driver/toolchain.h`: public enums and pure/testable APIs for host mapping, candidate selection, command construction, probing, execution, and cleanup.
- Create `driver/toolchain.cxx`: external toolchain implementation; no CG lowering logic.
- Create `driver/toolchain_test.cxx`: unit tests for host mapping, selection order, argv construction, and forwarded arguments.
- Create `driver/fake_toolchain.cxx`: test executable that logs argv and creates the path following `-o`.
- Create `driver/external_toolchain_test.cmake`: fake-tool integration tests for phase selection and diagnostics.
- Create `driver/native_toolchain_test.cmake`: real native `-c` and link smoke tests.
- Modify `common/options.h`: store output mode, requested toolchain policy/family, intermediate/final paths, pass-through arguments, link inputs, and keep policy.
- Modify `common/target.h` and `common/target.cxx`: expose host-to-native-target resolution as a pure function and recognize `native` through configuration.
- Modify `cg/target_configure.cxx`: resolve `native` before copying target metadata.
- Modify `driver/main.cxx`: parse GCC-compatible options, compute paths, and call the external toolchain module.
- Modify `driver/CMakeLists.txt`: compile the module and register tests.
- Modify `driver/target_cli_test.cmake`, `driver/arm64_cli_test.cmake`, and `cg/README.md`: replace the obsolete “integrated `-c` unsupported” contract.

### Task 1: Add deterministic native-target resolution

**Files:**
- Modify: `common/target.h`
- Modify: `common/target.cxx`
- Modify: `cg/target_configure.cxx`
- Test: `cg/target_info_test.cxx`

- [ ] **Step 1: Write failing host-mapping tests**

Add a testable host description and assertions independent of the machine running CTest:

```cpp
assert(Native_target_name(HOST_OS::DARWIN, HOST_ARCH::AARCH64) ==
       "arm64-apple-darwin");
assert(Native_target_name(HOST_OS::DARWIN, HOST_ARCH::X86_64) ==
       "x86_64-apple-darwin");
assert(Native_target_name(HOST_OS::LINUX, HOST_ARCH::AARCH64) ==
       "aarch64-linux-gnu");
assert(Native_target_name(HOST_OS::LINUX, HOST_ARCH::ARMV7) ==
       "armv7-linux-gnueabihf");
assert(Native_target_name(HOST_OS::LINUX, HOST_ARCH::X86_64) ==
       "x86_64-linux-gnu");
assert(Native_target_name(HOST_OS::LINUX, HOST_ARCH::I386) ==
       "i386-linux-gnu");
```

- [ ] **Step 2: Run the test and verify RED**

Run:

```bash
cmake --build /private/tmp/central-multitarget-build --target target_info_test -j4
```

Expected: compilation fails because `HOST_OS`, `HOST_ARCH`, and
`Native_target_name` do not exist.

- [ ] **Step 3: Implement host mapping and `native` resolution**

Add to `common/target.h`:

```cpp
enum class HOST_OS { LINUX, DARWIN, UNSUPPORTED };
enum class HOST_ARCH { ARMV7, AARCH64, I386, X86_64, UNSUPPORTED };

std::string Native_target_name(HOST_OS os, HOST_ARCH arch);
bool Detect_native_target(std::string *triple, std::string *error);
```

Implement the six mappings in `common/target.cxx`. Implement
`Detect_native_target` using `__APPLE__`, `__linux__`, `__aarch64__`,
`__arm__`, `__x86_64__`, and `__i386__`. In `Configure_target`, translate the
exact normalized spelling `native` through `Detect_native_target` before
calling `Resolve_target`. Do not add `native` as an alias to the default target.

- [ ] **Step 4: Run target tests and verify GREEN**

Run:

```bash
cmake --build /private/tmp/central-multitarget-build --target target_info_test -j4
/private/tmp/central-multitarget-build/cg/target_info_test
```

Expected: exit status 0.

- [ ] **Step 5: Commit**

```bash
git add common/target.h common/target.cxx cg/target_configure.cxx cg/target_info_test.cxx
git commit -m "feat: resolve native compilation target"
```

### Task 2: Define external toolchain selection and command construction

**Files:**
- Create: `driver/toolchain.h`
- Create: `driver/toolchain.cxx`
- Create: `driver/toolchain_test.cxx`
- Modify: `driver/CMakeLists.txt`

- [ ] **Step 1: Write failing selection and argv tests**

Define tests for the intended public API:

```cpp
TOOLCHAIN_REQUEST auto_request{TOOLCHAIN_FAMILY::AUTO};
auto apple = Toolchain_candidates("arm64-apple-darwin", auto_request);
assert(apple[0].program == "clang");
assert(apple[0].target_args ==
       std::vector<std::string>({"-target", "arm64-apple-macos"}));

auto linux = Toolchain_candidates("x86_64-linux-gnu", auto_request);
assert(linux[0].program == "gcc");
assert(linux[0].target_args == std::vector<std::string>({"-m64"}));
assert(linux[1].program == "clang");

TOOLCHAIN_REQUEST clang_only{TOOLCHAIN_FAMILY::CLANG};
auto forced = Toolchain_candidates("aarch64-linux-gnu", clang_only);
assert(forced.size() == 1 && forced[0].program == "clang");

TOOLCHAIN_DRIVER driver{"clang", {"-target", "arm64-apple-macos"}};
assert(Build_assemble_command(driver, "in.s", "out.o", {"-g"}) ==
       std::vector<std::string>({"clang", "-target", "arm64-apple-macos",
                                 "-c", "-Wa,-g", "in.s", "-o", "out.o"}));
assert(Build_link_command(driver, "in.o", {"libx.a"}, "a.out",
                          {"-dead_strip"}) ==
       std::vector<std::string>({"clang", "-target", "arm64-apple-macos",
                                 "in.o", "libx.a", "-Wl,-dead_strip",
                                 "-o", "a.out"}));
```

Also assert GCC-first/Clang-second for Linux i386, GCC-cross-first for both ARM
Linux targets, forced GCC filtering, unsupported triples, and preservation of
forwarded argument order.

- [ ] **Step 2: Run the test and verify RED**

Run:

```bash
cmake --build /private/tmp/central-multitarget-build --target toolchain_test -j4
```

Expected: CMake or compilation fails because the target and APIs do not exist.

- [ ] **Step 3: Implement the pure toolchain API**

Define in `driver/toolchain.h`:

```cpp
enum class TOOLCHAIN_FAMILY { AUTO, CLANG, GCC };

struct TOOLCHAIN_REQUEST {
  TOOLCHAIN_FAMILY family;
};

struct TOOLCHAIN_DRIVER {
  std::string program;
  std::vector<std::string> target_args;
};

std::vector<TOOLCHAIN_DRIVER> Toolchain_candidates(
    const std::string &triple, const TOOLCHAIN_REQUEST &request);
std::vector<std::string> Build_assemble_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &assembly,
    const std::string &object, const std::vector<std::string> &assembler_args);
std::vector<std::string> Build_link_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &object,
    const std::vector<std::string> &link_inputs, const std::string &output,
    const std::vector<std::string> &linker_args);
```

Use a table keyed by canonical triple. Join assembler and linker lists into
one `-Wa,...` or `-Wl,...` argument per original command-line occurrence so
ordering remains stable. Do not quote or concatenate a shell command.

- [ ] **Step 4: Register and run the unit test**

Add `toolchain.cxx` to `compiler`, add `toolchain_test`, then run:

```bash
cmake -S . -B /private/tmp/central-multitarget-build -DBUILD_TESTING=ON
cmake --build /private/tmp/central-multitarget-build --target toolchain_test -j4
/private/tmp/central-multitarget-build/driver/toolchain_test
```

Expected: exit status 0.

- [ ] **Step 5: Commit**

```bash
git add driver/toolchain.h driver/toolchain.cxx driver/toolchain_test.cxx driver/CMakeLists.txt
git commit -m "feat: model external compiler toolchains"
```

### Task 3: Parse GCC-style modes and pass-through arguments

**Files:**
- Modify: `common/options.h`
- Modify: `driver/main.cxx`
- Create: `driver/external_toolchain_test.cmake`
- Create: `driver/fake_toolchain.cxx`
- Modify: `driver/CMakeLists.txt`

- [ ] **Step 1: Add failing CLI parsing tests**

Build `fake_toolchain` so `--version` exits 0 and every other invocation appends
its executable name and argv to `$CENTRAL_FAKE_TOOL_LOG`, then creates the file
named after `-o`. In CMake, create `clang`, `gcc`, and cross-GCC symlinks to this
binary in a private directory prepended to `PATH`.

Add cases that expect:

```text
-S --target=native                 => no fake-tool log
-c --target=x64-linux -gcc         => gcc object command only
--target=x64-linux -clang          => clang object command, then clang link command
-clang -gcc                        => option error
--toolchain=off -c                 => option error
-Wa,-g,-Ifoo -Ws,--fatal-warnings  => assembler command only
-Wl,-Map,out.map -Wl,--gc-sections => linker command only
```

- [ ] **Step 2: Run the integration test and verify RED**

Run:

```bash
ctest --test-dir /private/tmp/central-multitarget-build \
  -R external_toolchain_test --output-on-failure
```

Expected: test is absent or fails because the new options are unrecognized.

- [ ] **Step 3: Add driver configuration fields**

Add to `common/options.h`:

```cpp
enum class DRIVER_OUTPUT_MODE { ASSEMBLY, OBJECT, LINK };
enum class EXTERNAL_TOOLCHAIN_MODE { AUTO, REQUIRED, OFF };
enum class EXTERNAL_TOOLCHAIN_FAMILY { AUTO, CLANG, GCC };

DRIVER_OUTPUT_MODE output_mode = DRIVER_OUTPUT_MODE::LINK;
EXTERNAL_TOOLCHAIN_MODE toolchain_mode = EXTERNAL_TOOLCHAIN_MODE::AUTO;
EXTERNAL_TOOLCHAIN_FAMILY toolchain_family = EXTERNAL_TOOLCHAIN_FAMILY::AUTO;
std::vector<std::vector<std::string>> assembler_arg_groups;
std::vector<std::vector<std::string>> linker_arg_groups;
std::vector<std::string> link_inputs;
std::string assembly_output_file;
std::string object_output_file;
std::string final_output_file;
BOOL keep_intermediates = FALSE;
```

- [ ] **Step 4: Implement argument pre-filtering and validation**

Before `args.hxx` parsing, consume repeatable `-Wa,`, `-Ws,`, and `-Wl,`
arguments, split their suffix on commas, reject empty suffix/elements, and store
groups. Add `-clang` and `-gcc` flags plus `--toolchain=<auto|required|off>`.
Reject mutually exclusive family flags. Classify positional `.o`, `.a`, `.so`,
and `.dylib` paths as link inputs; require exactly one remaining source input.
Accept repeatable `-L` and `-l` options as linker inputs.

- [ ] **Step 5: Implement GCC-style output path decisions**

Set:

```text
-S:    final/assembly = -o or <source-base>.s
-c:    final/object   = -o or <source-base>.o
link:  final          = -o or a.out
```

For object/link mode, use sibling intermediate names containing the process ID,
for example `.<base>.central-<pid>.s` and `.<base>.central-<pid>.o`. Assign
`config.output_file = config.assembly_output_file` before CG so existing CG
continues to write only assembly. With `--keep`, use visible `<base>.s` and
`<base>.o` names instead.

- [ ] **Step 6: Run parsing integration tests and verify GREEN**

Run:

```bash
cmake --build /private/tmp/central-multitarget-build -j4
ctest --test-dir /private/tmp/central-multitarget-build \
  -R external_toolchain_test --output-on-failure
```

Expected: all parsing/path cases pass.

- [ ] **Step 7: Commit**

```bash
git add common/options.h driver/main.cxx driver/fake_toolchain.cxx \
  driver/external_toolchain_test.cmake driver/CMakeLists.txt
git commit -m "feat: parse external toolchain driver options"
```

### Task 4: Probe and execute external assembler/link stages

**Files:**
- Modify: `driver/toolchain.h`
- Modify: `driver/toolchain.cxx`
- Modify: `driver/main.cxx`
- Test: `driver/toolchain_test.cxx`
- Test: `driver/external_toolchain_test.cmake`

- [ ] **Step 1: Add failing execution-policy tests**

Extend fake-tool tests to verify:

- auto mode skips an unavailable first candidate and chooses the second;
- `-gcc` does not fall back to Clang;
- failed `--version` is treated as unavailable;
- child object/link non-zero status is returned by central;
- intermediates are removed after success, retained after failure, and retained
  after success with `--keep`;
- diagnostics contain target triple, phase, argv, and exit status.

- [ ] **Step 2: Run tests and verify RED**

Run:

```bash
ctest --test-dir /private/tmp/central-multitarget-build \
  -R 'toolchain_test|external_toolchain_test' --output-on-failure
```

Expected: execution-policy assertions fail because no subprocess orchestration
exists.

- [ ] **Step 3: Implement safe subprocess execution**

Add:

```cpp
bool Discover_toolchain(const std::string &triple,
                        const TOOLCHAIN_REQUEST &request,
                        TOOLCHAIN_DRIVER *selected,
                        std::string *error);
int Run_tool_command(const std::vector<std::string> &argv,
                     const char *phase, const std::string &triple,
                     std::string *error);
int Run_external_toolchain(COMPILER_CONFIG *config, std::string *error);
```

`Discover_toolchain` probes each filtered candidate with `--version`.
`Run_tool_command` converts stable `std::string` storage into a null-terminated
`const char *` vector, calls `subprocess_create` with inherited environment,
joins it, destroys the subprocess, and returns the child exit status. It must
not use `system`, `popen`, or a shell.

`Run_external_toolchain` performs object creation for OBJECT/LINK modes and the
link command only for LINK mode. It removes only driver-created intermediates
after full success unless `keep_intermediates` is true. Any failure retains
intermediates.

- [ ] **Step 4: Replace the legacy ASM component**

In `Execute`, always run FE/BE/CG for the three output modes. Return immediately
after CG for ASSEMBLY. For OBJECT/LINK, call `Run_external_toolchain` and return
its status. Delete the fixed-size `operands[10]` ASM implementation and the old
`integrated_object_supported` rejection; keep `COMPONENT_ASM` tracing around
the external object stage and use `COMPONENT_LD` tracing for linking.

- [ ] **Step 5: Run focused tests and verify GREEN**

Run:

```bash
cmake --build /private/tmp/central-multitarget-build -j4
ctest --test-dir /private/tmp/central-multitarget-build \
  -R 'toolchain_test|external_toolchain_test' --output-on-failure
```

Expected: all focused tests pass.

- [ ] **Step 6: Commit**

```bash
git add driver/toolchain.h driver/toolchain.cxx driver/toolchain_test.cxx \
  driver/main.cxx driver/external_toolchain_test.cmake
git commit -m "feat: assemble and link through external toolchains"
```

### Task 5: Update existing CLI contracts and documentation

**Files:**
- Modify: `driver/target_cli_test.cmake`
- Modify: `driver/arm64_cli_test.cmake`
- Modify: `driver/x86_cli_test.cmake`
- Modify: `cg/README.md`
- Modify: `driver/main.cxx`

- [ ] **Step 1: Write failing contract assertions**

Replace the old expected `-c` failure with fake or available-tool assertions
that the requested `.o` exists. Add help-output checks for `--target=native`,
`--toolchain`, `-clang`, `-gcc`, `-Wa`, `-Ws`, and `-Wl`. Keep current `-S`
assembler syntax validation for every target.

- [ ] **Step 2: Run CLI tests and verify RED**

Run:

```bash
ctest --test-dir /private/tmp/central-multitarget-build \
  -R 'target_cli_test|arm64_cli_test|x86_cli_test' --output-on-failure
```

Expected: old integrated-object diagnostics or stale help text cause failure.

- [ ] **Step 3: Update help and README**

Document these canonical examples:

```bash
compiler -S --target=native input.c -o input.s
compiler -c --target=native input.c -o input.o
compiler --target=native input.c -o app
compiler --target=x64-linux -gcc -Wa,-g -Wl,-Map,app.map input.c -o app
compiler --target=arm64-macos -clang --keep input.c -o app
```

State that default target remains ARMv7, Linux prefers GCC then Clang, macOS
prefers Clang, and forced family selection never falls back.

- [ ] **Step 4: Run CLI tests and verify GREEN**

Run the same CTest filter and expect all tests to pass.

- [ ] **Step 5: Commit**

```bash
git add driver/target_cli_test.cmake driver/arm64_cli_test.cmake \
  driver/x86_cli_test.cmake driver/main.cxx cg/README.md
git commit -m "docs: describe integrated external toolchain workflow"
```

### Task 6: Real native macOS/Linux validation

**Files:**
- Create: `driver/native_toolchain_test.cmake`
- Modify: `driver/CMakeLists.txt`

- [ ] **Step 1: Add the native smoke test**

The test must run:

```text
compiler -S --target=native add2_main.sy -o native.s
compiler -c --target=native add2_main.sy -o native.o
compiler --target=native add2_main.sy -o native-app
```

It verifies all outputs exist, uses `file` when available to check ELF/Mach-O,
and runs `native-app`, expecting exit status 3. On macOS, add explicit
`arm64-apple-darwin` and `x86_64-apple-darwin` assemble/link cases when the host
Clang supports both targets. On Linux, require native GCC or Clang and attempt
installed ARM cross-GCC candidates without failing when an optional cross
compiler is absent.

- [ ] **Step 2: Run the test before registration and verify RED**

Run:

```bash
ctest --test-dir /private/tmp/central-multitarget-build \
  -R native_toolchain_test --output-on-failure
```

Expected: no such registered test.

- [ ] **Step 3: Register and run real validation**

Register `native_toolchain_test` in `driver/CMakeLists.txt`, rebuild, then run:

```bash
cmake --build /private/tmp/central-multitarget-build -j4
ctest --test-dir /private/tmp/central-multitarget-build \
  -R native_toolchain_test --output-on-failure
```

Expected on the current macOS host: native arm64 `.s`, `.o`, and Mach-O
executable are created and the executable exits 3; explicit x86-64 Mach-O link
also succeeds.

- [ ] **Step 4: Run the full regression suite**

Run:

```bash
ctest --test-dir /private/tmp/central-multitarget-build --output-on-failure
```

Expected: 100% tests passed.

- [ ] **Step 5: Inspect final changes**

Run:

```bash
git diff --check
git status --short
```

Expected: no whitespace errors; only intended implementation files and the
pre-existing unrelated `build-macos/` directory remain.

- [ ] **Step 6: Commit**

```bash
git add driver/native_toolchain_test.cmake driver/CMakeLists.txt
git commit -m "test: validate native external toolchain workflow"
```
