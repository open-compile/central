#include "target_info.h"
#include "options.h"

#include <cassert>
#include <string>

static void Expect_resolves_to(const char *name, const char *triple) {
  const TARGET_INFO *target = nullptr;
  std::string error;
  assert(Resolve_target(name, &target, &error));
  assert(target != nullptr);
  assert(target->triple == triple);
  assert(error.empty());
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

  COMPILER_CONFIG invalid_config;
  assert(!Configure_target("mips", &invalid_config, &error));
  assert(invalid_config.target.triple.empty());
  return 0;
}
