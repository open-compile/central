#ifndef OCC_DRIVER_TOOLCHAIN_H
#define OCC_DRIVER_TOOLCHAIN_H

#include <string>
#include <vector>

enum TOOLCHAIN_FAMILY { AUTO, CLANG, GCC };

struct TOOLCHAIN_REQUEST {
  TOOLCHAIN_FAMILY family = AUTO;
};

struct TOOLCHAIN_DRIVER {
  TOOLCHAIN_FAMILY family;
  std::string program;
  std::vector<std::string> target_args;
};

using TOOLCHAIN_ARG_GROUPS = std::vector<std::vector<std::string>>;

std::vector<TOOLCHAIN_DRIVER> Toolchain_candidates(
    const std::string &triple, const TOOLCHAIN_REQUEST &request);

bool Build_assemble_command(const TOOLCHAIN_DRIVER &driver,
                            const std::string &assembly,
                            const std::string &object,
                            const TOOLCHAIN_ARG_GROUPS &assembler_arg_groups,
                            std::vector<std::string> *argv,
                            std::string *error);

bool Build_link_command(const TOOLCHAIN_DRIVER &driver,
                        const std::string &object,
                        const std::vector<std::string> &link_inputs,
                        const std::string &output,
                        const TOOLCHAIN_ARG_GROUPS &linker_arg_groups,
                        std::vector<std::string> *argv,
                        std::string *error);

#endif  // OCC_DRIVER_TOOLCHAIN_H
