#ifndef OCC_TARGET_INFO_H
#define OCC_TARGET_INFO_H

#include <string>
#include <vector>

class COMPILER_CONFIG;

enum class TARGET_ARCH {
  ARMV7,
  AARCH64,
  I386,
  X86_64,
};

enum class TARGET_OS {
  LINUX,
  DARWIN,
};

enum class TARGET_ABI_KIND {
  AAPCS32_HARD_FLOAT,
  AAPCS64,
  SYSV_I386,
  SYSV_X86_64,
  DARWIN_ARM64,
  DARWIN_X86_64,
};

enum class TARGET_SECTION_SYNTAX {
  GNU_ELF,
  MACH_O,
};

struct TARGET_ABI {
  TARGET_ABI_KIND kind;
  unsigned pointer_size;
  unsigned stack_alignment;
  unsigned register_formal_count;
  unsigned stack_argument_slot_size;
  unsigned prologue_save_area;
  std::string stack_pointer;
  std::string frame_pointer;
  std::string link_register;
  std::string return_register;
  bool return_register_is_argument_zero;
  std::vector<std::string> integer_argument_registers;
  std::vector<std::string> caller_saved_registers;
  std::vector<std::string> callee_saved_registers;
  std::vector<std::string> allocatable_registers;
};

struct TARGET_INFO {
  std::string triple;
  TARGET_ARCH arch;
  TARGET_OS os;
  TARGET_ABI abi;
  std::string symbol_prefix;
  TARGET_SECTION_SYNTAX section_syntax;
  std::string comment_prefix;
  std::string external_assembler_hint;
  bool assembly_supported;
  bool integrated_object_supported;
};

const TARGET_INFO &Default_target();
const std::vector<TARGET_INFO> &Supported_targets();

// Resolves a canonical triple or documented alias. On failure, target is set
// to null and error contains the accepted canonical triples.
bool Resolve_target(const std::string &name, const TARGET_INFO **target,
                    std::string *error);

// Resolve and copy target metadata into the phase-neutral compiler config.
bool Configure_target(const std::string &name, COMPILER_CONFIG *config,
                      std::string *error);

#endif // OCC_TARGET_INFO_H
