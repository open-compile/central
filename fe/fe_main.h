//
// Created by xc5 on 2020/6/20.
//

#ifndef OCC_FE_MAIN_H
#define OCC_FE_MAIN_H


enum FEOPCODE {
  TCEQ = 270,                    /* TCEQ  */
  TCNE = 271,                    /* TCNE  */
  TCLT = 272,                    /* TCLT  */
  TCLE = 273,                    /* TCLE  */
  TCGT = 274,                    /* TCGT  */
  TCGE = 275,                    /* TCGE  */
  TEQUAL = 276,                  /* TEQUAL  */
  TAND = 277,                    /* TAND  */
  TOR = 278,                     /* TOR  */
  TLAND = 279,                   /* TLAND  */
  TLOR = 280,                    /* TLOR  */
  TLPAREN = 281,                 /* TLPAREN  */
  TRPAREN = 282,                 /* TRPAREN  */
  TLBRACE = 283,                 /* TLBRACE  */
  TRBRACE = 284,                 /* TRBRACE  */
  TCOMMA = 285,                  /* TCOMMA  */
  TDOT = 286,                    /* TDOT  */
  TSEMICOLON = 287,              /* TSEMICOLON  */
  TLBRACKET = 288,               /* TLBRACKET  */
  TRBRACKET = 289,               /* TRBRACKET  */
  TQUOTATION = 290,              /* TQUOTATION  */
  TPLUS = 291,                   /* TPLUS  */
  TMINUS = 292,                  /* TMINUS  */
  TMUL = 293,                    /* TMUL  */
  TDIV = 294,                    /* TDIV  */
  TXOR = 295,                    /* TXOR  */
  TMOD = 296,                    /* TMOD  */
  TNEG = 297,                    /* TNEG  */
  TNOT = 298,                    /* TNOT  */
  TANOT = 299,                   /* TANOT  */
  TSHIFTL = 300,                 /* TSHIFTL  */
  TSHIFTR = 301,                 /* TSHIFTR  */
  TIF = 302,                     /* TIF  */
  TELSE = 303,                   /* TELSE  */
  TFOR = 304,                    /* TFOR  */
  TWHILE = 305,                  /* TWHILE  */
  TRETURN = 306,                 /* TRETURN  */
  TSTRUCT = 307,                 /* TSTRUCT  */
  TCONST = 308                   /* TCONST  */
};

typedef struct BIN_OP_TO_OPCODE_T {
  const char *_name;
  FEOPCODE    _fe_opcode;
  OPERATOR    _irnode_opcode;
  MTYPE_ID    _rtype;
} BIN_OP_TO_OPR;

using std::shared_ptr;
extern NBlock *programBlock;
extern int yyparse();
// extern void yyparse_init(const char* filename);
// extern void yyparse_cleanup();
//
//void createCoreFunctions(CodeGenContext& context);
void Irgen_visit(NBlock *block); // Visiting the whole function
IR_ITER visitVarDecl(TREE *tree, const IR_ITER &block_iter, UINT32 level, BOOL is_formal,
                     const shared_ptr<NVariableDeclaration> &vardecl); // Visiting a var decl

void visitFunction(const shared_ptr<NFunctionDeclaration> &func);

IR_ITER visitBlock(TREE *tree, IR_ITER parent_block, int level,
                   shared_ptr<NBlock> block);

IR_ITER visitStatement(TREE *tree, IR_ITER parent, int level,
                       shared_ptr<NStatement> &stmt);

IR_ITER visitAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NAssignment> &stmt);

IR_ITER visitArrayAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                                 const shared_ptr<NArrayAssignment> &stmt);

IR_ITER visitIfStmt(TREE *tree, IR_ITER parent, int level,
                    const shared_ptr<NIfStatement> &stmt);

IR_ITER visitForStmt(TREE *tree, IR_ITER parent, int level,
                    const shared_ptr<NForStatement> &stmt);

IR_ITER visitReturnStmt(TREE *tree, IR_ITER parent, int level,
                        const shared_ptr<NReturnStatement> &stmt);

IR_ITER visitMethodCall(TREE *tree, IR_ITER parent, int level, BOOL is_expr,
                        const shared_ptr<NMethodCall> &stmt);

IR_ITER visitExpression(TREE *tree, IR_ITER parent, int level,
                        shared_ptr<NExpression> expr);

IR_ITER visitIdentifierStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NIdentifier> &stmt);

OPERATOR Get_op_by_token(FEOPCODE op);
MTYPE_ID Get_rtype_by_token(FEOPCODE op);

#endif //OCC_FE_MAIN_H
