#ifndef _OCC_TARGET_H_
#define _OCC_TARGET_H_

#include "host.h"
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

enum class HOST_OS {
  LINUX,
  DARWIN,
  UNSUPPORTED,
};

enum class HOST_ARCH {
  ARMV7,
  AARCH64,
  I386,
  X86_64,
  UNSUPPORTED,
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
  std::string assembler;              // standalone assembler binary name
  bool assembly_supported;
  bool integrated_object_supported;
  bool codegen_supported;             // whether our CG pass can generate code for this target
};

const TARGET_INFO &Default_target();
const std::vector<TARGET_INFO> &Supported_targets();

std::string Native_target_name(HOST_OS os, HOST_ARCH arch);
bool Detect_native_target(std::string *triple, std::string *error);

bool Resolve_target(const std::string &name, const TARGET_INFO **target,
                    std::string *error);

const char *Target_arch_name(TARGET_ARCH arch);
bool Parse_target_arch(const std::string &text, TARGET_ARCH *arch);
std::string Supported_target_arches();

// Implemented in cg/target_configure.cxx — requires full COMPILER_CONFIG definition.
bool Configure_target(const std::string &name, COMPILER_CONFIG *config,
                      std::string *error);

#endif // _OCC_TARGET_H_
