#include "toolchain.h"

#include <cassert>
#include <string>
#include <vector>

namespace {

using Strings = std::vector<std::string>;

TOOLCHAIN_REQUEST Request(TOOLCHAIN_FAMILY family) {
  TOOLCHAIN_REQUEST request;
  request.family = family;
  return request;
}

void Expect_driver(const TOOLCHAIN_DRIVER &driver, TOOLCHAIN_FAMILY family,
                   const std::string &program, const Strings &target_args) {
  assert(driver.family == family);
  assert(driver.program == program);
  assert(driver.target_args == target_args);
}

void Test_candidate_order_and_arguments() {
  auto drivers = Toolchain_candidates("arm64-apple-darwin", Request(AUTO));
  assert(drivers.size() == 1);
  Expect_driver(drivers[0], CLANG, "clang",
                {"-target", "arm64-apple-macos"});

  drivers = Toolchain_candidates("x86_64-apple-darwin", Request(AUTO));
  assert(drivers.size() == 1);
  Expect_driver(drivers[0], CLANG, "clang",
                {"-target", "x86_64-apple-macos"});

  drivers = Toolchain_candidates("x86_64-linux-gnu", Request(AUTO));
  assert(drivers.size() == 2);
  Expect_driver(drivers[0], GCC, "gcc", {"-m64"});
  Expect_driver(drivers[1], CLANG, "clang",
                {"-target", "x86_64-linux-gnu"});

  drivers = Toolchain_candidates("i386-linux-gnu", Request(AUTO));
  assert(drivers.size() == 2);
  Expect_driver(drivers[0], GCC, "gcc", {"-m32"});
  Expect_driver(drivers[1], CLANG, "clang",
                {"-target", "i386-linux-gnu"});

  drivers = Toolchain_candidates("armv7-linux-gnueabihf", Request(AUTO));
  assert(drivers.size() == 2);
  Expect_driver(drivers[0], GCC, "arm-linux-gnueabihf-gcc",
                {"-marm", "-march=armv7-a", "-mfloat-abi=hard"});
  Expect_driver(drivers[1], CLANG, "clang",
                {"-target", "armv7-linux-gnueabihf", "-marm",
                 "-march=armv7-a", "-mfloat-abi=hard"});

  drivers = Toolchain_candidates("aarch64-linux-gnu", Request(AUTO));
  assert(drivers.size() == 2);
  Expect_driver(drivers[0], GCC, "aarch64-linux-gnu-gcc", {});
  Expect_driver(drivers[1], CLANG, "clang",
                {"-target", "aarch64-linux-gnu"});
}

void Test_forced_family_filtering_and_unknown_target() {
  auto drivers =
      Toolchain_candidates("x86_64-linux-gnu", Request(CLANG));
  assert(drivers.size() == 1);
  Expect_driver(drivers[0], CLANG, "clang",
                {"-target", "x86_64-linux-gnu"});

  drivers = Toolchain_candidates("x86_64-linux-gnu", Request(GCC));
  assert(drivers.size() == 1);
  Expect_driver(drivers[0], GCC, "gcc", {"-m64"});

  assert(Toolchain_candidates("arm64-apple-darwin", Request(GCC)).empty());
  assert(Toolchain_candidates("x86_64-apple-darwin", Request(GCC)).empty());
  assert(Toolchain_candidates("mips64-linux-gnu", Request(AUTO)).empty());
}

void Test_assemble_command_exact_and_multiple_groups() {
  const auto drivers =
      Toolchain_candidates("arm64-apple-darwin", Request(AUTO));
  Strings command;
  std::string error;
  assert(Build_assemble_command(drivers[0], "source.s", "source.o",
                                {{"-g", "--fatal-warnings"}, {"-I", "inc"}},
                                &command, &error));
  assert(error.empty());
  assert(command == Strings({"clang", "-target", "arm64-apple-macos", "-c",
                             "-Wa,-g,--fatal-warnings", "-Wa,-I,inc",
                             "source.s", "-o", "source.o"}));
}

void Test_link_command_exact_and_multiple_groups() {
  const auto drivers =
      Toolchain_candidates("x86_64-linux-gnu", Request(AUTO));
  Strings command;
  std::string error;
  assert(Build_link_command(drivers[0], "source.o",
                            {"first.o", "-lc", "last.a"}, "program",
                            {{"--gc-sections"}, {"-rpath", "/opt/lib"}},
                            &command, &error));
  assert(error.empty());
  assert(command == Strings({"gcc", "-m64", "source.o", "first.o", "-lc",
                             "last.a", "-Wl,--gc-sections",
                             "-Wl,-rpath,/opt/lib", "-o", "program"}));
}

void Test_empty_argument_groups_and_elements_are_rejected() {
  TOOLCHAIN_DRIVER driver{CLANG, "clang", {}};
  Strings command{"stale"};
  std::string error;

  assert(!Build_assemble_command(driver, "source.s", "source.o", {{}},
                                 &command, &error));
  assert(command.empty());
  assert(!error.empty());

  error.clear();
  command = {"stale"};
  assert(!Build_assemble_command(driver, "source.s", "source.o",
                                 {{"-g", ""}}, &command, &error));
  assert(command.empty());
  assert(!error.empty());

  error.clear();
  command = {"stale"};
  assert(!Build_link_command(driver, "source.o", {}, "program",
                             {{"", "--gc-sections"}}, &command, &error));
  assert(command.empty());
  assert(!error.empty());
}

}  // namespace

int main() {
  Test_candidate_order_and_arguments();
  Test_forced_family_filtering_and_unknown_target();
  Test_assemble_command_exact_and_multiple_groups();
  Test_link_command_exact_and_multiple_groups();
  Test_empty_argument_groups_and_elements_are_rejected();
  return 0;
}
