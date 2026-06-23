#include "target.h"
#include "options.h"

#include <sstream>

namespace {

const char *Os_name(TARGET_OS os) {
  return os == TARGET_OS::LINUX ? "linux" : "darwin";
}

const char *Abi_name(TARGET_ABI_KIND abi) {
  switch (abi) {
    case TARGET_ABI_KIND::AAPCS32_HARD_FLOAT: return "aapcs32-hard-float";
    case TARGET_ABI_KIND::AAPCS64:            return "aapcs64";
    case TARGET_ABI_KIND::SYSV_I386:          return "sysv-i386";
    case TARGET_ABI_KIND::SYSV_X86_64:        return "sysv-x86_64";
    case TARGET_ABI_KIND::DARWIN_ARM64:       return "darwin-arm64";
    case TARGET_ABI_KIND::DARWIN_X86_64:      return "darwin-x86_64";
  }
  return "unknown";
}

std::string Join_registers(const std::vector<std::string> &regs) {
  std::ostringstream out;
  for (std::size_t i = 0; i < regs.size(); ++i) {
    if (i != 0) out << ',';
    out << regs[i];
  }
  return out.str();
}

} // namespace

bool Configure_target(const std::string &name, COMPILER_CONFIG *config,
                      std::string *error) {
  const TARGET_INFO *target = nullptr;
  if (config == nullptr || !Resolve_configured_target(name, &target, error)) {
    return false;
  }
  config->target.triple                      = target->triple;
  config->target.arch                        = Target_arch_name(target->arch);
  config->target.os                          = Os_name(target->os);
  config->target.pointer_size                = target->abi.pointer_size;
  config->target.abi                         = Abi_name(target->abi.kind);
  config->target.symbol_prefix               = target->symbol_prefix;
  config->target.section_syntax              =
      target->section_syntax == TARGET_SECTION_SYNTAX::GNU_ELF ? "gnu-elf" : "mach-o";
  config->target.stack_alignment             = target->abi.stack_alignment;
  config->target.register_set                = Join_registers(target->abi.allocatable_registers);
  config->target.external_assembler_hint     = target->external_assembler_hint;
  config->target.assembler                   = target->assembler;
  config->target.assembly_supported          = target->assembly_supported;
  config->target.integrated_object_supported = target->integrated_object_supported;
  config->target_arch                        = target->arch;
  return true;
}
