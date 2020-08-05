//
// Created by xc5 on 2020/6/14.
//

#include <iostream>
#include <fstream>
#include "symtab_access.h"
#include "parser_tree.h"
#include "options.h"
#include "stdarg.h"
#include "ir.h"
#include "fe_main.h"
#include "cpp_pre.h"

extern int yylineno;

// Opcode
BIN_OP_TO_OPR FEOPCODE_INFO[] = {
  // tok TOKEN   oper     rtype
  { "+", TPLUS,  OPR_ADD, MTYPE_I4 },
  { "*", TMUL,   OPR_MPY, MTYPE_I4 },
  { "-", TMINUS, OPR_SUB, MTYPE_I4 },
  { "/", TDIV,   OPR_DIV, MTYPE_I4 },
  { "<", TCLT,   OPR_LT , MTYPE_B  },
  { ">", TCGT,   OPR_GT , MTYPE_B  },
  { "%", TMOD,   OPR_MOD , MTYPE_I4  },
  { "&", TAND,   OPR_BAND , MTYPE_I4  },
//  { "|", TOR,   OPR_BAND , MTYPE_I4  },
  { "<=", TCLE,   OPR_LE , MTYPE_B  },
  { ">=", TCGE,   OPR_GE , MTYPE_B  },
  { "!=", TCNE,   OPR_NE , MTYPE_B  },
  { "==", TCEQ,   OPR_EQ , MTYPE_B  },
  { "!",  TNOT,   OPR_LNOT,MTYPE_B  },
  { "||", TLOR,   OPR_BIOR,MTYPE_B  },
  { "&&", TLAND,  OPR_LAND,MTYPE_B  },
};

void Create_internal_functions() {
  TY_IDX ty_i4 = MTYPE_to_ty(MTYPE_I4);
  TY_IDX ty_v = MTYPE_to_ty(MTYPE_V);
  ARB_IDX arb = File()->Create_array_bound_const(0, 4, 1, ARB_FLAGS::ARB_CONST_UBND);
  TY_IDX ty_ivec = File()->Create_array_ty(File()->Save_string(".internal.i4"),
                                           TY_FLAG_INTERNAL, ty_i4, arb);

  STR_IDX internal_func_name = File()->Save_string(".internal.func");
  std::vector<TY_IDX> *param_ret_vec = new std::vector<TY_IDX>;
  param_ret_vec->clear();
  param_ret_vec->push_back(ty_i4);
  param_ret_vec->push_back(ty_v);
  TY_IDX one_ret = File()->Create_func_ty(internal_func_name, 0, MTYPE_V, TY_FLAG::TY_ANONYMOUS,
                                          *param_ret_vec);

  param_ret_vec->clear();
  param_ret_vec->push_back(ty_i4);
  param_ret_vec->push_back(ty_ivec);
  TY_IDX one_ret_one_array_parm = File()->Create_func_ty(internal_func_name, 0, MTYPE_V, TY_FLAG::TY_ANONYMOUS,
                                                         *param_ret_vec);

  param_ret_vec->clear();
  param_ret_vec->push_back(ty_v);
  param_ret_vec->push_back(ty_i4);
  TY_IDX void_ret_one_parm = File()->Create_func_ty(internal_func_name, 0, MTYPE_V, TY_FLAG::TY_ANONYMOUS,
                                                    *param_ret_vec);

  param_ret_vec->clear();
  param_ret_vec->push_back(ty_v);
  param_ret_vec->push_back(ty_ivec);
  TY_IDX void_ret_one_vec = File()->Create_func_ty(internal_func_name, 0, MTYPE_V, TY_FLAG::TY_ANONYMOUS,
                                                   *param_ret_vec);

  STR_IDX func_name = File()->Save_string("getint");
  File()->Create_var(func_name, one_ret,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("getch");
  File()->Create_var(func_name, one_ret,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("getarray");
  File()->Create_var(func_name, one_ret_one_array_parm,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("putint");
  File()->Create_var(func_name, void_ret_one_parm,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("putch");
  File()->Create_var(func_name, void_ret_one_parm,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("putarray");
  File()->Create_var(func_name, void_ret_one_vec,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_EXTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("_sysy_starttime");
  ST_IDX internal_func = File()->Create_var(func_name, void_ret_one_parm,
                    GLOBAL_SYMTAB, SYMC_EXTERN, SYME_INTERNAL, SYM_CLASS_FUNC);

  func_name = File()->Save_string("_sysy_stoptime");
  File()->Create_var(func_name, void_ret_one_parm,
                     GLOBAL_SYMTAB, SYMC_EXTERN, SYME_INTERNAL, SYM_CLASS_FUNC);


//  int getint();
//  int getch();
//  int getarray(int a[]);
//  void putint(int a);
//  void putch(int a);
//  void putarray(int n,int a[]);
}

INT32 femain(COMPILER_CONFIG &conf, FILE_MANAGER &file_man, const char *file_name) {

  // Create internal functions.
  Create_internal_functions();

  extern FILE *yyin;
  if ((yyin = fopen(file_name, "r")) == NULL) {
    Comp_Failure("Failed to open source code : %s", file_name);
  }
  yyparse();

  if (!programBlock) {
    Comp_Failure("Syntax check failed for file : %s:%d", file_name, yylineno);
  }

  if(Tracing(COMPONENT_FE, TRACE_INFO)) {
    programBlock->print("--");
  }
  auto root = programBlock->jsonGen();
  Irgen_visit(programBlock);

//  string jsonFile = "visualization/A_tree.json";
//  std::ofstream astJson(jsonFile);
//  if( astJson.is_open() ){
//    astJson << root;
//    astJson.close();
//  }

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
    visitMethodCall(tree, parent, level, false,
                    reinterpret_cast<const shared_ptr<NMethodCall> &> (stmt));
  } else if (stmt->getTypeName() == "NReturnStatement") {
    visitReturnStmt(tree, parent, level,
                    reinterpret_cast<const shared_ptr<NReturnStatement> &> (stmt));
  } else if (stmt->getTypeName() == "NArrayAssignment") {
    visitArrayAssignmentStmt(tree, parent, level,
                             reinterpret_cast<const shared_ptr<NArrayAssignment> &> (stmt));
  } else if (stmt->getTypeName() == "NExpressionStatement") {
    shared_ptr<NExpressionStatement> expr = reinterpret_cast<const shared_ptr<NExpressionStatement> &> (stmt);
    if (expr->expression->getTypeName() == "NBlock") {
      visitBlock(tree, parent, level,
                 reinterpret_cast<const shared_ptr<NBlock> &> (expr->expression));
    } else {
      AssertThat(FALSE,
                 ("not implemented kind of expr-stmt, expr = %s", expr->getTypeName().c_str()));
    }
  } else if (stmt->getTypeName() == "NBlock") {
    shared_ptr<NBlock> expr = reinterpret_cast<const shared_ptr<NBlock> &> (stmt);
    visitBlock(tree, parent, level, expr);
  } else {
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


IR_ITER visitMethodCall(TREE *tree, IR_ITER parent, int level, BOOL is_expr,
                        const shared_ptr<NMethodCall> &stmt) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Call Stmt\n"));
  shared_ptr<NIdentifier> callee_name = stmt->id;
  shared_ptr<ExpressionList> args = stmt->arguments;
  AssertThat(callee_name != nullptr,
             ("Invalid callee name"));
  const char *real_name = callee_name->name.c_str();
  if (strcmp(real_name, "starttime") == 0) {
    real_name = "_sysy_starttime";
  } else if (strcmp(real_name, "stoptime") == 0) {
    real_name = "_sysy_stoptime";
  }

  ST_IDX func_sym = File()->Find_symbol_by_name(real_name);
  // If func_sym does not exist, this should be a compile-time error.
  if (func_sym == 0) {
    Comp_Failure("Cannot find function declaration for name = %s", real_name);
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
  // Add to tree.
  IR_ITER call_stmt;
  IR_ITER ret_stmt;
  if (is_expr) {
    // Create a COMMA + block
    IRNODE_IDX comma_idx   = tree->Create_node(OPC_COMMA);
    IRNODE_IDX block_idx   = tree->Create_node(OPC_BLOCK);
    IRNODE_IDX stid_idx    = tree->Create_node(OPC_I4STID);
    IRNODE_IDX ldid_idx    = tree->Create_node(OPC_I4LDID);
    IRNODE_IDX ld_ret_node = tree->Create_node(OPC_I4LDID);
    ret_stmt = tree->Insert_temp_node(comma_idx);
    IR_ITER block_expr = tree->Set_operand(ret_stmt, 0, block_idx);
    call_stmt = tree->Insert_stmt_to_block(block_expr, call_node);
    IR_ITER stid_expr =  tree->Insert_stmt_to_block(block_expr, stid_idx);
    IR_ITER ldid_expr = tree->Set_operand(ret_stmt, 1, ldid_idx);
    IR_ITER ld_ret_expr = tree->Set_operand(stid_expr, 0, ld_ret_node);
    PREG_IDX preg = File()->Create_preg(File()->Save_string(".ret_medium"), 0);
    PREG_IDX preg_ret = File()->Create_preg(File()->Save_string(".return_val"), 1);
    tree->Node(ldid_expr)->Set_symbol_idx(File()->Get_preg_sym(MTYPE_I4, preg));
    tree->Node(ldid_expr)->Set_preg_num(preg);
    tree->Node(stid_expr)->Set_symbol_idx(File()->Get_preg_sym(MTYPE_I4, preg));
    tree->Node(stid_expr)->Set_preg_num(preg);
    tree->Node(ld_ret_expr)->Set_symbol_idx(File()->Get_preg_sym(MTYPE_I4, preg));
    tree->Node(ld_ret_expr)->Set_preg_num(preg_ret);
  } else {
    call_stmt = tree->Insert_stmt_to_block(parent, call_node);
    ret_stmt = call_stmt;
  }
  tree->Node(call_stmt)->Set_symbol_idx(func_sym);
  if (ST_eclass(func_sym) == SYME_INTERNAL) {
    // stoptime / starttime.
    IRNODE_IDX const_node = tree->Create_node(OPC_I4CONST);
    tree->Set_operand(call_stmt, 0, const_node);
    tree->Node(const_node)->Set_const_val(stmt->Get_lineno());
  } else {
    for (UINT32 i = 0; i < args->size(); i++) {
      IR_ITER call_opnd = visitExpression(tree, call_stmt, level, (*args)[i]);
      AssertThat(call_opnd != parent && call_opnd != nullptr,
                 ("Invalid expr conversion result"));
      tree->Set_operand(call_stmt, i, call_opnd);
    }
  }
  return ret_stmt;
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

IR_ITER visitIdentifierStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NIdentifier> &stmt) {
  if (stmt->name == "break") {
    IRNODE_IDX identifier_node = tree->Create_node(OPC_GOTO_OUT);
    tree->Node(identifier_node)->Set_label_num(GOTO_OUT_BREAK);
    IR_ITER id_stmt = tree->Insert_stmt_to_block(parent, identifier_node);
  } else if (stmt->name == "continue") {
    IRNODE_IDX identifier_node = tree->Create_node(OPC_GOTO_OUT);
    tree->Node(identifier_node)->Set_label_num(GOTO_OUT_CONTINUE);
    IR_ITER id_stmt = tree->Insert_stmt_to_block(parent, identifier_node);
  }
  return parent;
}

IR_ITER visitArrayAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NArrayAssignment> &stmt) {
  std::shared_ptr<NArrayIndex> array_index = stmt->arrayIndex;
  shared_ptr<NExpression> rhs = stmt->expression;

  AssertThat(array_index != nullptr, ("Array index should not be null"));
  AssertThat(rhs != nullptr, ("Rhs should not be null"));

  IR_ITER stid_stmt;
  IRNODE_IDX assignment_node = tree->Create_node(OPC_I4I4ISTORE);
  stid_stmt = tree->Insert_stmt_to_block(parent, assignment_node);

  // 插入表达式结点
  IR_ITER rhs_expr = visitExpression(tree, stid_stmt, level, rhs);
  AssertThat(rhs_expr != parent && rhs_expr != stid_stmt && rhs_expr != nullptr, ("Invalid expr conversion result"));
  tree->Set_operand(stid_stmt, 0, rhs_expr);

  IRNODE_IDX array_node = tree->Create_node(OPC_ARRAY);

  ST_IDX sym = File()->Find_symbol_by_name(array_index->arrayName->name.c_str());
  AssertThat(sym != 0,
             ("Cannot find symbol for array store. = %s",
               array_index->arrayName->name.c_str()));
  TY_IDX ty = ST_ty(sym);
  ARB_IDX one_arb = TY_arb(ty);
  IRNODE_IDX lda_node = 0;
  if (ST_sclass(sym) == SYMC_FORMAL) {
    lda_node = tree->Create_node(OPC_I4LDID);
  } else {
    lda_node = tree->Create_node(OPC_LDA);
  }

  tree->Get_node(array_node)->Set_const_val(ARB_dimension(one_arb));
  tree->Get_node(lda_node)->Set_symbol_idx(sym);

  // 插入array结点
  tree->Set_operand(stid_stmt, 1, array_node);

  // 插入lda结点
  IR_ITER temp_array_node = tree->Get_operand(stid_stmt, 1);
  tree->Add_child(temp_array_node, lda_node);

  // 插入数组原维度大小的结点
  int i;
  for (i = 1; i <= ARB_dimension(one_arb); ++i) {
    IRNODE_IDX int_const_node = tree->Create_node(OPC_I4CONST);
    tree->Get_node(int_const_node)->Set_const_val(ARB_ubnd_val(one_arb + i - 1));
    tree->Set_operand(temp_array_node, i, int_const_node);
  }

  // 插入加载的各维度结点
  for (auto it = array_index->expressions->begin(); it != array_index->expressions->end(); it++, i++) {
    IR_ITER dimension = visitExpression(tree, temp_array_node, level,
                                        reinterpret_cast<const shared_ptr<struct NExpression> &>(*it));
    tree->Set_operand(temp_array_node, i, dimension);
  }

  return parent;
}

IR_ITER visitAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NAssignment> &stmt) {
  std::shared_ptr<NIdentifier> varname = stmt->lhs;
  shared_ptr<NExpression> rhs = stmt->rhs;

  AssertThat(varname != nullptr, ("Var name should not be null"));
  AssertThat(rhs != nullptr, ("Rhs should not be null"));

  ST_IDX sym_idx = File()->Find_symbol_by_name(varname->name.c_str());
  AssertThat(sym_idx != 0, ("Cannot find symbol to assign to."));
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
    OPERATOR                    opr    = Get_op_by_token((FEOPCODE) bin_op->op);
    AssertThat(opr >= OPERATOR_FIRST, ("Operator not implementeed"));
    OPCODE     opc      = (OPCODE) (opr + RTYPE(Get_rtype_by_token((FEOPCODE) bin_op->op)) + DESC(MTYPE_I4));
    IRNODE_IDX opr_node = tree->Create_node(opc);
    IR_ITER    cur_node = tree->Insert_temp_node(opr_node);
    IR_ITER    lhs      = visitExpression(tree, cur_node, level, bin_op->lhs);
    IR_ITER    rhs      = visitExpression(tree, cur_node, level, bin_op->rhs);
    tree->Set_operand(cur_node, 0, lhs); // lhs should be on the 0 operand.
    tree->Set_operand(cur_node, 1, rhs); // rhs should be on the 1 operand.
    return cur_node;
  } else if (expr->getTypeName() == "NUnaryOperator") {
    shared_ptr<NUnaryOperator> u_op = reinterpret_cast<const shared_ptr<NUnaryOperator> &>(expr);
    OPERATOR opr = Get_op_by_token((FEOPCODE) u_op->op);
    AssertThat(opr >= OPERATOR_FIRST, ("Operator not implementeed"));
    OPCODE opc = (OPCODE) (opr + RTYPE(Get_rtype_by_token((FEOPCODE) u_op->op)) + DESC(MTYPE_I4));
    IR_ITER rhs = visitExpression(tree, parent, level, u_op->rhs);
    IR_ITER cur_node = rhs;
    if (tree->Node(rhs)->Opcode() == OPC_I4CONST) {
      tree->Node(rhs)->Set_const_val(-tree->Node(rhs)->Get_const_val());
    } else {
      OPCODE opc = (u_op->op == TNEG) ? OPC_I4I4SUB : OPC_I4I4ADD;
      IRNODE_IDX opr_node = tree->Create_node(opc);
      IRNODE_IDX zero_node = tree->Create_node(OPC_I4CONST);
      cur_node = tree->Insert_temp_node(opr_node);
      tree->Set_operand(cur_node, 0, zero_node);
      tree->Set_operand(cur_node, 1, rhs); // rhs should be here
    }
    return cur_node;
  } else if (expr->getTypeName() == "NDouble") {
    AssertThat(false, ("double not implemented."));
  } else if (expr->getTypeName() == "NMethodCall") {
    return visitMethodCall(tree, parent, level, true,
                    reinterpret_cast<const shared_ptr<NMethodCall> &> (expr));
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
  } else if (expr->getTypeName() == "NArrayIndex") {
    // use of variable.
    auto       val        = reinterpret_cast<shared_ptr<NArrayIndex> &>(expr);
    IRNODE_IDX iload_node = tree->Create_node(OPC_I4I4ILOAD);
    IRNODE_IDX array_node = tree->Create_node(OPC_ARRAY);
    IRNODE_IDX lda_node   = 0;

    ST_IDX  sym     = File()->Find_symbol_by_name(val->arrayName->name.c_str());
    TY_IDX  ty      = ST_ty(sym);
    ARB_IDX one_arb = TY_arb(ty);

    if (ST_sclass(sym) == SYMC_FORMAL) {
      lda_node = tree->Create_node(OPC_I4LDID);
    } else {
      lda_node = tree->Create_node(OPC_LDA);
    }


    IR_ITER cur_node = tree->Insert_temp_node(iload_node);
    tree->Get_node(array_node)->Set_const_val(ARB_dimension(one_arb));
    tree->Get_node(lda_node)->Set_symbol_idx(sym);
    tree->Add_child(cur_node, array_node);

    IR_ITER temp_array_node = tree->Get_operand(cur_node, 0);
    tree->Add_child(temp_array_node, lda_node);

    // 插入数组原维度大小的结点
    int i;
    for (i = 1; i <= ARB_dimension(one_arb); ++i) {
      IRNODE_IDX int_const_node = tree->Create_node(OPC_I4CONST);
      tree->Get_node(int_const_node)->Set_const_val(
        ARB_ubnd_val(one_arb + i - 1));
      tree->Set_operand(temp_array_node, i, int_const_node);
    }

    // 插入加载的各维度结点
    for (auto it = val->expressions->begin();
         it != val->expressions->end(); it++, i++) {
      IR_ITER dimension = visitExpression(tree, temp_array_node, level,
                                          reinterpret_cast<const shared_ptr<struct NExpression> &>(*it));
      tree->Set_operand(temp_array_node, i, dimension);
    }

    return cur_node;
  } else if (expr->getTypeName() == "NInitializeExpr") {
    // Initialize expr with { {...}, ... i, j, } kind of format

    AssertThat(false,
               ("Initialization expr not implemented : %s",
                 expr->getTypeName().c_str()));
    // INITO Generation needed.
  } else if (expr->getTypeName() == "NInitializeExpr") {
    // Initialize expr with { {...}, ... i, j, } kind of format
    AssertThat(false,
               ("Initialization expr not implemented : %s",
                 expr->getTypeName().c_str()));
    // INITO Generation needed.
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

MTYPE_ID Get_rtype_by_token(FEOPCODE op) {
  UINT32 total = sizeof(FEOPCODE_INFO) / sizeof(BIN_OP_TO_OPR);
  for (UINT32 i = 0; i < total; i++) {
    if (op == FEOPCODE_INFO[i]._fe_opcode) {
      return FEOPCODE_INFO[i]._rtype;
    }
  }
  AssertThat(false, ("Operator %d not implemented.", op));
  return MTYPE_V;
}


INT64 Evaluate_const_expr(shared_ptr<NExpression> sharedPtr) {
  INT64 cur_val = 0;
  if (sharedPtr->getTypeName() == "NBinaryOperator") {
    shared_ptr<NBinaryOperator> bin_op = reinterpret_cast<const shared_ptr<NBinaryOperator> &>(sharedPtr);
    INT64                       lhs    = Evaluate_const_expr(bin_op->lhs);
    if (lhs == 0) {
      return 0;
    }
    INT64                       rhs    = Evaluate_const_expr(bin_op->rhs);
    switch (bin_op->op) {
      case TPLUS:
        cur_val = lhs + rhs;
        break;
      case TMINUS:
        cur_val = lhs - rhs;
        break;
      case TDIV:
        cur_val = lhs / rhs;
        break;
      case TMUL:
        cur_val = lhs * rhs;
        break;
      case TMOD:
        cur_val = lhs % rhs;
        break;
      default:
        AssertThat(false, ("Cannot handle situation op : %d", bin_op->op));
    }
  } else if (sharedPtr->getTypeName() == "NIdentifier"){
    shared_ptr<NIdentifier> bin_op = reinterpret_cast<const shared_ptr<NIdentifier> &>(sharedPtr);
    ST_IDX sym = File()->Find_symbol_by_name(bin_op->name.c_str());
    if (sym != 0 && ST_st(sym)->getInitoIdx() != 0) {
      INITO *inito = INITO_inito(ST_st(sym)->getInitoIdx());
      AssertThat(inito->Size() > 0, ("No value in inito found."));
      cur_val = inito->Value(0)->Val();
    } else {
      Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN),
               (TFile, "Cannot find predef value for sym : %d", sym));
    }
  } else if (sharedPtr->getTypeName() == "NUnaryOperator"){
    shared_ptr<NUnaryOperator> bin_op = reinterpret_cast<const shared_ptr<NUnaryOperator> &>(sharedPtr);
    INT32 lhs = 0;
    INT64 rhs = Evaluate_const_expr(bin_op->rhs);
    if (rhs == 0) {
      return 0;
    }
    switch (bin_op->op) {
      case TPLUS:
        cur_val = lhs + rhs;
        break;
      case TMINUS:
        cur_val = lhs - rhs;
        break;
      case TDIV:
        cur_val = lhs / rhs;
        break;
      case TMUL:
        cur_val = lhs * rhs;
        break;
      case TMOD:
        cur_val = lhs % rhs;
        break;
      default:
        AssertThat(false, ("Cannot handle situation op : %d", bin_op->op));
    }
  } else if (sharedPtr->getTypeName() == "NInteger"){
    shared_ptr<NInteger> bin_op = reinterpret_cast<const shared_ptr<NInteger> &>(sharedPtr);
    cur_val =  bin_op->value;
  } else {
    cur_val = 0;
  }
  return cur_val;
}


ST_IDX visitArrayDecl(UINT32 level,
                      shared_ptr<NIdentifier> varname,
                      shared_ptr<NIdentifier> vartype,
                      TY_IDX i4_idx,
                      SYM_SCLASS sclass) {
  ST_IDX                              sym_idx;
  STR_IDX                             anon_array = File()->Save_string(varname->name.c_str());//数组名
  ARB_IDX                             arb_idx[vartype->arraySize->size()];
  int                                 i          = 0;
  int                                 j          = vartype->arraySize->size();//维数
  for (ExpressionList::const_iterator it         = vartype->arraySize->cbegin(); it != vartype->arraySize->cend(); it++, i++) {
    if ((*it)->getTypeName() == "NInteger") {
      auto val = reinterpret_cast<const std::shared_ptr<NInteger> &> (*it);
      arb_idx[i] = File()->Create_array_bound_const(val->value, MTYPE_size(MTYPE_I4), j--,
                                                    i == 0 ? ARB_FIRST_DIMEN : (i == vartype->arraySize->size() - 1 ? ARB_LAST_DIMEN : 0));
    } else if ((*it)->getTypeName() == "NIdentifier") {
      INT64 const_val = Evaluate_const_expr(*it);
      if (const_val > 0) {
        arb_idx[i] = File()->Create_array_bound_const(const_val, MTYPE_size(MTYPE_I4), j--,
                                                      i == 0 ? ARB_FIRST_DIMEN : (i == vartype->arraySize->size() - 1 ? ARB_LAST_DIMEN : 0));
      } else {
        auto expr = (*it);
        const std::shared_ptr<NIdentifier> &val = reinterpret_cast<const std::shared_ptr<NIdentifier> &>(expr);
        ST_IDX sym = File()->Find_symbol_by_name(val->name.c_str());
        arb_idx[i] = File()->Create_array_bound_var(sym, MTYPE_size(MTYPE_I4),
                                                    j--,
                                                    i == 0 ? ARB_FIRST_DIMEN : (
                                                      i ==
                                                      vartype.get()->arraySize->size() -
                                                      1 ? ARB_LAST_DIMEN : 0));
      }
    } else {
      INT64 const_val = Evaluate_const_expr(*it);
      if (const_val > 0) {
        arb_idx[i] = File()->Create_array_bound_const(const_val, MTYPE_size(MTYPE_I4), j--,
                                                      i == 0 ? ARB_FIRST_DIMEN : (i == vartype->arraySize->size() - 1 ? ARB_LAST_DIMEN : 0));
      } else {
        arb_idx[i] = File()->Create_array_bound_var(0, MTYPE_size(MTYPE_I4),
                                                    j--,
                                                    i == 0 ? ARB_FIRST_DIMEN : (
                                                      i ==
                                                      vartype.get()->arraySize->size() -
                                                      1 ? ARB_LAST_DIMEN : 0));
      }
    }
  }
  TY_IDX array_ty[vartype->arraySize->size()];
  for (int k                                     = 0; k < vartype->arraySize->size(); ++k) {
    array_ty[k] = File()->Create_array_ty(anon_array, TY_FLAG_INTERNAL,
                                          k == 0 ? i4_idx : array_ty[k-1], arb_idx[--i]);
  }
  sym_idx = File()->Create_var(anon_array, array_ty[vartype->arraySize->size() - 1], level, sclass,
                               SYME_INTERNAL, SYM_CLASS_VAR);
  return sym_idx;
}


IR_ITER visitVarDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level, BOOL is_formal,
                     const shared_ptr<NVariableDeclaration>& vardecl) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Var Decl\n"));
  // Iterate over all children.
  if (vardecl->Get_decls().size() > 0) {
    for (UINT32 i = 0; i < vardecl->Get_decls().size(); i++) {
      visitVarDecl(tree, block_iter, level, is_formal, vardecl->Get_decls()[i]);
    }
  }

  // finish the children, process this var decl.
  std::shared_ptr<NIdentifier> varname = vardecl->id;
  std::shared_ptr<NIdentifier> vartype = vardecl->type;
  std::shared_ptr<NExpression> rhs = vardecl->assignmentExpr;
  TY_IDX i4_idx = MTYPE_to_ty(MTYPE_I4);
  ST_IDX sym_idx = File()->Find_symbol_by_name(varname->name.c_str());
  // Check if symbol exists, if so, use the previous one.
  if (sym_idx != 0 &&
     !(ST_sclass(sym_idx) == SYMC_FORMAL && level == GLOBAL_SYMTAB) &&
     !(ST_sclass(sym_idx) == SYMC_AUTO   && level == GLOBAL_SYMTAB) &&
     !(ST_sclass(sym_idx) == SYMC_FILE_STATIC && level == LOCAL_SYMTAB)) {
    // Variable redeclare
    Is_Trace(Tracing(COMPONENT_FE, TRACE_WARN),
             (TFile, "Variable redeclare: %s\n", varname->name.c_str()));
    return block_iter;
  }
  STR_IDX var_name_saved = File()->Save_string(varname->name.c_str());
  SYM_SCLASS sclass = SYMC_AUTO;
  if (level <= GLOBAL_SYMTAB) {
    // 数组声明, global
    sclass = SYMC_FILE_STATIC;
    if (vartype->isArray) {
      sym_idx = visitArrayDecl(level, varname, vartype, i4_idx, sclass);
    } else {
      sym_idx = File()->Create_var(var_name_saved, i4_idx, 1, SYMC_FILE_STATIC,
                                   SYME_INTERNAL, SYM_CLASS_VAR);
    }
    // Do we need to store this some where?
  } else {
    AssertThat(level == 2, ("Invalid level = %d", level));
    AssertThat(tree != NULL && block_iter != NULL, ("Null visit context in visitVarDecl"));
    sclass = SYMC_AUTO;
    if (is_formal) {
      sclass = SYMC_FORMAL;
    }
    // 数组声明
    if (vartype->isArray) {
      sym_idx = visitArrayDecl(level, varname, vartype, i4_idx, sclass);
    } else {
      sym_idx = File()->Create_var(var_name_saved, i4_idx, level, sclass,
                                   SYME_INTERNAL, SYM_CLASS_VAR);
    }
  }
  // Parsing the vartype and save to ST table
  if (rhs != nullptr) {
    if (rhs->getTypeName() == "NInitializeExpr") {
      // NInitializeExpr, inito creation
      Is_Trace(level == LOCAL_SYMTAB &&
               Tracing(COMPONENT_FE, TRACE_WARN),
               (TFile, "INITO creation in LOCAL not implemented."));

      std::vector<INITV> initvs;// 一个INITO的所有INITV
      INITV initv;
      auto vals = reinterpret_cast<shared_ptr<NInitializeExpr> &>(rhs);
      if (vals->children->empty()) {
        for (auto it = vals->values->begin(); it != vals->values->end(); it++) {
          auto val  = reinterpret_cast<shared_ptr<NInteger> &>(*it);
          initv.Set_val(val->value);
          if (val->value == 0) {
            initv.Set_kind(INITVKIND_PAD);
            initv.Set_pad(4);
          }
          else {
            initv.Set_kind(INITVKIND_VAL);
          }
          initvs.push_back(initv);
        }
        // 填充未赋值的部分
        TY_IDX  ty      = ST_ty(sym_idx);
        ARB_IDX one_arb = TY_arb(ty);
        int array_size = 1;
        for (int i = 0; i < ARB_dimension(one_arb); i++) { // 计算数组大小
          array_size *= ARB_ubnd_val(one_arb + i);
        }
        if (vals->values->size() < array_size) {
          initv.Set_kind(INITVKIND_PAD);
          initv.Set_pad(4 * (array_size - vals->values->size()));
          initvs.push_back(initv);
        }
        File()->Create_inito(sym_idx, initvs, level);
      } else { // 多维数组
        for (auto it1 = vals->children->begin(); it1 != vals->children->end(); it1++) {
          auto temp  = reinterpret_cast<shared_ptr<NInitializeExpr> &>(*it1);
          for (auto it2 = temp->values->begin(); it2 != temp->values->end(); it2++) {
            auto val   = reinterpret_cast<shared_ptr<NInteger> &>(*it2);
            initv.Set_val(val->value);
            if (val->value == 0) {
              initv.Set_kind(INITVKIND_PAD);
              initv.Set_pad(4);
            }
            else {
              initv.Set_kind(INITVKIND_VAL);
            }
            initvs.push_back(initv);
          }
          // 填充未赋值的部分
          TY_IDX  ty      = ST_ty(sym_idx);
          ARB_IDX one_arb = TY_arb(ty);
          if (temp->values->size() < ARB_ubnd_val(one_arb + 1)) {
            initv.Set_kind(INITVKIND_PAD);
            initv.Set_pad(4 * (ARB_ubnd_val(one_arb + 1) - temp->values->size()));
            initvs.push_back(initv);
          }
        }
        File()->Create_inito(sym_idx, initvs, level);
      }
      return block_iter;
    } else {
      if (level <= GLOBAL_SYMTAB && rhs->getTypeName() == "NInteger") {
        auto rhs_int = reinterpret_cast<const shared_ptr<NInteger> &>(rhs);
        std::vector<INITV> initvs; // 一个INITO的所有INITV
        INITV initv;
        initv.Set_kind(INITVKIND_VAL);
        initv.Set_val(rhs_int->value);
        initvs.push_back(initv);
        File()->Create_inito(sym_idx, initvs, level);
      } else {
        AssertThat(level == LOCAL_SYMTAB, ("Incorrect level"));
        if (vartype->isConst() && rhs->getTypeName() == "NInteger") {
          INT64 val = Evaluate_const_expr(rhs);
          auto rhs_int = reinterpret_cast<const shared_ptr<NInteger> &>(rhs);
          std::vector<INITV> initvs; // 一个INITO的所有INITV
          INITV initv;
          initv.Set_kind(INITVKIND_VAL);
          initv.Set_val(rhs_int->value);
          initvs.push_back(initv);
          File()->Create_inito(sym_idx, initvs, level);
        }
        // assignment is present, create stmts to do this.
        IRNODE_IDX assignment_node = tree->Create_node(OPC_I4STID);
        tree->Get_node(assignment_node)->Set_symbol_idx(sym_idx);
        tree->Get_node(assignment_node)->Set_load_offset(0);
        IR_ITER stid_stmt = tree->Insert_stmt_to_block(block_iter,
                                                       assignment_node);
        IR_ITER rhs_expr  = visitExpression(tree, stid_stmt, level, rhs);
        AssertThat(
          rhs_expr != block_iter && rhs_expr != stid_stmt &&
          rhs_expr != nullptr,
          ("Invalid expr conversion result"));
        tree->Set_operand(stid_stmt, 0, rhs_expr);
      }
    }
  }
  return block_iter;
}

void yyerror(char *s, ...)
{

  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "Grammar checking failed at line %d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
