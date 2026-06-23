#include "x86_target.h"

#include "cgir.h"
#include "tn.h"

#include <cstdio>

namespace {

TN *Operand(CGOP *op, unsigned position) {
  const CG_OPRAND operand = op->getResOpnd()[position];
  AssertThat(operand != 0, ("CGOP operand must not be empty"));
  return TN_tn(operand);
}

std::string Label_name_for(const X86_TARGET &target, TN *tn, bool external) {
  AssertThat(TN_is_label(tn), ("expected label operand"));
  const std::string name = LABEL_name(TN_label(tn));
  return external ? target.Spell_global_symbol(name) : name;
}

std::string Register_for(const X86_TARGET &target, TN *tn, unsigned size = 0) {
  if (TN_is_dedicated(tn)) {
    const UINT32 reg = TN_register(tn);
    if (TN_register_class(tn) == REGISTER_CLASS_sp && reg == REGISTER_sp)
      return target.Dedicated_register_name(TARGET_REGISTER_ROLE::STACK_POINTER);
    if (TN_register_class(tn) == REGISTER_CLASS_fp && reg == REGISTER_fp)
      return target.Dedicated_register_name(TARGET_REGISTER_ROLE::FRAME_POINTER);
    if (TN_register_class(tn) == REGISTER_CLASS_ra && reg == REGISTER_ra)
      return target.Dedicated_register_name(TARGET_REGISTER_ROLE::LINK_REGISTER);
    if (TN_register_class(tn) == REGISTER_CLASS_v0 && reg == REGISTER_v0) {
      if (size != 0 && size <= 4) return "%eax";
      return target.Dedicated_register_name(TARGET_REGISTER_ROLE::RETURN_VALUE);
    }
  }
  return target.Integer_value_register_name(TN_register(tn),
                                             size == 0 ? TN_size(tn) : size);
}

std::string Value_for(const X86_TARGET &target, TN *tn, unsigned size = 0) {
  if (TN_is_constant(tn)) return "$" + std::to_string(TN_value(tn));
  if (TN_is_label(tn)) return Label_name_for(target, tn, false);
  return Register_for(target, tn, size);
}

const char *Branch_name(CGOPC opcode) {
  switch (opcode) {
    case CGOPC_B: return "jmp";
    case CGOPC_BEQ: return "je";
    case CGOPC_BNE: return "jne";
    case CGOPC_BGE: return "jge";
    case CGOPC_BLT: return "jl";
    case CGOPC_BGT: return "jg";
    case CGOPC_BLE: return "jle";
    default: return nullptr;
  }
}

} // namespace

bool X86_TARGET::Is_64_bit() const { return _info.arch == TARGET_ARCH::X86_64; }
bool X86_TARGET::Is_apple() const { return _info.os == TARGET_OS::DARWIN; }

void X86_TARGET::Emit_code_prologue(FILE *out) const {
  if (Is_apple()) fprintf(out, ".section __TEXT,__text,regular,pure_instructions\n\n");
  else fprintf(out, ".text\n\n");
}

void X86_TARGET::Emit_code_epilogue(FILE *out) const { (void)out; }

void X86_TARGET::Emit_data_prologue(FILE *out) const {
  if (Is_apple()) fprintf(out, ".section __DATA,__data\n\n");
  else fprintf(out, ".data\n\n");
}

void X86_TARGET::Emit_function_header(FILE *out, const std::string &name) const {
  const std::string symbol = Spell_global_symbol(name);
  fprintf(out, Is_apple() ? ".globl %s\n" : ".global %s\n", symbol.c_str());
  if (!Is_apple()) fprintf(out, ".type %s, @function\n", symbol.c_str());
  if (Is_apple()) fprintf(out, "\t.p2align\t4, 0x90\n");
  fprintf(out, "%s:\n", symbol.c_str());
}

void X86_TARGET::Emit_function_prologue(FILE *out, unsigned local_size) const {
  if (Is_64_bit()) {
    fprintf(out, "\tpushq\t%%rbp\n\tmovq\t%%rsp, %%rbp\n");
    if (local_size != 0) fprintf(out, "\tsubq\t$%u, %%rsp\n", local_size);
  } else {
    fprintf(out, "\tpushl\t%%ebp\n\tmovl\t%%esp, %%ebp\n");
    if (local_size != 0) fprintf(out, "\tsubl\t$%u, %%esp\n", local_size);
  }
}

void X86_TARGET::Emit_function_epilogue(FILE *out) const {
  fprintf(out, "\tleave\n\tret\n");
}

std::string X86_TARGET::Spell_global_symbol(const std::string &name) const {
  return _info.symbol_prefix + name;
}

std::string X86_TARGET::Integer_register_name(unsigned number) const {
  static const char *i386[] = {"%eax", "%ecx", "%edx", "%ebx", "%esi", "%edi", "%eax"};
  static const char *x64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10"};
  const unsigned index = number % 7;
  return Is_64_bit() ? x64[index] : i386[index];
}

std::string X86_TARGET::Integer_value_register_name(unsigned number,
                                                     unsigned size) const {
  if (!Is_64_bit() || size > 4) return Integer_register_name(number);
  static const char *x64_32[] = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d", "%r10d"};
  return x64_32[number % 7];
}

std::string X86_TARGET::Dedicated_register_name(TARGET_REGISTER_ROLE role) const {
  switch (role) {
    case TARGET_REGISTER_ROLE::STACK_POINTER: return Is_64_bit() ? "%rsp" : "%esp";
    case TARGET_REGISTER_ROLE::FRAME_POINTER: return Is_64_bit() ? "%rbp" : "%ebp";
    case TARGET_REGISTER_ROLE::LINK_REGISTER: return "";
    case TARGET_REGISTER_ROLE::RETURN_VALUE: return Is_64_bit() ? "%rax" : "%eax";
  }
  return "";
}

std::string X86_TARGET::Instruction_name(unsigned logical_opcode,
                                          const char *current_name) const {
  (void)logical_opcode;
  return current_name;
}

const char *X86_TARGET::Pointer_directive() const {
  return Is_64_bit() ? ".quad" : ".long";
}

void X86_TARGET::Emit_op(CGOP *op, FILE *out) const {
  const CGOPC opcode = op->getOpcode();
  const char value_suffix = 'l';
  const char pointer_suffix = Is_64_bit() ? 'q' : 'l';
  if (opcode == CGOPC_NOP) { fprintf(out, "\tnop\n"); return; }
  if (opcode == CGOPC_RET || opcode == CGOPC_BX ||
      opcode == CGOPC_PUSHR || opcode == CGOPC_POPR) return;

  if (opcode == CGOPC_LIMM) {
    fprintf(out, "\tmov%c\t$%lld, %s\n", value_suffix,
            TN_value(Operand(op, 1)), Register_for(*this, Operand(op, 0), 4).c_str());
    return;
  }
  if (opcode == CGOPC_LADDR || opcode == CGOPC_LDRLBL) {
    TN *label_tn = Operand(op, 1);
    LABEL *label = LABEL_label(TN_label(label_tn));
    const ST_IDX symbol_idx = label->Get_temp_sym();
    if (symbol_idx != 0) {
      const std::string symbol = Spell_global_symbol(ST_name(symbol_idx));
      fprintf(out, "\tlea%c\t%s%s, %s\n", pointer_suffix, symbol.c_str(),
              Is_64_bit() ? "(%rip)" : "",
              Register_for(*this, Operand(op, 0), _info.abi.pointer_size).c_str());
      return;
    }
    AssertThat(opcode == CGOPC_LDRLBL,
               ("LADDR relocation label has no symbol"));
    const std::string label_text = Label_name_for(*this, Operand(op, 1), false);
    fprintf(out, "\tmov%c\t%s%s, %s\n", pointer_suffix, label_text.c_str(),
            Is_64_bit() ? "(%rip)" : "",
            Register_for(*this, Operand(op, 0), _info.abi.pointer_size).c_str());
    return;
  }
  if (opcode == CGOPC_LDR || opcode == CGOPC_STR) {
    TN *value = Operand(op, 0);
    TN *base = Operand(op, 1);
    TN *offset = Operand(op, 2);
    const std::string memory = std::to_string(TN_value(offset)) + "(" +
        Register_for(*this, base, _info.abi.pointer_size) + ")";
    if (opcode == CGOPC_LDR)
      fprintf(out, "\tmovl\t%s, %s\n", memory.c_str(), Register_for(*this, value, 4).c_str());
    else
      fprintf(out, "\tmovl\t%s, %s\n", Register_for(*this, value, 4).c_str(), memory.c_str());
    return;
  }
  if (opcode == CGOPC_MOV) {
    fprintf(out, "\tmovl\t%s, %s\n", Value_for(*this, Operand(op, 1), 4).c_str(),
            Register_for(*this, Operand(op, 0), 4).c_str());
    return;
  }
  if (opcode == CGOPC_MOVT) {
    fprintf(out, "\torl\t$%lld, %s\n", TN_value(Operand(op, 1)) << 16,
            Register_for(*this, Operand(op, 0), 4).c_str());
    return;
  }
  if (opcode == CGOPC_ADJSP) {
    const INT64 amount = TN_value(Operand(op, 1));
    if (amount != 0)
      fprintf(out, "\t%s%c\t$%lld, %s\n", amount < 0 ? "sub" : "add",
              pointer_suffix, amount < 0 ? -amount : amount,
              Dedicated_register_name(TARGET_REGISTER_ROLE::STACK_POINTER).c_str());
    return;
  }
  if (opcode == CGOPC_BL) {
    fprintf(out, "\tcall\t%s\n", Label_name_for(*this, Operand(op, 1), true).c_str());
    return;
  }
  const char *branch = Branch_name(opcode);
  if (branch != nullptr) {
    fprintf(out, "\t%s\t%s\n", branch, Label_name_for(*this, Operand(op, 1), false).c_str());
    return;
  }
  if (opcode == CGOPC_CMP) {
    fprintf(out, "\tcmpl\t%s, %s\n", Value_for(*this, Operand(op, 2), 4).c_str(),
            Value_for(*this, Operand(op, 1), 4).c_str());
    return;
  }
  if (opcode == CGOPC_DIV) {
    fprintf(out, "\tmovl\t%s, %%eax\n\tcltd\n\tidivl\t%s\n\tmovl\t%%eax, %s\n",
            Value_for(*this, Operand(op, 1), 4).c_str(),
            Value_for(*this, Operand(op, 2), 4).c_str(),
            Register_for(*this, Operand(op, 0), 4).c_str());
    return;
  }
  if (opcode == CGOPC_MVN) {
    const std::string dst = Register_for(*this, Operand(op, 0), 4);
    fprintf(out, "\tmovl\t%s, %s\n\tnotl\t%s\n",
            Value_for(*this, Operand(op, 1), 4).c_str(), dst.c_str(), dst.c_str());
    return;
  }

  const char *instruction = nullptr;
  switch (opcode) {
    case CGOPC_ADD: instruction = "addl"; break;
    case CGOPC_SUBS: instruction = "subl"; break;
    case CGOPC_MUL: instruction = "imull"; break;
    case CGOPC_AND: instruction = "andl"; break;
    case CGOPC_ORR: instruction = "orl"; break;
    default: break;
  }
  AssertThat(instruction != nullptr,
             ("unsupported x86 CGOP %s", Get_cg_opc_info(opcode)->getName()));
  const std::string dst = Register_for(*this, Operand(op, 0), 4);
  const std::string lhs = Value_for(*this, Operand(op, 1), 4);
  const std::string rhs = Value_for(*this, Operand(op, 2), 4);
  if (dst != lhs) fprintf(out, "\tmovl\t%s, %s\n", lhs.c_str(), dst.c_str());
  fprintf(out, "\t%s\t%s, %s\n", instruction, rhs.c_str(), dst.c_str());
}
