#include "arm64_target.h"
#include "cgir.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

static std::string Read_file(FILE *file) {
  fflush(file);
  fseek(file, 0, SEEK_END);
  const long size = ftell(file);
  rewind(file);
  std::string text(static_cast<std::size_t>(size), '\0');
  if (size > 0) fread(&text[0], 1, text.size(), file);
  return text;
}

static std::unique_ptr<TARGET_BACKEND> Backend(const char *name) {
  const TARGET_INFO *info = nullptr;
  std::string error;
  assert(Resolve_target(name, &info, &error));
  std::unique_ptr<TARGET_BACKEND> backend = Create_target_backend(*info, &error);
  assert(backend != nullptr);
  return backend;
}

int main() {
  assert(std::string(Get_cg_opc_info(CGOPC_LIMM)->getName()) == "CGOPC_LIMM");
  assert(std::string(Get_cg_opc_info(CGOPC_LADDR)->getName()) == "CGOPC_LADDR");
  assert(std::string(Get_cg_opc_info(CGOPC_RET)->getName()) == "CGOPC_RET");
  assert(std::string(Get_cg_opc_info(CGOPC_ADJSP)->getName()) == "CGOPC_ADJSP");
  // NOTE: GCC predefines `linux=1`, so using `linux` (or short forms like
  // `apple`) as a variable name would expand `auto linux = ...` into
  // `auto 1 = ...` and break compilation. Use longer, unambiguous names.
  auto aarch64_linux_backend = Backend("aarch64-linux");
  auto arm64_darwin_backend = Backend("arm64-macos");

  assert(aarch64_linux_backend->Spell_global_symbol("main") == "main");
  assert(arm64_darwin_backend->Spell_global_symbol("main") == "_main");
  assert(aarch64_linux_backend->Integer_register_name(3) == "x3");
  assert(aarch64_linux_backend->Integer_value_register_name(3, 4) == "w3");
  assert(aarch64_linux_backend->Integer_value_register_name(3, 8) == "x3");
  assert(std::string(aarch64_linux_backend->Pointer_directive()) == ".xword");
  assert(std::string(arm64_darwin_backend->Pointer_directive()) == ".quad");

  FILE *file = tmpfile();
  assert(file != nullptr);
  aarch64_linux_backend->Emit_code_prologue(file);
  aarch64_linux_backend->Emit_function_header(file, "main");
  aarch64_linux_backend->Emit_function_prologue(file, 32);
  aarch64_linux_backend->Emit_function_epilogue(file);
  const std::string aarch64_linux_assembly = Read_file(file);
  fclose(file);
  assert(aarch64_linux_assembly.find(".text") != std::string::npos);
  assert(aarch64_linux_assembly.find(".global main") != std::string::npos);
  assert(aarch64_linux_assembly.find("stp\tx29, x30") != std::string::npos);
  assert(aarch64_linux_assembly.find("sub\tsp, sp, #32") != std::string::npos);
  assert(aarch64_linux_assembly.find("ret") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  aarch64_linux_backend->Emit_function_prologue(file, 0x12345);
  const std::string large_frame_assembly = Read_file(file);
  fclose(file);
  assert(large_frame_assembly.find("movz\tx16, #9029") != std::string::npos);
  assert(large_frame_assembly.find("movk\tx16, #1, lsl #16") != std::string::npos);
  assert(large_frame_assembly.find("sub\tsp, sp, x16") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  arm64_darwin_backend->Emit_code_prologue(file);
  arm64_darwin_backend->Emit_function_header(file, "main");
  const std::string arm64_darwin_assembly = Read_file(file);
  fclose(file);
  assert(arm64_darwin_assembly.find("__TEXT,__text") != std::string::npos);
  assert(arm64_darwin_assembly.find(".globl _main") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  CGOP nop(CGOPC_NOP, 0, 0, 0, 0, 0);
  aarch64_linux_backend->Emit_op(&nop, file);
  const std::string nop_assembly = Read_file(file);
  fclose(file);
  assert(nop_assembly == "\tnop\n");
  return 0;
}
