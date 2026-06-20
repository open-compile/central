#include "armv7_linux_target.h"
#include "cgir.h"
#include "tn.h"

#include <cstdio>

void ARMV7_LINUX_TARGET::Emit_code_prologue(FILE *out) const {
  fprintf(out, ".text\n\n");
  fprintf(out, ".global __aeabi_idiv \n");
}

void ARMV7_LINUX_TARGET::Emit_code_epilogue(FILE *out) const { (void)out; }

void ARMV7_LINUX_TARGET::Emit_data_prologue(FILE *out) const {
  fprintf(out, ".data\n\n");
}

void ARMV7_LINUX_TARGET::Emit_function_header(FILE *out,
                                               const std::string &name) const {
  const std::string symbol = Spell_global_symbol(name);
  fprintf(out, ".global %s\n", symbol.c_str());
  fprintf(out, "%s: \n", symbol.c_str());
}

void ARMV7_LINUX_TARGET::Emit_function_prologue(FILE *out,
                                                 unsigned local_size) const {
  const unsigned sp_extra = _info.abi.prologue_save_area - 4;
  if (local_size > (1u << 8)) {
    fprintf(out, "\tpush\t{fp, lr}\n\tpush\t{r4-r10}\n");
    fprintf(out, "\tmov\tr4, #%u\n", local_size & 0xFFFF);
    if (((local_size >> 16) & 0xFFFF) != 0) {
      fprintf(out, "\tmovt\tr4, #%u\n", (local_size >> 16) & 0xFFFF);
    }
    fprintf(out, "\tadd\tfp, sp, #%u\n\tsub\tsp, sp, r4\n", sp_extra);
  } else {
    fprintf(out,
            "\tpush\t{fp, lr}\n\tpush\t{r4-r10}\n"
            "\tadd\tfp, sp, #%u\n\tsub\tsp, sp, #%u\n",
            sp_extra, local_size);
  }
}

void ARMV7_LINUX_TARGET::Emit_function_epilogue(FILE *out) const {
  const unsigned sp_extra = _info.abi.prologue_save_area - 4;
  fprintf(out, "\tsub\tsp, fp, #%u\n", sp_extra);
  fprintf(out, "\tpop\t{r4-r10}\n\tpop\t{fp, pc}\n");
}

std::string ARMV7_LINUX_TARGET::Spell_global_symbol(
    const std::string &name) const {
  return _info.symbol_prefix + name;
}

std::string ARMV7_LINUX_TARGET::Integer_register_name(unsigned number) const {
  return "r" + std::to_string(number);
}

std::string ARMV7_LINUX_TARGET::Dedicated_register_name(
    TARGET_REGISTER_ROLE role) const {
  switch (role) {
    case TARGET_REGISTER_ROLE::STACK_POINTER: return _info.abi.stack_pointer;
    case TARGET_REGISTER_ROLE::FRAME_POINTER: return _info.abi.frame_pointer;
    case TARGET_REGISTER_ROLE::LINK_REGISTER: return _info.abi.link_register;
    case TARGET_REGISTER_ROLE::RETURN_VALUE: return _info.abi.return_register;
  }
  return "";
}

std::string ARMV7_LINUX_TARGET::Instruction_name(
    unsigned logical_opcode, const char *current_name) const {
  (void)logical_opcode;
  return current_name;
}

namespace {

void Emit_armv7_operand(const ARMV7_LINUX_TARGET &target, CGOP *op,
                        UINT32 position, FILE *out) {
  const CG_OPRAND operand = op->getResOpnd()[position];
  AssertThat(operand != 0, ("CGOP operand must not be empty"));
  TN *tn = TN_tn(operand);
  if (TN_is_symbol(tn)) {
    fprintf(out, "%s ", LABEL_name(Get_addr_label(TN_var(tn))));
  } else if (TN_is_label(tn)) {
    fprintf(out, "%s ", LABEL_name(TN_label(tn)));
  } else if (TN_is_constant(tn)) {
    fprintf(out, "%s%lld ", target.Immediate_prefix(), TN_value(tn));
  } else if (TN_is_dedicated(tn)) {
    const UINT32 reg_id = TN_register(tn);
    if (TN_register_class(tn) == REGISTER_CLASS_ra && reg_id == REGISTER_ra) {
      fprintf(out, "%s ", target.Dedicated_register_name(
          TARGET_REGISTER_ROLE::LINK_REGISTER).c_str());
    } else if (TN_register_class(tn) == REGISTER_CLASS_sp &&
               reg_id == REGISTER_sp) {
      fprintf(out, "%s", target.Dedicated_register_name(
          TARGET_REGISTER_ROLE::STACK_POINTER).c_str());
    } else if (TN_register_class(tn) == REGISTER_CLASS_fp &&
               reg_id == REGISTER_fp) {
      fprintf(out, "%s", target.Dedicated_register_name(
          TARGET_REGISTER_ROLE::FRAME_POINTER).c_str());
    } else if (TN_register_class(tn) == REGISTER_CLASS_v0 &&
               reg_id == REGISTER_v0) {
      fprintf(out, "%s", target.Dedicated_register_name(
          TARGET_REGISTER_ROLE::RETURN_VALUE).c_str());
    } else {
      AssertThat(false, ("unsupported ARMv7 dedicated register"));
    }
  } else {
    fprintf(out, "%s ", target.Integer_register_name(TN_register(tn)).c_str());
  }
}

} // namespace

void ARMV7_LINUX_TARGET::Emit_op(CGOP *op, FILE *out) const {
  if (op->getOpcode() == CGOPC_LIMM) {
    TN *dst = TN_tn(op->getResOpnd()[0]);
    TN *value = TN_tn(op->getResOpnd()[1]);
    const UINT64 raw = static_cast<UINT64>(TN_value(value));
    fprintf(out, "\tmov\t%s , #%llu \n",
            Integer_register_name(TN_register(dst)).c_str(), raw & 0xFFFF);
    if (((raw >> 16) & 0xFFFF) != 0) {
      fprintf(out, "\tmovt\t%s , #%llu \n",
              Integer_register_name(TN_register(dst)).c_str(),
              (raw >> 16) & 0xFFFF);
    }
    return;
  }
  if (op->getOpcode() == CGOPC_LADDR) {
    fprintf(out, "\tldr\t");
    Emit_armv7_operand(*this, op, 0, out);
    fprintf(out, ", ");
    Emit_armv7_operand(*this, op, 1, out);
    fprintf(out, "\n");
    return;
  }
  if (op->getOpcode() == CGOPC_RET) {
    fprintf(out, "\tbx\tlr \n");
    return;
  }
  if (op->getOpcode() == CGOPC_ADJSP) {
    TN *amount = TN_tn(op->getResOpnd()[1]);
    const INT64 value = TN_value(amount);
    fprintf(out, "\t%s\tsp, sp, #%lld \n", value < 0 ? "subs" : "add",
            value < 0 ? -value : value);
    return;
  }
  CGOPC_INFO *info = Get_cg_opc_info(op->getOpcode());
  const std::string instruction = Instruction_name(op->getOpcode(), info->ins_token);
  fprintf(out, "\t%s\t", instruction.c_str());
  if (info->n_res >= 1) Emit_armv7_operand(*this, op, 0, out);
  if (info->n_oprs >= 1) {
    if (info->n_res >= 1) fprintf(out, ", ");
    if (CGOPC_is_ldst(op->getOpcode())) fprintf(out, "[");
    Emit_armv7_operand(*this, op, 1, out);
  }
  if (info->n_oprs >= 2) {
    fprintf(out, ", ");
    Emit_armv7_operand(*this, op, 2, out);
  }
  if (CGOPC_is_ldst(op->getOpcode())) fprintf(out, "]");
  fprintf(out, "\n");
}
