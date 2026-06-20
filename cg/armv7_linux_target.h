#ifndef OCC_ARMV7_LINUX_TARGET_H
#define OCC_ARMV7_LINUX_TARGET_H

#include "target_backend.h"

class ARMV7_LINUX_TARGET final : public TARGET_BACKEND {
private:
  const TARGET_INFO &_info;

public:
  explicit ARMV7_LINUX_TARGET(const TARGET_INFO &info) : _info(info) {}

  const TARGET_INFO &Info() const override { return _info; }
  void Emit_code_prologue(FILE *out) const override;
  void Emit_code_epilogue(FILE *out) const override;
  void Emit_data_prologue(FILE *out) const override;
  void Emit_function_header(FILE *out, const std::string &name) const override;
  void Emit_function_prologue(FILE *out, unsigned local_size) const override;
  void Emit_function_epilogue(FILE *out) const override;
  std::string Spell_global_symbol(const std::string &name) const override;
  std::string Integer_register_name(unsigned number) const override;
  std::string Integer_value_register_name(unsigned number,
                                          unsigned size) const override {
    (void)size;
    return Integer_register_name(number);
  }
  std::string Dedicated_register_name(TARGET_REGISTER_ROLE role) const override;
  const char *Immediate_prefix() const override { return "#"; }
  std::string Instruction_name(unsigned logical_opcode,
                               const char *current_name) const override;
  void Emit_op(CGOP *op, FILE *out) const override;
  const char *Pointer_directive() const override { return ".word"; }
};

#endif // OCC_ARMV7_LINUX_TARGET_H
