//
// Created by xc5 on 2020/6/14.
//

#include <iostream>
#include <fstream>
#include "ASTNodes.h"
#include "options.h"
#include "stdarg.h"
#include "ir.h"
#include "fe_main.h"

BIN_OP_TO_OPR FEOPCODE_INFO[6] = {
  { "+", TPLUS,  OPR_ADD },
  { "*", TMUL,   OPR_MPY },
  { "-", TMINUS, OPR_SUB },
  { "/", TDIV,   OPR_DIV },
  { "<", TCLT,   OPR_LT  },
  { ">", TCGT,   OPR_GT  },
};

INT32 femain(COMPILER_CONFIG &conf, FILE_MANAGER &file_man, const char *file_name) {
  extern FILE *yyin;
  if ((yyin = fopen(file_name, "r")) == NULL) {
    Comp_Failure("Failed to open source code : %s", file_name);
  }
  yyparse();

  if (!programBlock) {
    Comp_Failure("Syntax check failed for file : %s", file_name);
  }
  
  // std::cout << programBlock << std::endl;
  programBlock->print("--");
  auto root = programBlock->jsonGen();
  Irgen_visit(programBlock);

//    cout << root;

////    cout << root << endl;
//    CodeGenContext context;
////    createCoreFunctions(context);
//    context.generateCode(*programBlock);
//    ObjGen(context);

  string jsonFile = "visualization/A_tree.json";
  std::ofstream astJson(jsonFile);
  if( astJson.is_open() ){
    astJson << root;
    astJson.close();
    cout << "writing json to " << jsonFile << endl;
  }

  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO), (TFile, "Front end finishing, dump file info %d\n", (File()->Print(TFile), 1)));
  return 0;
}

/**
 * Visiting all global declarations
 * @param block
 */
void Irgen_visit(NBlock *block) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO), (TFile, "Visit Global Block\n"));
  auto child = block->child;
  for (auto it = child->begin(); it != child->end(); it++) {
    const string &name = (*it)->getTypeName();
    Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
             (TFile, "  Decl Kind : %s\n", name.c_str()));
    if (name == "NExpressionStatement") {
      // drop this;
      Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN),
               (TFile, "Skipping NExpressionStatement for now\n"));
      continue;
    } else if (name == "NVariableDeclaration") {
      // Add an assignment statement if rhs is not null
      shared_ptr<NVariableDeclaration> vardecl = reinterpret_cast<const shared_ptr<NVariableDeclaration> &>(*it);
      IR_ITER var_decl_iter = visitVarDecl(NULL, NULL, 1, vardecl);
    } else if (name == "NFunctionDeclaration") {
      visitFunction(reinterpret_cast<const shared_ptr<NFunctionDeclaration> &>(*it));
    } else {
      Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN), (TFile, "Skip other kind of global decl\n"));
    }
  }
}

void visitFunction(const shared_ptr<NFunctionDeclaration> &func) {
  STR_IDX func_name = File()->Save_string(func->id->name.c_str());
  TY_IDX ty_i4 = MTYPE_to_ty(MTYPE_I4);
  TY_IDX ty_v = MTYPE_to_ty(MTYPE_V);
  std::vector<TY_IDX> *param_ret_vec = new std::vector<TY_IDX>;
  param_ret_vec->push_back(ty_i4);
  param_ret_vec->push_back(ty_v);
  TY_IDX basic_func_ty = File()->Create_func_ty(func_name, 0, MTYPE_V,
                                                (TY_FLAG) 0, *param_ret_vec);
  ST_IDX func_sym = File()->Create_var(func_name, basic_func_ty,
                                       GLOBAL_SYMTAB, SYMC_EXTERN,
                                       SYME_PREEMPTIBLE,
                                       SYM_CLASS_FUNC);
  PU_INFO_IDX pu_info = File()->Create_function(func_sym, basic_func_ty);
  TREE *tree = PU_INFO_pu_info(pu_info)->entry;
  IR_ITER root_entry = tree->Get_root();
  IR_ITER block_iter = tree->Get_operand(root_entry, TREE_SEQ_BODY);
  visitBlock(tree, block_iter, 2, func->block);
  File()->Finish_creating_function(func_sym);
}

IR_ITER visitBlock(TREE *tree, IR_ITER parent_block, int level,
                   shared_ptr<NBlock> block) {
  for (auto & it : *block->child) {
    visitStatement(tree, parent_block, level, it);
  }
  return parent_block;
}

IR_ITER visitStatement(TREE *tree, IR_ITER parent, int level,
                       shared_ptr<NStatement> &stmt) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Stmt\n"));
  if (stmt->getTypeName() == "NVariableDeclaration") {
    visitVarDecl(tree, parent, level,
                 reinterpret_cast<const shared_ptr<NVariableDeclaration> &> (stmt));
  } else if (stmt->getTypeName() == "NExpressionStatement") {
    shared_ptr<NExpressionStatement> expr = reinterpret_cast<const shared_ptr<NExpressionStatement> &> (stmt);
    visitStatement(tree, parent, level, reinterpret_cast<shared_ptr<NStatement> &>(expr->expression));
  } else if (stmt->getTypeName() == "NAssignment") {
    visitAssignmentStmt(tree, parent, level,
                        reinterpret_cast<const shared_ptr<NAssignment> &> (stmt));
  } else if (stmt->getTypeName() == "NIfStatement") {
      visitIfStmt(tree, parent, level,
                          reinterpret_cast<const shared_ptr<NIfStatement> &> (stmt));
  } else if (stmt->getTypeName() == "NForStatement") {
      visitForStmt(tree, parent, level,
                          reinterpret_cast<const shared_ptr<NForStatement> &> (stmt));
  }
  else if (stmt->getTypeName() == "NReturnStatement") {
    visitReturnStmt(tree, parent, level,
                 reinterpret_cast<const shared_ptr<NReturnStatement> &> (stmt));
  }
  else {
    AssertThat(FALSE, ("not implemented kind of stmt = %s", stmt->getTypeName().c_str()));
  }
  return parent;
}

IR_ITER visitReturnStmt(TREE *tree, IR_ITER parent, int level,
                        const shared_ptr<NReturnStatement> &stmt) {
  shared_ptr<NExpression> return_val = stmt->expression;

  // 无返回值
  if (return_val == nullptr) {
    IR_ITER return_stmt;
    IRNODE_IDX return_node = tree->Create_node(OPC_RETURN);
    return_stmt = tree->Insert_stmt_to_block(parent, return_node);
    return parent;
  }

  IR_ITER return_stmt;
  IRNODE_IDX return_node = tree->Create_node(OPC_RETURN_VAL);
  return_stmt = tree->Insert_stmt_to_block(parent, return_node);

  // 处理返回值
  IR_ITER return_val_expr = visitExpression(tree, return_stmt, level, return_val);
  AssertThat(return_val_expr != parent && return_val_expr != return_stmt && return_val_expr != nullptr, ("Invalid expr conversion result"));
  tree->Set_operand(return_stmt, 0, return_val_expr);

  return parent;
}

IR_ITER visitIfStmt(TREE *tree, IR_ITER parent, int level,
                    const shared_ptr<NIfStatement> &stmt) {
    shared_ptr<NExpression> condition = stmt->condition;
    shared_ptr<NBlock> true_block = stmt->trueBlock;
    shared_ptr<NBlock> false_block = stmt->falseBlock;
    AssertThat(condition != nullptr, ("condition should not be null"));
    AssertThat(true_block != nullptr, ("trueBlock should not be null"));

    IR_ITER if_stmt;
    IRNODE_IDX if_node = tree->Create_node(OPC_IF);
    if_stmt = tree->Insert_stmt_to_block(parent, if_node);

    // 处理条件
    IR_ITER condition_expr = visitExpression(tree, if_stmt, level, condition);
    AssertThat(condition_expr != parent && condition_expr != if_stmt && condition_expr != nullptr, ("Invalid expr conversion result"));
    tree->Set_operand(if_stmt, 0, condition_expr);

    // 处理then块
    IR_ITER then_stmt;
    IRNODE_IDX then_node = tree->Create_node(OPC_BLOCK);
    then_stmt = tree->Set_operand(if_stmt, 1, then_node);
    visitBlock(tree, then_stmt, level, true_block);

    // 处理else块，有可能不存在
    if(false_block != nullptr) {
        IR_ITER else_stmt;
        IRNODE_IDX else_node = tree->Create_node(OPC_BLOCK);
        else_stmt = tree->Set_operand(if_stmt, 2, else_node);
        visitBlock(tree, else_stmt, level, false_block);
    }
    return parent;
}

IR_ITER visitForStmt(TREE *tree, IR_ITER parent, int level,
                    const shared_ptr<NForStatement> &stmt) {

    shared_ptr<NExpression> condition = stmt->condition;
    shared_ptr<NBlock> true_block = stmt->block;
    AssertThat(condition != nullptr, ("condition should not be null"));

    IR_ITER while_stmt;
    IRNODE_IDX while_node = tree->Create_node(OPC_WHILE_DO  );
    while_stmt = tree->Insert_stmt_to_block(parent, while_node);

    //处理循环判断条件
    IR_ITER condition_expr = visitExpression(tree, while_stmt, level, condition);
    AssertThat(condition_expr != parent && condition_expr != while_stmt && condition_expr != nullptr, ("Invalid expr conversion result"));
    tree->Set_operand(while_stmt, 0, condition_expr);

    //处理循环体
    IR_ITER do_stmt;
    IRNODE_IDX then_node = tree->Create_node(OPC_BLOCK);
    do_stmt = tree->Set_operand(while_stmt, 1, then_node);
    visitBlock(tree, do_stmt, level, true_block);


    return parent;
}

IR_ITER visitAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NAssignment> &stmt) {
  std::shared_ptr<NIdentifier> varname = stmt->lhs;
  shared_ptr<NExpression> rhs = stmt->rhs;

  AssertThat(varname != nullptr, ("Var name should not be null"));
  AssertThat(rhs != nullptr, ("Rhs should not be null"));

  ST_IDX sym_idx = File()->Find_symbol_by_name(varname->name.c_str());
  TY_IDX var_type = ST_ty(sym_idx);
  AssertThat(var_type == MTYPE_to_ty(MTYPE_I4),
             ("Previous defined symbol (%d) has a strange type = %d",
               sym_idx, ST_ty(sym_idx)));
  IR_ITER stid_stmt;

  // Determine whether LHS is a array ref or direct var
  if (TY_kind(var_type) == KIND_ARRAY) {
    // ...
    IRNODE_IDX assignment_node = tree->Create_node(OPC_I4I4ISTORE);
    tree->Get_node(assignment_node)->Set_load_offset(0);
    stid_stmt = tree->Insert_stmt_to_block(parent, assignment_node);
    AssertThat(FALSE, ("not implemented array assignment"));
  } else {
    // assignment is present
    IRNODE_IDX assignment_node = tree->Create_node(OPC_I4STID);
    tree->Get_node(assignment_node)->Set_symbol_idx(sym_idx);
    tree->Get_node(assignment_node)->Set_load_offset(0);
    stid_stmt = tree->Insert_stmt_to_block(parent, assignment_node);
  }
  IR_ITER rhs_expr = visitExpression(tree, stid_stmt, level, rhs);
  AssertThat(rhs_expr != parent && rhs_expr != stid_stmt && rhs_expr != nullptr, ("Invalid expr conversion result"));
  tree->Set_operand(stid_stmt, 0, rhs_expr);
  return parent;
}

IR_ITER visitExpression(TREE *tree, IR_ITER parent, int level,
                        shared_ptr<NExpression> expr) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Expression : %s \n", expr->getTypeName().c_str()));
  if (expr->getTypeName() == "NBinaryOperator") {
    shared_ptr<NBinaryOperator> bin_op = reinterpret_cast<const shared_ptr<NBinaryOperator> &>(expr);
    OPERATOR opr = Get_op_by_token((FEOPCODE) bin_op->op);
    AssertThat(opr != OPERATOR_UNKNOTREE && opr >= OPERATOR_FIRST, ("Operator not implementeed"));
    OPCODE opc = (OPCODE) (opr + RTYPE(MTYPE_I4) + DESC(MTYPE_I4));
    IRNODE_IDX opr_node = tree->Create_node(opc);
    IR_ITER cur_node = tree->Insert_temp_node(opr_node);
    IR_ITER lhs = visitExpression(tree, cur_node, level, bin_op->lhs);
    IR_ITER rhs = visitExpression(tree, cur_node, level, bin_op->rhs);
    tree->Set_operand(cur_node, 0, lhs); // lhs should be on the 0 operand.
    tree->Set_operand(cur_node, 1, rhs); // rhs should be on the 1 operand.
    return cur_node;
  } else if (expr->getTypeName() == "NDouble") {
    AssertThat(false, ("double not implemented."));
  } else if (expr->getTypeName() == "NInteger") {
    auto val = reinterpret_cast<shared_ptr<NInteger> &>(expr);
    IRNODE_IDX int_const_node = tree->Create_node(OPC_I4CONST);
    tree->Get_node(int_const_node)->Set_const_val(val->value);
    return tree->Insert_temp_node(int_const_node);
  } else if (expr->getTypeName() == "NIdentifier") {
    // use of variable.
    auto val = reinterpret_cast<shared_ptr<NIdentifier> &>(expr);
    AssertThat(!val->isArray, ("Array access not implemented."));
    IRNODE_IDX ldid_node = tree->Create_node(OPC_I4LDID);
    // Find symbol idx.
    ST_IDX sym = File()->Find_symbol_by_name(val->name.c_str());
    if (sym == 0) {
      Comp_Failure("Use of undeclared symbol : %s ", val->name.c_str());
    }
    tree->Get_node(ldid_node)->Set_symbol_idx(sym);
    IR_ITER cur_node = tree->Insert_temp_node(ldid_node);
    return cur_node;
  } else {
    AssertThat(false,
               ("Expression type not implemented : %s",
                expr->getTypeName().c_str()));
  }
  return parent;
}

OPERATOR Get_op_by_token(FEOPCODE op) {
  UINT32 total = sizeof(FEOPCODE_INFO) / sizeof(BIN_OP_TO_OPR);
  for (UINT32 i = 0; i < total; i++) {
    if (op == FEOPCODE_INFO[i]._fe_opcode) {
      return FEOPCODE_INFO[i]._irnode_opcode;
    }
  }
  AssertThat(false, ("Operator %d not implemented.", op));
  return OPERATOR_UNKNOTREE;
}

IR_ITER visitVarDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level,
                  const shared_ptr<NVariableDeclaration>& vardecl) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Var Decl\n"));
  std::shared_ptr<NIdentifier> varname = vardecl->id;
  std::shared_ptr<NIdentifier> vartype = vardecl->type;
  std::shared_ptr<NExpression> rhs = vardecl->assignmentExpr;
  TY_IDX i4_idx = MTYPE_to_ty(MTYPE_I4);
  ST_IDX sym_idx = File()->Find_symbol_by_name(varname->name.c_str());
  // Check if symbol exists, if so, use the previous one.
  if (sym_idx != 0) {
    // Variable redeclare
    Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN),
             (TFile, "Variable redeclare: %s\n", varname->name.c_str()));
    return block_iter;
  }
  STR_IDX var_name_saved = File()->Save_string(varname->name.c_str());
  if (level <= 1) {
    sym_idx = File()->Create_var(var_name_saved, i4_idx, 1, SYMC_FILE_STATIC,
                                        SYME_INTERNAL, SYM_CLASS_VAR);
    // Do we need to store this some where?
  } else {
    AssertThat(level == 2, ("Invalid level = %d", level));
    AssertThat(tree != NULL && block_iter != NULL, ("Null visit context in visitVarDecl"));
    sym_idx = File()->Create_var(var_name_saved, i4_idx, level, SYMC_AUTO,
                                        SYME_INTERNAL, SYM_CLASS_VAR);
    // Parsing the vartype and save to ST table
    if (rhs != nullptr) {
      // assignment is present
      IRNODE_IDX assignment_node = tree->Create_node(OPC_I4STID);
      tree->Get_node(assignment_node)->Set_symbol_idx(sym_idx);
      tree->Get_node(assignment_node)->Set_load_offset(0);
      IR_ITER stid_stmt = tree->Insert_stmt_to_block(block_iter, assignment_node);
      IR_ITER rhs_expr = visitExpression(tree, stid_stmt, level, rhs);
      AssertThat(rhs_expr != block_iter && rhs_expr != stid_stmt && rhs_expr != nullptr, ("Invalid expr conversion result"));
      tree->Set_operand(stid_stmt, 0, rhs_expr);
    }
  }
  return block_iter;
}


void yyerror(char *s, ...)
{
  extern int yylineno;

  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "[Grammar.y] %d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
