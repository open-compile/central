#include "toolchain.h"

#include <utility>

namespace {

TOOLCHAIN_DRIVER Driver(TOOLCHAIN_FAMILY family, const std::string &program,
                        std::vector<std::string> target_args) {
  return TOOLCHAIN_DRIVER{family, program, std::move(target_args)};
}

void Add_if_requested(const TOOLCHAIN_DRIVER &driver,
                      const TOOLCHAIN_REQUEST &request,
                      std::vector<TOOLCHAIN_DRIVER> *drivers) {
  if (request.family == AUTO || request.family == driver.family) {
    drivers->push_back(driver);
  }
}

bool Append_groups(const TOOLCHAIN_ARG_GROUPS &groups, const char *prefix,
                   std::vector<std::string> *argv, std::string *error) {
  for (const auto &group : groups) {
    if (group.empty()) {
      *error = "toolchain argument groups must not be empty";
      return false;
    }

    std::string argument(prefix);
    for (const auto &element : group) {
      if (element.empty()) {
        *error = "toolchain argument group elements must not be empty";
        return false;
      }
      argument += ',';
      argument += element;
    }
    argv->push_back(std::move(argument));
  }
  return true;
}

void Initialize_command(const TOOLCHAIN_DRIVER &driver,
                        std::vector<std::string> *argv, std::string *error) {
  argv->clear();
  error->clear();
  argv->push_back(driver.program);
  argv->insert(argv->end(), driver.target_args.begin(), driver.target_args.end());
}

}  // namespace

std::vector<TOOLCHAIN_DRIVER> Toolchain_candidates(
    const std::string &triple, const TOOLCHAIN_REQUEST &request) {
  std::vector<TOOLCHAIN_DRIVER> drivers;

  if (triple == "arm64-apple-darwin") {
    Add_if_requested(Driver(CLANG, "clang",
                            {"-target", "arm64-apple-macos"}),
                     request, &drivers);
  } else if (triple == "x86_64-apple-darwin") {
    Add_if_requested(Driver(CLANG, "clang",
                            {"-target", "x86_64-apple-macos"}),
                     request, &drivers);
  } else if (triple == "x86_64-linux-gnu") {
    Add_if_requested(Driver(GCC, "gcc", {"-m64"}), request, &drivers);
    Add_if_requested(Driver(CLANG, "clang",
                            {"-target", "x86_64-linux-gnu"}),
                     request, &drivers);
  } else if (triple == "i386-linux-gnu") {
    Add_if_requested(Driver(GCC, "gcc", {"-m32"}), request, &drivers);
    Add_if_requested(Driver(CLANG, "clang", {"-target", "i386-linux-gnu"}),
                     request, &drivers);
  } else if (triple == "armv7-linux-gnueabihf") {
    Add_if_requested(Driver(GCC, "arm-linux-gnueabihf-gcc",
                            {"-marm", "-march=armv7-a",
                             "-mfloat-abi=hard"}),
                     request, &drivers);
    Add_if_requested(Driver(CLANG, "clang",
                            {"-target", "armv7-linux-gnueabihf", "-marm",
                             "-march=armv7-a", "-mfloat-abi=hard"}),
                     request, &drivers);
  } else if (triple == "aarch64-linux-gnu") {
    Add_if_requested(Driver(GCC, "aarch64-linux-gnu-gcc", {}), request,
                     &drivers);
    Add_if_requested(Driver(CLANG, "clang",
                            {"-target", "aarch64-linux-gnu"}),
                     request, &drivers);
  }

  return drivers;
}

bool Build_assemble_command(const TOOLCHAIN_DRIVER &driver,
                            const std::string &assembly,
                            const std::string &object,
                            const TOOLCHAIN_ARG_GROUPS &assembler_arg_groups,
                            std::vector<std::string> *argv,
                            std::string *error) {
  Initialize_command(driver, argv, error);
  argv->push_back("-c");
  if (!Append_groups(assembler_arg_groups, "-Wa", argv, error)) {
    argv->clear();
    return false;
  }
  argv->push_back(assembly);
  argv->push_back("-o");
  argv->push_back(object);
  return true;
}

bool Build_link_command(const TOOLCHAIN_DRIVER &driver,
                        const std::string &object,
                        const std::vector<std::string> &link_inputs,
                        const std::string &output,
                        const TOOLCHAIN_ARG_GROUPS &linker_arg_groups,
                        std::vector<std::string> *argv,
                        std::string *error) {
  Initialize_command(driver, argv, error);
  argv->push_back(object);
  argv->insert(argv->end(), link_inputs.begin(), link_inputs.end());
  if (!Append_groups(linker_arg_groups, "-Wl", argv, error)) {
    argv->clear();
    return false;
  }
  argv->push_back("-o");
  argv->push_back(output);
  return true;
}
