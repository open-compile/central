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
  if (request.family == TOOLCHAIN_FAMILY::AUTO ||
      request.family == driver.family) {
    drivers->push_back(driver);
  }
}

bool Append_group(const std::vector<std::string> &group, const char *prefix,
                  TOOLCHAIN_COMMAND_RESULT *result) {
  if (group.empty()) {
    result->error = "toolchain argument groups must not be empty";
    return false;
  }

  std::string argument(prefix);
  for (const auto &element : group) {
    if (element.empty()) {
      result->error = "toolchain argument group elements must not be empty";
      return false;
    }
    argument += ',';
    argument += element;
  }
  result->argv.push_back(std::move(argument));
  return true;
}

void Initialize_command(const TOOLCHAIN_DRIVER &driver,
                        TOOLCHAIN_COMMAND_RESULT *result) {
  result->argv.push_back(driver.program);
  result->argv.insert(result->argv.end(), driver.target_args.begin(),
                      driver.target_args.end());
}

TOOLCHAIN_COMMAND_RESULT Invalid_result(TOOLCHAIN_COMMAND_RESULT result) {
  result.argv.clear();
  return result;
}

}  // namespace

TOOLCHAIN_LINK_ITEM Link_input(const std::string &input) {
  return TOOLCHAIN_LINK_ITEM{TOOLCHAIN_LINK_ITEM_KIND::INPUT, input, {}};
}

TOOLCHAIN_LINK_ITEM Linker_arg_group(std::vector<std::string> linker_args) {
  return TOOLCHAIN_LINK_ITEM{TOOLCHAIN_LINK_ITEM_KIND::LINKER_ARG_GROUP, "",
                             std::move(linker_args)};
}

std::vector<TOOLCHAIN_DRIVER> Toolchain_candidates(
    const std::string &triple, const TOOLCHAIN_REQUEST &request) {
  std::vector<TOOLCHAIN_DRIVER> drivers;

  if (triple == "arm64-apple-darwin") {
    // There is intentionally no GNU GCC candidate for Apple targets. Apple's
    // /usr/bin/gcc is Clang, and no compatible GNU executable can be assumed.
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "arm64-apple-macos"}),
                     request, &drivers);
  } else if (triple == "x86_64-apple-darwin") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "x86_64-apple-macos"}),
                     request, &drivers);
  } else if (triple == "x86_64-linux-gnu") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC, "gcc", {"-m64"}), request,
                     &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "x86_64-linux-gnu"}),
                     request, &drivers);
  } else if (triple == "i386-linux-gnu") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC, "gcc", {"-m32"}), request,
                     &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "i386-linux-gnu"}),
                     request, &drivers);
  } else if (triple == "armv7-linux-gnueabihf") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC,
                            "arm-linux-gnueabihf-gcc",
                            {"-marm", "-march=armv7-a",
                             "-mfloat-abi=hard"}),
                     request, &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "armv7-linux-gnueabihf", "-marm",
                             "-march=armv7-a", "-mfloat-abi=hard"}),
                     request, &drivers);
  } else if (triple == "aarch64-linux-gnu") {
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::GCC, "aarch64-linux-gnu-gcc",
                            {}),
                     request, &drivers);
    Add_if_requested(Driver(TOOLCHAIN_FAMILY::CLANG, "clang",
                            {"-target", "aarch64-linux-gnu"}),
                     request, &drivers);
  }

  return drivers;
}

TOOLCHAIN_COMMAND_RESULT Build_assemble_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &assembly,
    const std::string &object,
    const TOOLCHAIN_ARG_GROUPS &assembler_arg_groups) {
  TOOLCHAIN_COMMAND_RESULT result;
  Initialize_command(driver, &result);
  result.argv.push_back("-c");
  for (const auto &group : assembler_arg_groups) {
    if (!Append_group(group, "-Wa", &result)) {
      return Invalid_result(std::move(result));
    }
  }
  result.argv.push_back(assembly);
  result.argv.push_back("-o");
  result.argv.push_back(object);
  return result;
}

TOOLCHAIN_COMMAND_RESULT Build_link_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &object,
    const std::vector<TOOLCHAIN_LINK_ITEM> &items, const std::string &output) {
  TOOLCHAIN_COMMAND_RESULT result;
  Initialize_command(driver, &result);
  result.argv.push_back(object);
  for (const auto &item : items) {
    if (item.kind == TOOLCHAIN_LINK_ITEM_KIND::INPUT) {
      if (item.input.empty()) {
        result.error = "toolchain link inputs must not be empty";
        return Invalid_result(std::move(result));
      }
      result.argv.push_back(item.input);
    } else if (!Append_group(item.linker_args, "-Wl", &result)) {
      return Invalid_result(std::move(result));
    }
  }
  result.argv.push_back("-o");
  result.argv.push_back(output);
  return result;
}
