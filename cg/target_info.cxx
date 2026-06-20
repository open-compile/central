#include "target_info.h"
#include "options.h"

#include <map>
#include <sstream>
#include <utility>

namespace {

TARGET_ABI Make_abi(TARGET_ABI_KIND kind, unsigned pointer_size,
                    unsigned stack_alignment, unsigned register_formal_count,
                    unsigned prologue_save_area, const char *sp,
                    const char *fp, const char *lr, const char *ret,
                    std::vector<std::string> args,
                    std::vector<std::string> caller_saved,
                    std::vector<std::string> callee_saved,
                    std::vector<std::string> allocatable) {
  TARGET_ABI abi;
  abi.kind = kind;
  abi.pointer_size = pointer_size;
  abi.stack_alignment = stack_alignment;
  abi.register_formal_count = register_formal_count;
  abi.stack_argument_slot_size = pointer_size;
  abi.prologue_save_area = prologue_save_area;
  abi.stack_pointer = sp;
  abi.frame_pointer = fp;
  abi.link_register = lr;
  abi.return_register = ret;
  abi.return_register_is_argument_zero =
      kind == TARGET_ABI_KIND::AAPCS32_HARD_FLOAT ||
      kind == TARGET_ABI_KIND::AAPCS64 ||
      kind == TARGET_ABI_KIND::DARWIN_ARM64;
  abi.integer_argument_registers = std::move(args);
  abi.caller_saved_registers = std::move(caller_saved);
  abi.callee_saved_registers = std::move(callee_saved);
  abi.allocatable_registers = std::move(allocatable);
  return abi;
}

const std::vector<TARGET_INFO> &Targets() {
  static const std::vector<TARGET_INFO> targets = {
      {"armv7-linux-gnueabihf", TARGET_ARCH::ARMV7, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::AAPCS32_HARD_FLOAT, 4, 8, 4, 36, "sp",
                "fp", "lr", "r0", {"r0", "r1", "r2", "r3"},
                {"r0", "r1", "r2", "r3", "r12", "lr"},
                {"r4", "r5", "r6", "r7", "r8", "r9", "r10", "fp"},
                {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r10", "fp"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "#",
       "arm-linux-gnueabihf-gcc -marm -march=armv7-a -c", true, true},
      {"aarch64-linux-gnu", TARGET_ARCH::AARCH64, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::AAPCS64, 8, 16, 8, 16, "sp", "x29",
                "x30", "x0", {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x30"},
                {"x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "//", "aarch64-linux-gnu-gcc -c", true, false},
      {"arm64-apple-darwin", TARGET_ARCH::AARCH64, TARGET_OS::DARWIN,
       Make_abi(TARGET_ABI_KIND::DARWIN_ARM64, 8, 16, 8, 16, "sp", "x29",
                "x30", "x0", {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x30"},
                {"x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"}),
       "_", TARGET_SECTION_SYNTAX::MACH_O, "//", "clang -target arm64-apple-macos -c", true, false},
      {"i386-linux-gnu", TARGET_ARCH::I386, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::SYSV_I386, 4, 16, 0, 8, "%esp", "%ebp",
                "", "%eax", {}, {"%eax", "%ecx", "%edx"},
                {"%ebx", "%esi", "%edi", "%ebp"},
                {"%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "#", "gcc -m32 -c", true, false},
      {"x86_64-linux-gnu", TARGET_ARCH::X86_64, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::SYSV_X86_64, 8, 16, 6, 16, "%rsp", "%rbp",
                "", "%rax", {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"},
                {"%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"},
                {"%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15"},
                {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "#", "gcc -m64 -c", true, false},
      {"x86_64-apple-darwin", TARGET_ARCH::X86_64, TARGET_OS::DARWIN,
       Make_abi(TARGET_ABI_KIND::DARWIN_X86_64, 8, 16, 6, 16, "%rsp", "%rbp",
                "", "%rax", {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"},
                {"%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"},
                {"%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15"},
                {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"}),
       "_", TARGET_SECTION_SYNTAX::MACH_O, "#", "clang -target x86_64-apple-macos -c", true, false},
  };
  return targets;
}

const std::map<std::string, std::string> &Aliases() {
  static const std::map<std::string, std::string> aliases = {
      {"armv7", "armv7-linux-gnueabihf"},
      {"arm32", "armv7-linux-gnueabihf"},
      {"aarch64-linux", "aarch64-linux-gnu"},
      {"arm64-linux", "aarch64-linux-gnu"},
      {"arm64-macos", "arm64-apple-darwin"},
      {"x86-linux", "i386-linux-gnu"},
      {"x64-linux", "x86_64-linux-gnu"},
      {"x64-macos", "x86_64-apple-darwin"},
  };
  return aliases;
}

} // namespace

const TARGET_INFO &Default_target() { return Targets().front(); }

const std::vector<TARGET_INFO> &Supported_targets() { return Targets(); }

bool Resolve_target(const std::string &name, const TARGET_INFO **target,
                    std::string *error) {
  if (target != nullptr) {
    *target = nullptr;
  }
  std::string canonical = name.empty() ? Default_target().triple : name;
  const auto alias = Aliases().find(canonical);
  if (alias != Aliases().end()) {
    canonical = alias->second;
  }
  for (const TARGET_INFO &candidate : Targets()) {
    if (candidate.triple == canonical) {
      if (target != nullptr) {
        *target = &candidate;
      }
      if (error != nullptr) {
        error->clear();
      }
      return true;
    }
  }
  if (error != nullptr) {
    std::ostringstream message;
    message << "unknown target '" << name << "'; supported targets:";
    for (const TARGET_INFO &candidate : Targets()) {
      message << " " << candidate.triple;
    }
    *error = message.str();
  }
  return false;
}

namespace {

const char *Arch_name(TARGET_ARCH arch) {
  switch (arch) {
    case TARGET_ARCH::ARMV7: return "armv7";
    case TARGET_ARCH::AARCH64: return "aarch64";
    case TARGET_ARCH::I386: return "i386";
    case TARGET_ARCH::X86_64: return "x86_64";
  }
  return "unknown";
}

const char *Os_name(TARGET_OS os) {
  return os == TARGET_OS::LINUX ? "linux" : "darwin";
}

const char *Abi_name(TARGET_ABI_KIND abi) {
  switch (abi) {
    case TARGET_ABI_KIND::AAPCS32_HARD_FLOAT: return "aapcs32-hard-float";
    case TARGET_ABI_KIND::AAPCS64: return "aapcs64";
    case TARGET_ABI_KIND::SYSV_I386: return "sysv-i386";
    case TARGET_ABI_KIND::SYSV_X86_64: return "sysv-x86_64";
    case TARGET_ABI_KIND::DARWIN_ARM64: return "darwin-arm64";
    case TARGET_ABI_KIND::DARWIN_X86_64: return "darwin-x86_64";
  }
  return "unknown";
}

std::string Join_registers(const std::vector<std::string> &registers) {
  std::ostringstream result;
  for (std::size_t i = 0; i < registers.size(); ++i) {
    if (i != 0) result << ',';
    result << registers[i];
  }
  return result.str();
}

} // namespace

bool Configure_target(const std::string &name, COMPILER_CONFIG *config,
                      std::string *error) {
  const TARGET_INFO *target = nullptr;
  if (config == nullptr || !Resolve_target(name, &target, error)) {
    return false;
  }
  config->target.triple = target->triple;
  config->target.arch = Arch_name(target->arch);
  config->target.os = Os_name(target->os);
  config->target.pointer_size = target->abi.pointer_size;
  config->target.abi = Abi_name(target->abi.kind);
  config->target.symbol_prefix = target->symbol_prefix;
  config->target.section_syntax =
      target->section_syntax == TARGET_SECTION_SYNTAX::GNU_ELF ? "gnu-elf" : "mach-o";
  config->target.stack_alignment = target->abi.stack_alignment;
  config->target.register_set = Join_registers(target->abi.allocatable_registers);
  config->target.external_assembler_hint = target->external_assembler_hint;
  config->target.assembly_supported = target->assembly_supported;
  config->target.integrated_object_supported = target->integrated_object_supported;
  return true;
}
