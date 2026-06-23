#ifndef OCC_TARGET_BACKEND_H
#define OCC_TARGET_BACKEND_H

#include "target.h"

#include <cstdio>
#include <memory>
#include <string>

enum class TARGET_REGISTER_ROLE {
  STACK_POINTER,
  FRAME_POINTER,
  LINK_REGISTER,
  RETURN_VALUE,
};

class CGOP;

class TARGET_BACKEND {
public:
  virtual ~TARGET_BACKEND() = default;

  virtual const TARGET_INFO &Info() const = 0;
  virtual void Emit_code_prologue(FILE *out) const = 0;
  virtual void Emit_code_epilogue(FILE *out) const = 0;
  virtual void Emit_data_prologue(FILE *out) const = 0;
  virtual void Emit_function_header(FILE *out, const std::string &name) const = 0;
  virtual void Emit_function_prologue(FILE *out, unsigned local_size) const = 0;
  virtual void Emit_function_epilogue(FILE *out) const = 0;
  virtual std::string Spell_global_symbol(const std::string &name) const = 0;
  virtual std::string Integer_register_name(unsigned number) const = 0;
  virtual std::string Integer_value_register_name(unsigned number,
                                                  unsigned size) const = 0;
  virtual std::string Dedicated_register_name(TARGET_REGISTER_ROLE role) const = 0;
  virtual const char *Immediate_prefix() const = 0;
  virtual std::string Instruction_name(unsigned logical_opcode,
                                       const char *current_name) const = 0;
  virtual void Emit_op(CGOP *op, FILE *out) const = 0;
  virtual const char *Pointer_directive() const = 0;
  virtual bool Uses_literal_address_pool() const = 0;
};

std::unique_ptr<TARGET_BACKEND> Create_target_backend(
    const TARGET_INFO &info, std::string *error);

#endif // OCC_TARGET_BACKEND_H
