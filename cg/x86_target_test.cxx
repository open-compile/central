#include "x86_target.h"
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
  auto backend = Create_target_backend(*info, &error);
  assert(backend != nullptr);
  return backend;
}

int main() {
  auto i386 = Backend("x86-linux");
  auto x64_linux = Backend("x64-linux");
  auto x64_apple = Backend("x64-macos");

  assert(i386->Integer_register_name(0) == "%eax");
  assert(std::string(i386->Pointer_directive()) == ".long");
  assert(i386->Dedicated_register_name(TARGET_REGISTER_ROLE::STACK_POINTER) == "%esp");
  assert(i386->Dedicated_register_name(TARGET_REGISTER_ROLE::RETURN_VALUE) == "%eax");

  assert(x64_linux->Integer_register_name(0) == "%rdi");
  assert(x64_linux->Integer_value_register_name(0, 4) == "%edi");
  assert(x64_linux->Integer_value_register_name(1, 8) == "%rsi");
  assert(std::string(x64_linux->Pointer_directive()) == ".quad");
  assert(x64_linux->Spell_global_symbol("main") == "main");
  assert(x64_apple->Spell_global_symbol("main") == "_main");

  FILE *file = tmpfile();
  assert(file != nullptr);
  x64_linux->Emit_code_prologue(file);
  x64_linux->Emit_function_header(file, "main");
  x64_linux->Emit_function_prologue(file, 32);
  x64_linux->Emit_function_epilogue(file);
  const std::string linux_text = Read_file(file);
  fclose(file);
  assert(linux_text.find(".text") != std::string::npos);
  assert(linux_text.find(".type main, @function") != std::string::npos);
  assert(linux_text.find("pushq\t%rbp") != std::string::npos);
  assert(linux_text.find("subq\t$32, %rsp") != std::string::npos);
  assert(linux_text.find("ret") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  x64_apple->Emit_code_prologue(file);
  x64_apple->Emit_function_header(file, "main");
  const std::string apple_text = Read_file(file);
  fclose(file);
  assert(apple_text.find("__TEXT,__text") != std::string::npos);
  assert(apple_text.find(".globl _main") != std::string::npos);

  file = tmpfile();
  assert(file != nullptr);
  CGOP nop(CGOPC_NOP, 0, 0, 0, 0, 0);
  i386->Emit_op(&nop, file);
  assert(Read_file(file) == "\tnop\n");
  fclose(file);
  return 0;
}
