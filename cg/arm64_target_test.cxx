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
  auto linux = Backend("aarch64-linux");
  auto apple = Backend("arm64-macos");

  assert(linux->Spell_global_symbol("main") == "main");
  assert(apple->Spell_global_symbol("main") == "_main");
  assert(linux->Integer_register_name(3) == "x3");
  assert(linux->Integer_value_register_name(3, 4) == "w3");
  assert(linux->Integer_value_register_name(3, 8) == "x3");
  assert(std::string(linux->Pointer_directive()) == ".xword");
  assert(std::string(apple->Pointer_directive()) == ".quad");

  FILE *file = tmpfile();
  assert(file != nullptr);
  linux->Emit_code_prologue(file);
  linux->Emit_function_header(file, "main");
  linux->Emit_function_prologue(file, 32);
  linux->Emit_function_epilogue(file);
  const std::string linux_text = Read_file(file);
  fclose(file);
  assert(linux_text.find(".text") != std::string::npos);
  assert(linux_text.find(".global main") != std::string::npos);
  assert(linux_text.find("stp\tx29, x30") != std::string::npos);
  assert(linux_text.find("sub\tsp, sp, #32") != std::string::npos);
  assert(linux_text.find("ret") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  linux->Emit_function_prologue(file, 0x12345);
  const std::string large_frame_text = Read_file(file);
  fclose(file);
  assert(large_frame_text.find("movz\tx16, #9029") != std::string::npos);
  assert(large_frame_text.find("movk\tx16, #1, lsl #16") != std::string::npos);
  assert(large_frame_text.find("sub\tsp, sp, x16") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  apple->Emit_code_prologue(file);
  apple->Emit_function_header(file, "main");
  const std::string apple_text = Read_file(file);
  fclose(file);
  assert(apple_text.find("__TEXT,__text") != std::string::npos);
  assert(apple_text.find(".globl _main") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  CGOP nop(CGOPC_NOP, 0, 0, 0, 0, 0);
  linux->Emit_op(&nop, file);
  const std::string nop_text = Read_file(file);
  fclose(file);
  assert(nop_text == "\tnop\n");
  return 0;
}
