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

IR_ITER visitAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NAssignment> &stmt);

IR_ITER visitExpression(TREE *tree, IR_ITER parent, int level,
                        shared_ptr<NExpression> expr);

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
  } else {
    AssertThat(FALSE, ("not implemented kind of stmt = %s", stmt->getTypeName().c_str()));
  }
  return parent;
}

IR_ITER visitAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NAssignment> &stmt) {
  std::shared_ptr<NIdentifier> varname = stmt->lhs;
  shared_ptr<NExpression> rhs = stmt->rhs;

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
    AssertThat(false, ("binary operator not implemented."));
  } else if (expr->getTypeName() == "NDouble") {
    AssertThat(false, ("double not implemented."));
  } else if (expr->getTypeName() == "NInteger") {
    auto val = reinterpret_cast<shared_ptr<NInteger> &>(expr);
    IRNODE_IDX int_const_node = tree->Create_node(OPC_I4CONST);
    tree->Get_node(int_const_node)->Set_const_val(val->value);
    return tree->Insert_temp_node(int_const_node);
  }
  return parent;
}

IR_ITER visitVarDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level,
                  const shared_ptr<NVariableDeclaration>& vardecl) {
  Is_Trace(Tracing(COMPONENT_FE, TRACE_INFO),
           (TFile, "Visit Var Decl\n"));
  std::shared_ptr<NIdentifier> varname = vardecl->id;
  std::shared_ptr<NIdentifier> vartype = vardecl->type;
  std::shared_ptr<NExpression> assignment = vardecl->assignmentExpr;
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
    if (assignment != nullptr) {
      // assignment is present
      IRNODE_IDX assignment_node = tree->Create_node(OPC_I4STID);
      tree->Get_node(assignment_node)->Set_symbol_idx(sym_idx);
      tree->Get_node(assignment_node)->Set_load_offset(0);
      return tree->Insert_stmt_to_block(block_iter, assignment_node);
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
