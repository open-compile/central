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

// Opcode
BIN_OP_TO_OPR FEOPCODE_INFO[] = {
  { "+", TPLUS,  OPR_ADD },
  { "*", TMUL,   OPR_MPY },
  { "-", TMINUS, OPR_SUB },
  { "/", TDIV,   OPR_DIV },
  { "<", TCLT,   OPR_LT  },
  { ">", TCGT,   OPR_GT  },
};

INT32 femain(COMPILER_CONFIG &conf, FILE_MANAGER &file_man, const char *file_name) {
    // TODO:
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
      IR_ITER var_decl_iter = visitVarDecl(NULL, NULL, 1, false, vardecl);
    } else if (name == "NFunctionDeclaration") {
      visitFunction(reinterpret_cast<const shared_ptr<NFunctionDeclaration> &>(*it));
    } else {
      Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN), (TFile, "Skip other kind of global decl\n"));
    }
  }
}

/**
 * 处理某个function
 * @param func
 */
void visitFunction(const shared_ptr<NFunctionDeclaration> &func) {
  STR_IDX func_name = File()->Save_string(func->id->name.c_str());
  TY_IDX ty_i4 = MTYPE_to_ty(MTYPE_I4);
  TY_IDX ty_v = MTYPE_to_ty(MTYPE_V);
  std::vector<TY_IDX> *param_ret_vec = new std::vector<TY_IDX>;
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "  func typename : %s\n", func->type->name.c_str()));
  if (func->type->name == "int") {
    param_ret_vec->push_back(ty_i4);
  } else if (func->type->name == "void") {
    param_ret_vec->push_back(ty_v);
  } else {
    AssertThat(false, ("Not impl  : %s\n", func->type->name.c_str()))
  }
  for (UINT32 i = 0; i < func->arguments->size(); i++) {
    if ((*(func->arguments))[i]->type->name == "int") {
      param_ret_vec->push_back(ty_i4);
    } else {
      AssertThat(false, ("Not impl  : %s\n", func->type->name.c_str()))
    }
  }
  if (func->arguments->size() == 0) {
    param_ret_vec->push_back(ty_v);
  }
  AssertThat(param_ret_vec->size() >= 2,
             ("param return vec length should be at least 2  : %d\n", param_ret_vec->size()) );
  TY_IDX basic_func_ty = File()->Create_func_ty(func_name, 0, MTYPE_V,
                                                (TY_FLAG) 0, *param_ret_vec);
  ST_IDX func_sym = File()->Create_var(func_name, basic_func_ty,
                                       GLOBAL_SYMTAB, SYMC_TEXT,
                                       SYME_PREEMPTIBLE,
                                       SYM_CLASS_FUNC);
  PU_INFO_IDX pu_info = File()->Create_function(func_sym, basic_func_ty);
  TREE *tree = PU_INFO_pu_info(pu_info)->entry;
  IR_ITER root_entry = tree->Get_root();
  IR_ITER block_iter = tree->Get_operand(root_entry, TREE_SEQ_BODY);

  // Before processing the body, create the formals.
  for (UINT32 i = 0; i < func->arguments->size(); i++) {
    if ((*(func->arguments))[i]->type->name == "int") {
      visitVarDecl(tree, block_iter, LOCAL_SYMTAB, true, (*(func->arguments))[i]);
    } else {
      AssertThat(false, ("Not impl  : %s\n", func->type->name.c_str()))
    }
  }

  // Visit the block contents
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
    visitVarDecl(tree, parent, level, false,
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
  } else if (stmt->getTypeName() == "NIdentifier") {
    visitIdentifierStmt(tree, parent, level,
                        reinterpret_cast<const shared_ptr<NIdentifier> &> (stmt));
  } else if (stmt->getTypeName() == "NMethodCall") {
    visitMethodCall(tree, parent, level,
                    reinterpret_cast<const shared_ptr<NMethodCall> &> (stmt));
  } else if (stmt->getTypeName() == "NReturnStatement") {
    visitReturnStmt(tree, parent, level,
                    reinterpret_cast<const shared_ptr<NReturnStatement> &> (stmt));
  } /*else if (stmt->getTypeName() == "NArrayIndex") {
    visitArrayDecl(tree, parent, level,
                    reinterpret_cast<const shared_ptr<NArrayIndex> &> (stmt));
  }*/ else {
    AssertThat(FALSE, ("not implemented kind of stmt = %s", stmt->getTypeName().c_str()));
  }
  return parent;
}

IR_ITER visitReturnStmt(TREE *tree, IR_ITER parent, int level,
                        const shared_ptr<NReturnStatement> &stmt) {
  shared_ptr<NExpression> return_val = stmt->expression;
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Return Stmt\n"));
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


IR_ITER visitMethodCall(TREE *tree, IR_ITER parent, int level,
                        const shared_ptr<NMethodCall> &stmt) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Call Stmt\n"));
  shared_ptr<NIdentifier> callee_name = stmt->id;
  shared_ptr<ExpressionList> args = stmt->arguments;
  AssertThat(callee_name != nullptr,
             ("Invalid callee name"));

  ST_IDX func_sym = File()->Find_symbol_by_name(callee_name->name.c_str());
  // If func_sym does not exist, this should be a compile-time error.
  if (func_sym == 0) {
    Comp_Failure("Cannot find function declaration for name = %s", callee_name->name.c_str());
  }
  AssertThat(ST_sclass(func_sym) == SYMC_TEXT ||
             ST_sclass(func_sym) == SYMC_EXTERN,
             ("Must be internal or imported function. %s",
              ST_name(func_sym)));
  TY_IDX func_ty = ST_ty(func_sym);
  TYLIST_IDX tyl_idx = TY_tylist_id(func_ty);

  IRNODE_IDX call_node = 0;
  if (TY_kind(TYLIST_tylist(tyl_idx)->ty_id) == TY_KIND::KIND_SCALAR) {
    // Assume I4
    call_node = tree->Create_node(OPC_I4CALL);
  } else {
    call_node = tree->Create_node(OPC_VCALL);
  }
  // TODO check whether the arguments are matching ....
  IR_ITER call_stmt    = tree->Insert_stmt_to_block(parent, call_node);
  tree->Node(call_stmt)->Set_symbol_idx(func_sym);
  for (UINT32 i = 0; i < args->size(); i++) {
    IR_ITER call_opnd = visitExpression(tree, call_stmt, level, (*args)[i]);
    AssertThat(call_opnd != parent && call_opnd != nullptr, ("Invalid expr conversion result"));
    tree->Set_operand(call_stmt, 0, call_opnd);
  }
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
  if (OPCODE_rtype(tree->Get_node(condition_expr)->Opcode()) != MTYPE_B) {
    // Add a NE as a condition
    IRNODE_IDX cond = tree->Create_node(OPC_I4I4NE);
    IRNODE_IDX const_zero = tree->Create_node(OPC_I4CONST);
    IR_ITER cond_node = tree->Set_operand(if_stmt, 0, cond);
    tree->Node(const_zero)->Set_const_val(0);
    tree->Set_operand(cond_node, 0, condition_expr);
    tree->Set_operand(cond_node, 1, const_zero);
  } else {
    tree->Set_operand(if_stmt, 0, condition_expr);
  }

  // 处理then块
  IR_ITER then_stmt;
  IRNODE_IDX then_node = tree->Create_node(OPC_BLOCK);
  then_stmt = tree->Set_operand(if_stmt, 1, then_node);
  visitBlock(tree, then_stmt, level, true_block);

  // 处理else块，有可能不存在
  IR_ITER else_stmt;
  IRNODE_IDX else_node = tree->Create_node(OPC_BLOCK);
  else_stmt = tree->Set_operand(if_stmt, 2, else_node);
  if(false_block != nullptr) {
      visitBlock(tree, else_stmt, level, false_block);
  }
  return parent;
}

IR_ITER visitForStmt(TREE *tree, IR_ITER parent, int level,
                    const shared_ptr<NForStatement> &stmt) {


    shared_ptr<NExpression> condition = stmt->condition;
    shared_ptr<NBlock> true_block = stmt->block;
    AssertThat(condition != nullptr, ("condition should not be null"));

    IR_ITER label1_stmt;
    STR_IDX str = File()->Save_string("while_start");
    IRNODE_IDX label1_node = tree->Create_node(OPC_LABEL);
    LABEL_IDX label_id = File()->Create_label(str,
                                              LABEL_ADDR_SAVED,
                                              LKIND_DEFAULT);
    label1_stmt = tree->Insert_stmt_to_block(parent, label1_node);
    tree->Get_node(label1_stmt)->Set_label_num(label_id);

    IR_ITER while_stmt;
    IRNODE_IDX while_node = tree->Create_node(OPC_WHILE_DO  );
    while_stmt = tree->Insert_stmt_to_block(parent, while_node);

    //处理循环判断条件
    IR_ITER condition_expr = visitExpression(tree, while_stmt, level, condition);
    AssertThat(condition_expr != parent && condition_expr != while_stmt && condition_expr != nullptr, ("Invalid expr conversion result"));
    tree->Set_operand(while_stmt, 0, condition_expr);

    IR_ITER label2_stmt;
    STR_IDX str2 = File()->Save_string("while_do");
    IRNODE_IDX label2_node = tree->Create_node(OPC_LABEL);
    LABEL_IDX label2_id = File()->Create_label(str2,
                                              LABEL_ADDR_SAVED,
                                              LKIND_DEFAULT);

    //处理循环体
    IR_ITER do_stmt;
    IRNODE_IDX then_node = tree->Create_node(OPC_BLOCK);
    do_stmt = tree->Set_operand(while_stmt, 1, then_node);
    visitBlock(tree, do_stmt, level, true_block);
    label2_stmt = tree->Insert_stmt_to_block(parent, label2_node);
    tree->Get_node(label2_stmt)->Set_label_num(label2_id);

    IR_ITER label3_stmt;
    STR_IDX str3 = File()->Save_string("while_end");
    IRNODE_IDX label3_node = tree->Create_node(OPC_LABEL);
    LABEL_IDX label3_id = File()->Create_label(str3,
                                               LABEL_ADDR_SAVED,
                                               LKIND_DEFAULT);
    label3_stmt = tree->Insert_stmt_to_block(parent, label3_node);
    tree->Get_node(label3_stmt)->Set_label_num(label3_id);

    return parent;
}

IR_ITER visitIdentifierStmt(TREE *tree, IR_ITER parent, int level,
                     const shared_ptr<NIdentifier> &stmt){
    IR_ITER Identifier_stmt;
    IRNODE_IDX Identifier_node = tree->Create_node(OPC_GOTO);
    Identifier_stmt = tree->Insert_stmt_to_block(parent, Identifier_node);
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
    OPCODE opc = (OPCODE) (opr + RTYPE(MTYPE_B) + DESC(MTYPE_I4));
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
  } /*else if (expr->getTypeName() == "NArrayIndex") {
    // use of variable.
    auto val = reinterpret_cast<shared_ptr<NArrayIndex> &>(expr);
    IRNODE_IDX iload_node = tree->Create_node(OPC_I4I4ILOAD);
    // Find symbol idx.
    ST_IDX sym = File()->Find_symbol_by_name(val->arrayName->name.c_str());
    if (sym == 0) {
      Comp_Failure("Use of undeclared symbol : %s ", val->arrayName->name.c_str());
    }
    tree->Get_node(iload_node)->Set_symbol_idx(sym);
    IR_ITER cur_node = tree->Insert_temp_node(iload_node);
    return cur_node;


    ARB_IDX one_arb = TY_arb(basic_array_ty);
  } */else {
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

IR_ITER visitVarDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level, BOOL is_formal,
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
    SYM_SCLASS sclass = SYMC_AUTO;
    if (is_formal) {
      sclass = SYMC_FORMAL;
    }
    sym_idx = File()->Create_var(var_name_saved, i4_idx, level, sclass,
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
/*

IR_ITER visitArrayDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level,
                     const shared_ptr<NArrayIndex>& arraydecl) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Var Decl\n"));
  std::shared_ptr<NIdentifier> arrayname = arraydecl->arrayName;
  std::shared_ptr<ExpressionList> expressions = arraydecl->expressions;

  TY_IDX i4_idx = MTYPE_to_ty(MTYPE_I4);
  ST_IDX sym_idx = File()->Find_symbol_by_name(arrayname->name.c_str());
  // Check if symbol exists, if so, use the previous one.
  if (sym_idx != 0) {
    // Variable redeclare
    Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN),
             (TFile, "Variable redeclare: %s\n", arrayname->name.c_str()));
    return block_iter;
  }
  STR_IDX anon_array = File()->Save_string(arrayname->name.c_str());
  if (level <= 1) {
    ARB_IDX arb_idx[expressions->size()];//维数
    int i = 0;
    int j = expressions->size();
    for (auto it = expressions->begin(); it != expressions->end(); it++, i++) {
      IR_ITER expr = visitExpression(tree, if_stmt, level, it);
      arb_idx[i] = File()->Create_array_bound_const(expr, MTYPE_size(MTYPE_I4), j--,
                                                    i == 0 ? ARB_FIRST_DIMEN : (i == expressions->size() - 1 ? ARB_LAST_DIMEN : 0));
    }

    TY_IDX array_ty[expressions->size()];
    for (int k = 0; k < expressions->size(); ++k) {
      array_ty[k] = File()->Create_array_ty(anon_array, TY_FLAG_INTERNAL,
                                            k == 0 ? i4_idx : array_ty[k-1], arb_idx[--i]);
    }
  } else {
    AssertThat(level == 2, ("Invalid level = %d", level));
    AssertThat(tree != NULL && block_iter != NULL, ("Null visit context in visitVarDecl"));
    ARB_IDX arb_idx[expressions->size()];//维数
    int i = 0;
    int j = expressions->size();
    for (auto it = expressions->begin(); it != expressions->end(); it++, i++) {
      IR_ITER expr = visitExpression(tree, if_stmt, level, it);
      arb_idx[i] = File()->Create_array_bound_const(expr, MTYPE_size(MTYPE_I4), j--,
                                                    i == 0 ? ARB_FIRST_DIMEN : (i == expressions->size() - 1 ? ARB_LAST_DIMEN : 0));
    }

    TY_IDX array_ty[expressions->size()];
    for (int k = 0; k < expressions->size(); ++k) {
      array_ty[k] = File()->Create_array_ty(anon_array, TY_FLAG_INTERNAL,
                                            i4_idx, arb_idx[--i]);
    }
  }
  return block_iter;
}
*/

void yyerror(char *s, ...)
{
  extern int yylineno;

  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "[Grammar.y] %d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
