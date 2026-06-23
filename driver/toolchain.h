#ifndef OCC_DRIVER_TOOLCHAIN_H
#define OCC_DRIVER_TOOLCHAIN_H

#include <string>
#include <vector>

enum class TOOLCHAIN_FAMILY { AUTO, CLANG, GCC };

struct TOOLCHAIN_REQUEST {
  TOOLCHAIN_FAMILY family = TOOLCHAIN_FAMILY::AUTO;
};

struct TOOLCHAIN_DRIVER {
  TOOLCHAIN_FAMILY family;
  std::string program;
  std::vector<std::string> target_args;
};

using TOOLCHAIN_ARG_GROUPS = std::vector<std::vector<std::string>>;

enum class TOOLCHAIN_LINK_ITEM_KIND { INPUT, LINKER_ARG_GROUP };

struct TOOLCHAIN_LINK_ITEM {
  TOOLCHAIN_LINK_ITEM_KIND kind;
  std::string input;
  std::vector<std::string> linker_args;
};

TOOLCHAIN_LINK_ITEM Link_input(const std::string &input);
TOOLCHAIN_LINK_ITEM Linker_arg_group(std::vector<std::string> linker_args);

struct TOOLCHAIN_COMMAND_RESULT {
  std::vector<std::string> argv;
  std::string error;

  bool ok() const { return error.empty(); }
};

std::vector<TOOLCHAIN_DRIVER> Toolchain_candidates(
    const std::string &triple, const TOOLCHAIN_REQUEST &request);

TOOLCHAIN_COMMAND_RESULT Build_assemble_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &assembly,
    const std::string &object,
    const TOOLCHAIN_ARG_GROUPS &assembler_arg_groups);

TOOLCHAIN_COMMAND_RESULT Build_link_command(
    const TOOLCHAIN_DRIVER &driver, const std::string &object,
    const std::vector<TOOLCHAIN_LINK_ITEM> &items, const std::string &output);

#endif  // OCC_DRIVER_TOOLCHAIN_H
