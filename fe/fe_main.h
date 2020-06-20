//
// Created by xc5 on 2020/6/20.
//

#ifndef OCC_FE_MAIN_H
#define OCC_FE_MAIN_H


using std::shared_ptr;
extern NBlock *programBlock;
extern int yyparse();
// extern void yyparse_init(const char* filename);
// extern void yyparse_cleanup();
//
//void createCoreFunctions(CodeGenContext& context);
void Irgen_visit(NBlock *block); // Visiting the whole function
IR_ITER visitVarDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level,
                     const shared_ptr<NVariableDeclaration> &vardecl); // Visiting a var decl

void visitFunction(const shared_ptr<NFunctionDeclaration> &func);

IR_ITER visitBlock(TREE *tree, IR_ITER parent_block, int level,
                   shared_ptr<NBlock> block);

IR_ITER visitStatement(TREE *tree, IR_ITER parent, int level,
                       shared_ptr<NStatement> &stmt);

#endif //OCC_FE_MAIN_H
