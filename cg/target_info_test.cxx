#include "target.h"
#include "options.h"

#include <cassert>
#include <cstdio>
#include <string>

static void Expect_resolves_to(const char *name, const char *triple) {
  const TARGET_INFO *target = nullptr;
  std::string error;
  assert(Resolve_target(name, &target, &error));
  assert(target != nullptr);
  assert(target->triple == triple);
  assert(error.empty());
}

static void Expect_native_name(HOST_OS os, HOST_ARCH arch,
                               const char *triple) {
  assert(Native_target_name(os, arch) == triple);
}

int main() {
  const TARGET_INFO &default_target = Default_target();
  assert(default_target.triple == "armv7-linux-gnueabihf");
  assert(default_target.arch == TARGET_ARCH::ARMV7);
  assert(default_target.os == TARGET_OS::LINUX);
  assert(default_target.abi.pointer_size == 4);
  assert(default_target.abi.stack_alignment == 8);
  assert(default_target.abi.register_formal_count == 4);
  assert(default_target.abi.prologue_save_area == 36);

  Expect_native_name(HOST_OS::DARWIN, HOST_ARCH::AARCH64,
                     "arm64-apple-darwin");
  Expect_native_name(HOST_OS::DARWIN, HOST_ARCH::X86_64,
                     "x86_64-apple-darwin");
  Expect_native_name(HOST_OS::LINUX, HOST_ARCH::AARCH64,
                     "aarch64-linux-gnu");
  Expect_native_name(HOST_OS::LINUX, HOST_ARCH::ARMV7,
                     "armv7-linux-gnueabihf");
  Expect_native_name(HOST_OS::LINUX, HOST_ARCH::X86_64,
                     "x86_64-linux-gnu");
  Expect_native_name(HOST_OS::LINUX, HOST_ARCH::I386,
                     "i386-linux-gnu");
  assert(Native_target_name(HOST_OS::DARWIN, HOST_ARCH::ARMV7).empty());
  assert(Native_target_name(HOST_OS::UNSUPPORTED, HOST_ARCH::X86_64).empty());
  assert(Native_target_name(HOST_OS::LINUX, HOST_ARCH::UNSUPPORTED).empty());

  Expect_resolves_to("armv7-linux-gnueabihf", "armv7-linux-gnueabihf");
  Expect_resolves_to("aarch64-linux-gnu", "aarch64-linux-gnu");
  Expect_resolves_to("arm64-apple-darwin", "arm64-apple-darwin");
  Expect_resolves_to("i386-linux-gnu", "i386-linux-gnu");
  Expect_resolves_to("x86_64-linux-gnu", "x86_64-linux-gnu");
  Expect_resolves_to("x86_64-apple-darwin", "x86_64-apple-darwin");

  Expect_resolves_to("armv7", "armv7-linux-gnueabihf");
  Expect_resolves_to("arm32", "armv7-linux-gnueabihf");
  Expect_resolves_to("aarch64-linux", "aarch64-linux-gnu");
  Expect_resolves_to("arm64-linux", "aarch64-linux-gnu");
  Expect_resolves_to("arm64-macos", "arm64-apple-darwin");
  Expect_resolves_to("x86-linux", "i386-linux-gnu");
  Expect_resolves_to("x64-linux", "x86_64-linux-gnu");
  Expect_resolves_to("x64-macos", "x86_64-apple-darwin");

  const TARGET_INFO *target = nullptr;
  std::string error;
  assert(Resolve_target("", &target, &error));
  assert(target == &default_target);
  assert(!Resolve_target("native", &target, &error));
  assert(target == nullptr);
  assert(!Resolve_target("mips64-linux-gnu", &target, &error));
  assert(target == nullptr);
  assert(error.find("unknown target") != std::string::npos);
  assert(error.find("armv7-linux-gnueabihf") != std::string::npos);

  COMPILER_CONFIG config;
  assert(Configure_target("x64-macos", &config, &error));
  assert(config.target.triple == "x86_64-apple-darwin");
  assert(config.target.arch == "x86_64");
  assert(config.target.os == "darwin");
  assert(config.target.pointer_size == 8);
  assert(config.target.abi == "darwin-x86_64");
  assert(config.target.symbol_prefix == "_");
  assert(config.target.section_syntax == "mach-o");
  assert(config.target.stack_alignment == 16);
  assert(config.target.register_set.find("%rax") != std::string::npos);
  assert(config.target.external_assembler_hint.find("clang -target") != std::string::npos);

  std::string native_triple;
  std::printf("[target_info_test] exercising real-host Detect_native_target "
              "(no SIMULATE_HOST_AS_PLATFORM)\n");
  if (Detect_native_target(&native_triple, &error)) {
    assert(!native_triple.empty());
    assert(error.empty());

    const TARGET_INFO *configured_target = nullptr;
    assert(Resolve_configured_target(" NaTiVe ", &configured_target, &error));
    assert(configured_target != nullptr);
    assert(configured_target->triple == native_triple);

    COMPILER_CONFIG native_config;
    assert(Configure_target("native", &native_config, &error));
    assert(native_config.target.triple == native_triple);
    assert(error.empty());

    COMPILER_CONFIG normalized_native_config;
    assert(Configure_target(" NaTiVe ", &normalized_native_config, &error));
    assert(normalized_native_config.target.triple == native_triple);
  } else {
    assert(native_triple.empty());
    assert(!error.empty());
    std::printf("[target_info_test] native host unsupported: %s\n",
                error.c_str());
  }

  COMPILER_CONFIG invalid_config;
  assert(!Configure_target("mips", &invalid_config, &error));
  assert(invalid_config.target.triple.empty());
  return 0;
}
