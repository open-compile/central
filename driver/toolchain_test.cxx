#include "toolchain.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

using Strings = std::vector<std::string>;

int failures = 0;

void Check(bool condition, const char *expression, int line) {
  if (!condition) {
    std::cerr << "check failed at line " << line << ": " << expression << '\n';
    ++failures;
  }
}

#define CHECK(expression) Check(static_cast<bool>(expression), #expression, __LINE__)

TOOLCHAIN_REQUEST Request(TOOLCHAIN_FAMILY family) {
  TOOLCHAIN_REQUEST request;
  request.family = family;
  return request;
}

void Expect_driver(const TOOLCHAIN_DRIVER &driver, TOOLCHAIN_FAMILY family,
                   const std::string &program, const Strings &target_args) {
  CHECK(driver.family == family);
  CHECK(driver.program == program);
  CHECK(driver.target_args == target_args);
}

void Test_candidate_order_and_arguments() {
  auto drivers = Toolchain_candidates("arm64-apple-darwin",
                                      Request(TOOLCHAIN_FAMILY::AUTO));
  CHECK(drivers.size() == 1);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "arm64-apple-macos"});

  drivers = Toolchain_candidates("x86_64-apple-darwin",
                                 Request(TOOLCHAIN_FAMILY::AUTO));
  CHECK(drivers.size() == 1);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "x86_64-apple-macos"});

  drivers = Toolchain_candidates("x86_64-linux-gnu", Request(TOOLCHAIN_FAMILY::AUTO));
  CHECK(drivers.size() == 2);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::GCC, "gcc", {"-m64"});
  Expect_driver(drivers[1], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "x86_64-linux-gnu"});

  drivers = Toolchain_candidates("i386-linux-gnu", Request(TOOLCHAIN_FAMILY::AUTO));
  CHECK(drivers.size() == 2);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::GCC, "gcc", {"-m32"});
  Expect_driver(drivers[1], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "i386-linux-gnu"});

  drivers = Toolchain_candidates("armv7-linux-gnueabihf",
                                 Request(TOOLCHAIN_FAMILY::AUTO));
  CHECK(drivers.size() == 2);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::GCC, "arm-linux-gnueabihf-gcc",
                {"-marm", "-march=armv7-a", "-mfloat-abi=hard"});
  Expect_driver(drivers[1], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "armv7-linux-gnueabihf", "-marm",
                 "-march=armv7-a", "-mfloat-abi=hard"});

  drivers = Toolchain_candidates("aarch64-linux-gnu", Request(TOOLCHAIN_FAMILY::AUTO));
  CHECK(drivers.size() == 2);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::GCC, "aarch64-linux-gnu-gcc", {});
  Expect_driver(drivers[1], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "aarch64-linux-gnu"});
}

void Test_forced_family_filtering_and_unknown_target() {
  auto drivers = Toolchain_candidates("x86_64-linux-gnu",
                                      Request(TOOLCHAIN_FAMILY::CLANG));
  CHECK(drivers.size() == 1);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::CLANG, "clang",
                {"-target", "x86_64-linux-gnu"});

  drivers = Toolchain_candidates("x86_64-linux-gnu",
                                 Request(TOOLCHAIN_FAMILY::GCC));
  CHECK(drivers.size() == 1);
  Expect_driver(drivers[0], TOOLCHAIN_FAMILY::GCC, "gcc", {"-m64"});

  // Empty is deliberate: Apple /usr/bin/gcc is Clang, and no compatible GNU
  // GCC candidate name can be assumed. The caller diagnoses the empty result.
  CHECK(Toolchain_candidates("arm64-apple-darwin",
                             Request(TOOLCHAIN_FAMILY::GCC)).empty());
  CHECK(Toolchain_candidates("x86_64-apple-darwin",
                             Request(TOOLCHAIN_FAMILY::GCC)).empty());
  CHECK(Toolchain_candidates("mips64-linux-gnu",
                             Request(TOOLCHAIN_FAMILY::AUTO)).empty());
}

void Test_assemble_command_exact_and_multiple_groups() {
  const auto drivers = Toolchain_candidates("arm64-apple-darwin",
                                            Request(TOOLCHAIN_FAMILY::AUTO));
  const auto command = Build_assemble_command(
      drivers[0], "source.s", "source.o",
      {{"-g", "--fatal-warnings"}, {"-I", "inc"}});
  CHECK(command.ok());
  CHECK(command.error.empty());
  CHECK((command.argv == Strings({"clang", "-target", "arm64-apple-macos",
                                  "-c", "-Wa,-g,--fatal-warnings",
                                  "-Wa,-I,inc", "source.s", "-o",
                                  "source.o"})));
}

void Test_link_command_preserves_interleaved_order() {
  const auto drivers = Toolchain_candidates("x86_64-linux-gnu",
                                            Request(TOOLCHAIN_FAMILY::AUTO));
  const auto command = Build_link_command(
      drivers[0], "source.o",
      {Link_input("first.o"), Linker_arg_group({"--whole-archive"}),
       Link_input("liball.a"), Linker_arg_group({"--no-whole-archive"}),
       Link_input("-lc"), Linker_arg_group({"--start-group"}),
       Link_input("liba.a"), Link_input("libb.a"),
       Linker_arg_group({"--end-group"})},
      "program");
  CHECK(command.ok());
  CHECK((command.argv == Strings({"gcc", "-m64", "source.o", "first.o",
                                  "-Wl,--whole-archive", "liball.a",
                                  "-Wl,--no-whole-archive", "-lc",
                                  "-Wl,--start-group", "liba.a", "libb.a",
                                  "-Wl,--end-group", "-o", "program"})));
}

void Test_empty_argument_groups_and_elements_are_rejected() {
  TOOLCHAIN_DRIVER driver{TOOLCHAIN_FAMILY::CLANG, "clang", {}};

  auto command = Build_assemble_command(driver, "source.s", "source.o", {{}});
  CHECK(!command.ok());
  CHECK(command.argv.empty());
  CHECK(!command.error.empty());

  command = Build_assemble_command(driver, "source.s", "source.o",
                                   {{"-g", ""}});
  CHECK(!command.ok());
  CHECK(command.argv.empty());
  CHECK(!command.error.empty());

  command = Build_link_command(
      driver, "source.o", {Linker_arg_group({"", "--gc-sections"})},
      "program");
  CHECK(!command.ok());
  CHECK(command.argv.empty());
  CHECK(!command.error.empty());

  command = Build_link_command(driver, "source.o", {Link_input("")},
                               "program");
  CHECK(!command.ok());
  CHECK(command.argv.empty());
  CHECK(!command.error.empty());
}

}  // namespace

int main() {
  // Run a builder error path first so Release/NDEBUG builds prove checks and
  // validation execute before the successful command examples.
  Test_empty_argument_groups_and_elements_are_rejected();
  Test_candidate_order_and_arguments();
  Test_forced_family_filtering_and_unknown_target();
  Test_assemble_command_exact_and_multiple_groups();
  Test_link_command_preserves_interleaved_order();
  return failures == 0 ? 0 : 1;
}
