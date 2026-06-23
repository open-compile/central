#include "arm64_target.h"

#include "cgir.h"
#include "tn.h"

#include <cstdio>

namespace {

bool Is_apple(const TARGET_INFO &info) { return info.os == TARGET_OS::DARWIN; }

void Emit_u64_immediate(FILE *out, const char *reg, UINT64 value) {
  fprintf(out, "\tmovz\t%s, #%llu\n", reg, value & 0xFFFF);
  for (unsigned shift = 16; shift < 64; shift += 16) {
    const UINT64 part = (value >> shift) & 0xFFFF;
    if (part != 0)
      fprintf(out, "\tmovk\t%s, #%llu, lsl #%u\n", reg, part, shift);
  }
}

void Emit_sp_adjust(FILE *out, INT64 amount) {
  if (amount == 0) return;
  const UINT64 magnitude = static_cast<UINT64>(amount < 0 ? -amount : amount);
  const char *instruction = amount < 0 ? "sub" : "add";
  if (magnitude <= 4095) {
    fprintf(out, "\t%s\tsp, sp, #%llu\n", instruction, magnitude);
  } else {
    Emit_u64_immediate(out, "x16", magnitude);
    fprintf(out, "\t%s\tsp, sp, x16\n", instruction);
  }
}

TN *Operand(CGOP *op, unsigned position) {
  const CG_OPRAND operand = op->getResOpnd()[position];
  AssertThat(operand != 0, ("CGOP operand must not be empty"));
  return TN_tn(operand);
}

std::string Label_operand(const ARM64_TARGET &target, TN *tn, bool external) {
  AssertThat(TN_is_label(tn), ("expected label operand"));
  const std::string name = LABEL_name(TN_label(tn));
  return external ? target.Spell_global_symbol(name) : name;
}

std::string Value_register(const ARM64_TARGET &target, TN *tn,
                           bool address = false) {
  if (TN_is_dedicated(tn)) {
    const UINT32 reg = TN_register(tn);
    if (TN_register_class(tn) == REGISTER_CLASS_sp && reg == REGISTER_sp)
      return "sp";
    if (TN_register_class(tn) == REGISTER_CLASS_fp && reg == REGISTER_fp)
      return "x29";
    if (TN_register_class(tn) == REGISTER_CLASS_ra && reg == REGISTER_ra)
      return "x30";
    if (TN_register_class(tn) == REGISTER_CLASS_v0 && reg == REGISTER_v0)
      return address || TN_size(tn) > 4 ? "x0" : "w0";
  }
  return target.Integer_value_register_name(TN_register(tn),
                                             address ? 8 : TN_size(tn));
}

std::string Value_operand(const ARM64_TARGET &target, TN *tn,
                          bool address = false) {
  if (TN_is_constant(tn))
    return std::string(target.Immediate_prefix()) + std::to_string(TN_value(tn));
  if (TN_is_label(tn)) return Label_operand(target, tn, false);
  return Value_register(target, tn, address);
}

} // namespace

void ARM64_TARGET::Emit_code_prologue(FILE *out) const {
  if (Is_apple(_info))
    fprintf(out, ".section __TEXT,__text,regular,pure_instructions\n\n");
  else
    fprintf(out, ".text\n\n");
}

void ARM64_TARGET::Emit_code_epilogue(FILE *out) const { (void)out; }

void ARM64_TARGET::Emit_data_prologue(FILE *out) const {
  if (Is_apple(_info)) fprintf(out, ".section __DATA,__data\n\n");
  else fprintf(out, ".data\n\n");
}

void ARM64_TARGET::Emit_function_header(FILE *out,
                                        const std::string &name) const {
  const std::string symbol = Spell_global_symbol(name);
  fprintf(out, Is_apple(_info) ? ".globl %s\n" : ".global %s\n", symbol.c_str());
  if (!Is_apple(_info)) fprintf(out, ".type %s, %%function\n", symbol.c_str());
  fprintf(out, "\t.p2align\t2\n");
  fprintf(out, "%s:\n", symbol.c_str());
}

void ARM64_TARGET::Emit_function_prologue(FILE *out, unsigned local_size) const {
  fprintf(out, "\tstp\tx29, x30, [sp, #-16]!\n\tmov\tx29, sp\n");
  Emit_sp_adjust(out, -static_cast<INT64>(local_size));
}

void ARM64_TARGET::Emit_function_epilogue(FILE *out) const {
  fprintf(out, "\tmov\tsp, x29\n\tldp\tx29, x30, [sp], #16\n\tret\n");
}

std::string ARM64_TARGET::Spell_global_symbol(const std::string &name) const {
  return _info.symbol_prefix + name;
}

std::string ARM64_TARGET::Integer_register_name(unsigned number) const {
  return "x" + std::to_string(number);
}

std::string ARM64_TARGET::Integer_value_register_name(unsigned number,
                                                       unsigned size) const {
  return std::string(size <= 4 ? "w" : "x") + std::to_string(number);
}

std::string ARM64_TARGET::Dedicated_register_name(
    TARGET_REGISTER_ROLE role) const {
  switch (role) {
    case TARGET_REGISTER_ROLE::STACK_POINTER: return "sp";
    case TARGET_REGISTER_ROLE::FRAME_POINTER: return "x29";
    case TARGET_REGISTER_ROLE::LINK_REGISTER: return "x30";
    case TARGET_REGISTER_ROLE::RETURN_VALUE: return "x0";
  }
  return "";
}

std::string ARM64_TARGET::Instruction_name(unsigned logical_opcode,
                                            const char *current_name) const {
  (void)logical_opcode;
  return current_name;
}

const char *ARM64_TARGET::Pointer_directive() const {
  return Is_apple(_info) ? ".quad" : ".xword";
}

void ARM64_TARGET::Emit_op(CGOP *op, FILE *out) const {
  const CGOPC opcode = op->getOpcode();
  if (opcode == CGOPC_PUSHR || opcode == CGOPC_POPR) return;
  if (opcode == CGOPC_NOP) { fprintf(out, "\tnop\n"); return; }
  if (opcode == CGOPC_BX) return;
  if (opcode == CGOPC_RET) return;

  if (opcode == CGOPC_LIMM) {
    TN *dst = Operand(op, 0);
    TN *value = Operand(op, 1);
    const UINT64 raw = static_cast<UINT64>(TN_value(value));
    const std::string rd = Value_register(*this, dst);
    fprintf(out, "\tmov\t%s, #%llu\n", rd.c_str(), raw & 0xFFFF);
    if (((raw >> 16) & 0xFFFF) != 0)
      fprintf(out, "\tmovk\t%s, #%llu, lsl #16\n", rd.c_str(),
              (raw >> 16) & 0xFFFF);
    return;
  }

  if (opcode == CGOPC_MOV || opcode == CGOPC_MOVT) {
    TN *dst = Operand(op, 0);
    TN *src = Operand(op, 1);
    const std::string rd = Value_register(*this, dst);
    if (opcode == CGOPC_MOVT)
      fprintf(out, "\tmovk\t%s, %s, lsl #16\n", rd.c_str(),
              Value_operand(*this, src).c_str());
    else
      fprintf(out, "\tmov\t%s, %s\n", rd.c_str(),
              Value_operand(*this, src).c_str());
    return;
  }

  if (opcode == CGOPC_LADDR || opcode == CGOPC_LDRLBL) {
    TN *label_tn = Operand(op, 1);
    LABEL *label = LABEL_label(TN_label(label_tn));
    const ST_IDX symbol_idx = label->Get_temp_sym();
    if (symbol_idx != 0) {
      const std::string symbol = Spell_global_symbol(ST_name(symbol_idx));
      const std::string dst = Value_register(*this, Operand(op, 0), true);
      if (Is_apple(_info)) {
        fprintf(out, "\tadrp\t%s, %s@PAGE\n", dst.c_str(), symbol.c_str());
        fprintf(out, "\tadd\t%s, %s, %s@PAGEOFF\n",
                dst.c_str(), dst.c_str(), symbol.c_str());
      } else {
        fprintf(out, "\tadrp\t%s, %s\n", dst.c_str(), symbol.c_str());
        fprintf(out, "\tadd\t%s, %s, :lo12:%s\n",
                dst.c_str(), dst.c_str(), symbol.c_str());
      }
      return;
    }
    AssertThat(opcode == CGOPC_LDRLBL,
               ("LADDR relocation label has no symbol"));
    fprintf(out, "\tldr\t%s, %s\n",
            Value_register(*this, Operand(op, 0), true).c_str(),
            Label_operand(*this, Operand(op, 1), false).c_str());
    return;
  }
  if (opcode == CGOPC_ADJSP) {
    const INT64 amount = TN_value(Operand(op, 1));
    Emit_sp_adjust(out, amount);
    return;
  }
  if (opcode == CGOPC_LDR || opcode == CGOPC_STR) {
    fprintf(out, "\t%s\t%s, [%s, %s]\n",
            opcode == CGOPC_LDR ? "ldr" : "str",
            Value_register(*this, Operand(op, 0)).c_str(),
            Value_register(*this, Operand(op, 1), true).c_str(),
            Value_operand(*this, Operand(op, 2)).c_str());
    return;
  }
  if (opcode == CGOPC_BL) {
    fprintf(out, "\tbl\t%s\n",
            Label_operand(*this, Operand(op, 1), true).c_str());
    return;
  }
  if (opcode == CGOPC_B || opcode == CGOPC_BEQ || opcode == CGOPC_BNE ||
      opcode == CGOPC_BGE || opcode == CGOPC_BLT || opcode == CGOPC_BGT ||
      opcode == CGOPC_BLE) {
    fprintf(out, "\t%s\t%s\n", Get_cg_opc_info(opcode)->ins_token,
            Label_operand(*this, Operand(op, 1), false).c_str());
    return;
  }
  if (opcode == CGOPC_CMP) {
    fprintf(out, "\tcmp\t%s, %s\n",
            Value_operand(*this, Operand(op, 1)).c_str(),
            Value_operand(*this, Operand(op, 2)).c_str());
    return;
  }

  if ((opcode == CGOPC_ADD || opcode == CGOPC_SUBS) &&
      TN_is_dedicated(Operand(op, 0)) &&
      TN_register_class(Operand(op, 0)) == REGISTER_CLASS_sp) {
    TN *amount = Operand(op, 2);
    if (TN_is_constant(amount) && TN_value(amount) == 0) return;
    fprintf(out, "\t%s\tsp, sp, %s\n",
            opcode == CGOPC_ADD ? "add" : "sub",
            Value_operand(*this, amount).c_str());
    return;
  }

  CGOPC_INFO *info = Get_cg_opc_info(opcode);
  fprintf(out, "\t%s", info->ins_token);
  if (info->n_res > 0)
    fprintf(out, "\t%s", Value_register(*this, Operand(op, 0)).c_str());
  for (unsigned i = 0; i < info->n_oprs; ++i) {
    fprintf(out, "%s%s", (info->n_res > 0 || i > 0) ? ", " : "\t",
            Value_operand(*this, Operand(op, info->n_res + i)).c_str());
  }
  fprintf(out, "\n");
}
