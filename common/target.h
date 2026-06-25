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

// HOST 描述符：进程运行的 OS + 架构 + ABI 变体 token。
// 与 TARGET 完全独立 —— 你可以为任意 target 编译，与 host 无关。
struct HOST_DESC {
  HOST_OS os;
  HOST_ARCH arch;
  // ABI 变体 token，由 SIMULATE_HOST_AS_PLATFORM 解析得到。
  //   ""      — 默认 ABI（64 位 arch 上 LP64，32 位 arch 上 ILP32）
  //   "ilp32" — 64 位 arch 上的 ILP32 变体（如 x86_64-x32、aarch64-ilp32）
  // 多个 token 用 '+' 串接，例如 "ilp32+soft-float"。
  std::string variant;
};

// 把 "x86_64-linux+ilp32" / "armv7-linux" / "arm64-darwin" 之类解析成 HOST_DESC。
// 解析失败返回 {UNSUPPORTED, UNSUPPORTED, ""}。
HOST_DESC Parse_host_desc(const std::string &platform_name);

// 当前进程的 HOST。这是整个代码库里**唯一**读编译器 host 宏的地方。
// 当且仅当编译时定义了 SIMULATE_HOST_AS_PLATFORM 时走模拟路径；否则读
// __APPLE__/__linux__/__aarch64__/__arm__/__x86_64__/__i386__/__ILP32__/__AARCH64EB__/
// __ARMEB__/__ARM_ARCH/__ARM_PCS_VFP 等。
HOST_DESC Current_host();

// 纯函数：HOST_DESC → target triple。无副作用、无宏读取。
// *error 含 "ILP32" 当且仅当 host.variant 包含 "ilp32" token。
bool Resolve_native_target(const HOST_DESC &host,
                           std::string *triple, std::string *error);

// 向后兼容的薄壳：内部 = Resolve_native_target(Current_host(), ...)。
bool Detect_native_target(std::string *triple, std::string *error);

bool Resolve_configured_target(const std::string &name,
                               const TARGET_INFO **target,
                               std::string *error);

bool Resolve_target(const std::string &name, const TARGET_INFO **target,
                    std::string *error);

const char *Target_arch_name(TARGET_ARCH arch);
bool Parse_target_arch(const std::string &text, TARGET_ARCH *arch);
std::string Supported_target_arches();

// Implemented in cg/target_configure.cxx — requires full COMPILER_CONFIG definition.
bool Configure_target(const std::string &name, COMPILER_CONFIG *config,
                      std::string *error);

#endif // _OCC_TARGET_H_
