#include "armv7_linux_target.h"
#include "cgir.h"

#include <cassert>
#include <cstdio>
#include <string>

static std::string Capture(void (TARGET_BACKEND::*emit)(FILE *) const,
                           const TARGET_BACKEND &backend) {
  FILE *file = tmpfile();
  assert(file != nullptr);
  (backend.*emit)(file);
  fflush(file);
  fseek(file, 0, SEEK_END);
  const long size = ftell(file);
  rewind(file);
  std::string result(static_cast<std::size_t>(size), '\0');
  if (size != 0) fread(&result[0], 1, result.size(), file);
  fclose(file);
  return result;
}

static std::string Capture_frame(bool prologue, unsigned local_size,
                                 const TARGET_BACKEND &backend) {
  FILE *file = tmpfile();
  assert(file != nullptr);
  if (prologue) backend.Emit_function_prologue(file, local_size);
  else backend.Emit_function_epilogue(file);
  fflush(file);
  fseek(file, 0, SEEK_END);
  const long size = ftell(file);
  rewind(file);
  std::string result(static_cast<std::size_t>(size), '\0');
  if (size != 0) fread(&result[0], 1, result.size(), file);
  fclose(file);
  return result;
}

int main() {
  const TARGET_INFO *info = nullptr;
  std::string error;
  assert(Resolve_target("armv7", &info, &error));
  ARMV7_LINUX_TARGET backend(*info);

  assert(Capture(&TARGET_BACKEND::Emit_code_prologue, backend) ==
         ".text\n\n.global __aeabi_idiv \n");
  assert(Capture(&TARGET_BACKEND::Emit_code_epilogue, backend).empty());
  assert(Capture(&TARGET_BACKEND::Emit_data_prologue, backend) == ".data\n\n");
  assert(backend.Spell_global_symbol("main") == "main");
  assert(backend.Integer_register_name(7) == "r7");
  assert(backend.Dedicated_register_name(TARGET_REGISTER_ROLE::STACK_POINTER) == "sp");
  assert(backend.Dedicated_register_name(TARGET_REGISTER_ROLE::FRAME_POINTER) == "fp");
  assert(backend.Dedicated_register_name(TARGET_REGISTER_ROLE::LINK_REGISTER) == "lr");
  assert(backend.Dedicated_register_name(TARGET_REGISTER_ROLE::RETURN_VALUE) == "r0");
  assert(std::string(backend.Immediate_prefix()) == "#");

  FILE *op_file = tmpfile();
  assert(op_file != nullptr);
  CGOP nop(CGOPC_NOP, 0, 0, 0, 0, 0);
  backend.Emit_op(&nop, op_file);
  fflush(op_file);
  rewind(op_file);
  char nop_text[32] = {};
  fread(nop_text, 1, sizeof(nop_text) - 1, op_file);
  fclose(op_file);
  assert(std::string(nop_text) == "\tnop\t\n");

  assert(Capture_frame(true, 16, backend) ==
         "\tpush\t{fp, lr}\n\tpush\t{r4-r10}\n\tadd\tfp, sp, #32\n\tsub\tsp, sp, #16\n");
  assert(Capture_frame(false, 0, backend) ==
         "\tsub\tsp, fp, #32\n\tpop\t{r4-r10}\n\tpop\t{fp, pc}\n");
  return 0;
}
