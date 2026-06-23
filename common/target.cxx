#include "target.h"

#include <algorithm>
#include <cctype>
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
       "arm-linux-gnueabihf-gcc -marm -march=armv7-a -c",
       "arm-linux-gnueabihf-as",
       /*assembly_supported=*/true, /*integrated_object_supported=*/true,
       /*codegen_supported=*/true},
      {"aarch64-linux-gnu", TARGET_ARCH::AARCH64, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::AAPCS64, 8, 16, 8, 16, "sp", "x29",
                "x30", "x0", {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
                 "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x30"},
                {"x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
                 "x10", "x11", "x12", "x13", "x14", "x15", "x19", "x20", "x21",
                 "x22", "x23", "x24", "x25", "x26", "x27", "x28"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "//",
       "aarch64-linux-gnu-gcc -c",
       "aarch64-linux-gnu-as",
       /*assembly_supported=*/true, /*integrated_object_supported=*/false,
       /*codegen_supported=*/true},
      {"arm64-apple-darwin", TARGET_ARCH::AARCH64, TARGET_OS::DARWIN,
       Make_abi(TARGET_ABI_KIND::DARWIN_ARM64, 8, 16, 8, 16, "sp", "x29",
                "x30", "x0", {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
                 "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x30"},
                {"x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29"},
                {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
                 "x10", "x11", "x12", "x13", "x14", "x15", "x19", "x20", "x21",
                 "x22", "x23", "x24", "x25", "x26", "x27", "x28"}),
       "_", TARGET_SECTION_SYNTAX::MACH_O, "//",
       "clang -target arm64-apple-macos -c",
       "as",
       /*assembly_supported=*/true, /*integrated_object_supported=*/false,
       /*codegen_supported=*/true},
      {"i386-linux-gnu", TARGET_ARCH::I386, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::SYSV_I386, 4, 16, 0, 8, "%esp", "%ebp",
                "", "%eax", {}, {"%eax", "%ecx", "%edx"},
                {"%ebx", "%esi", "%edi", "%ebp"},
                {"%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "#",
       "gcc -m32 -c",
       "as",
       /*assembly_supported=*/true, /*integrated_object_supported=*/false,
       /*codegen_supported=*/true},
      {"x86_64-linux-gnu", TARGET_ARCH::X86_64, TARGET_OS::LINUX,
       Make_abi(TARGET_ABI_KIND::SYSV_X86_64, 8, 16, 6, 16, "%rsp", "%rbp",
                "", "%rax",
                {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"},
                {"%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"},
                {"%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15"},
                {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi",
                 "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"}),
       "", TARGET_SECTION_SYNTAX::GNU_ELF, "#",
       "gcc -m64 -c",
       "as",
       /*assembly_supported=*/true, /*integrated_object_supported=*/false,
       /*codegen_supported=*/true},
      {"x86_64-apple-darwin", TARGET_ARCH::X86_64, TARGET_OS::DARWIN,
       Make_abi(TARGET_ABI_KIND::DARWIN_X86_64, 8, 16, 6, 16, "%rsp", "%rbp",
                "", "%rax",
                {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"},
                {"%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"},
                {"%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15"},
                {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi",
                 "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"}),
       "_", TARGET_SECTION_SYNTAX::MACH_O, "#",
       "clang -target x86_64-apple-macos -c",
       "as",
       /*assembly_supported=*/true, /*integrated_object_supported=*/false,
       /*codegen_supported=*/true},
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

std::string Normalize_target_name(std::string s) {
  size_t start = 0;
  while (start < s.size() && isspace(static_cast<unsigned char>(s[start])))
    ++start;
  size_t end = s.size();
  while (end > start && isspace(static_cast<unsigned char>(s[end - 1])))
    --end;
  s = s.substr(start, end - start);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return static_cast<char>(tolower(c)); });
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '.') s[i] = '-';
  }
  return s;
}

const char *Arch_name(TARGET_ARCH arch) {
  switch (arch) {
    case TARGET_ARCH::ARMV7:  return "armv7";
    case TARGET_ARCH::AARCH64: return "aarch64";
    case TARGET_ARCH::I386:   return "i386";
    case TARGET_ARCH::X86_64: return "x86_64";
  }
  return "unknown";
}

} // namespace

const TARGET_INFO &Default_target() { return Targets().front(); }

const std::vector<TARGET_INFO> &Supported_targets() { return Targets(); }

std::string Native_target_name(HOST_OS os, HOST_ARCH arch) {
  if (os == HOST_OS::DARWIN) {
    if (arch == HOST_ARCH::AARCH64) return "arm64-apple-darwin";
    if (arch == HOST_ARCH::X86_64) return "x86_64-apple-darwin";
  } else if (os == HOST_OS::LINUX) {
    if (arch == HOST_ARCH::ARMV7) return "armv7-linux-gnueabihf";
    if (arch == HOST_ARCH::AARCH64) return "aarch64-linux-gnu";
    if (arch == HOST_ARCH::I386) return "i386-linux-gnu";
    if (arch == HOST_ARCH::X86_64) return "x86_64-linux-gnu";
  }
  return std::string();
}

bool Detect_native_target(std::string *triple, std::string *error) {
  HOST_OS os = HOST_OS::UNSUPPORTED;
#if defined(__APPLE__)
  os = HOST_OS::DARWIN;
#elif defined(__linux__)
  os = HOST_OS::LINUX;
#endif

  HOST_ARCH arch = HOST_ARCH::UNSUPPORTED;
  const char *arch_error = "unsupported native host architecture";
#if defined(__aarch64__)
#if defined(__ILP32__)
  arch_error = "unsupported native AArch64 ILP32 ABI";
#elif defined(__AARCH64EB__)
  arch_error = "unsupported native big-endian AArch64 host";
#else
  arch = HOST_ARCH::AARCH64;
#endif
#elif defined(__arm__)
#if defined(__ARMEB__)
  arch_error = "unsupported native big-endian ARM host";
#elif !defined(__ARM_ARCH) || __ARM_ARCH < 7
  arch_error = "unsupported native ARM architecture below ARMv7";
#elif !defined(__ARM_PCS_VFP)
  arch_error = "unsupported native ARM ABI without hard-float calling convention";
#else
  arch = HOST_ARCH::ARMV7;
#endif
#elif defined(__x86_64__)
#if defined(__ILP32__)
  arch_error = "unsupported native x86_64 ILP32 ABI";
#else
  arch = HOST_ARCH::X86_64;
#endif
#elif defined(__i386__)
  arch = HOST_ARCH::I386;
#endif

  if (triple != nullptr) triple->clear();
  if (os == HOST_OS::UNSUPPORTED) {
    if (error != nullptr) *error = "unsupported native host operating system";
    return false;
  }
  if (arch == HOST_ARCH::UNSUPPORTED) {
    if (error != nullptr) *error = arch_error;
    return false;
  }

  const std::string native = Native_target_name(os, arch);
  if (native.empty()) {
    if (error != nullptr) *error = "unsupported native host OS/architecture combination";
    return false;
  }
  if (triple != nullptr) *triple = native;
  if (error != nullptr) error->clear();
  return true;
}

bool Resolve_configured_target(const std::string &name,
                               const TARGET_INFO **target,
                               std::string *error) {
  std::string resolved_name = name;
  if (Normalize_target_name(name) == "native" &&
      !Detect_native_target(&resolved_name, error)) {
    if (target != nullptr) *target = nullptr;
    return false;
  }
  return Resolve_target(resolved_name, target, error);
}

bool Resolve_target(const std::string &name, const TARGET_INFO **target,
                    std::string *error) {
  if (target != nullptr) {
    *target = nullptr;
  }
  std::string canonical = name.empty() ? Default_target().triple
                                       : Normalize_target_name(name);
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

const char *Target_arch_name(TARGET_ARCH arch) {
  return Arch_name(arch);
}

bool Parse_target_arch(const std::string &text, TARGET_ARCH *arch) {
  const TARGET_INFO *target = nullptr;
  std::string err;
  if (!Resolve_target(text, &target, &err)) {
    return false;
  }
  if (arch != nullptr) {
    *arch = target->arch;
  }
  return true;
}

std::string Supported_target_arches() {
  std::ostringstream out;
  bool first = true;
  for (const TARGET_INFO &t : Targets()) {
    if (!first) out << ", ";
    out << t.triple;
    first = false;
  }
  return out.str();
}
