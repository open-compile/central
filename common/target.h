#ifndef _OCC_TARGET_H_
#define _OCC_TARGET_H_

#include "host.h"
#include <string>

enum TARGET_ARCH {
  TARGET_ARCH_ARMV8_A32 = 0,
  TARGET_ARCH_ARMV8_A64,
  TARGET_ARCH_X86_64,
};

struct TARGET_INFO {
  TARGET_ARCH arch;
  const char *name;
  const char *triple;
  UINT32 pointer_size;
  UINT32 word_size;
  UINT32 stack_alignment;
  UINT32 int_arg_regs;
  const char *assembler;
  BOOL codegen_supported;
};

const TARGET_INFO &Target_info(TARGET_ARCH arch);
const char *Target_arch_name(TARGET_ARCH arch);
BOOL Parse_target_arch(const std::string &text, TARGET_ARCH *arch);
std::string Supported_target_arches();

#endif
