//
// Created by xc5 on 2020/6/20.
//

#ifndef OCC_FE_MAIN_H
#define OCC_FE_MAIN_H


enum FEOPCODE {
  TIDENTIFIER = 258,             /* TIDENTIFIER  */
  TINTEGER = 259,                /* TINTEGER  */
  THEX = 260,                    /* THEX  */
  TDOUBLE = 261,                 /* TDOUBLE  */
  TYINT = 262,                   /* TYINT  */
  TYDOUBLE = 263,                /* TYDOUBLE  */
  TYFLOAT = 264,                 /* TYFLOAT  */
  TYCHAR = 265,                  /* TYCHAR  */
  TYBOOL = 266,                  /* TYBOOL  */
  TYVOID = 267,                  /* TYVOID  */
  TYSTRING = 268,                /* TYSTRING  */
  TEXTERN = 269,                 /* TEXTERN  */
  TLITERAL = 270,                /* TLITERAL  */
  TCEQ = 271,                    /* TCEQ  */
  TCNE = 272,                    /* TCNE  */
  TCLT = 273,                    /* TCLT  */
  TCLE = 274,                    /* TCLE  */
  TCGT = 275,                    /* TCGT  */
  TCGE = 276,                    /* TCGE  */
  TEQUAL = 277,                  /* TEQUAL  */
  TAND = 278,                    /* TAND  */
  TOR = 279,                     /* TOR  */
  TLAND = 280,                   /* TLAND  */
  TLOR = 281,                    /* TLOR  */
  TLPAREN = 282,                 /* TLPAREN  */
  TRPAREN = 283,                 /* TRPAREN  */
  TLBRACE = 284,                 /* TLBRACE  */
  TRBRACE = 285,                 /* TRBRACE  */
  TCOMMA = 286,                  /* TCOMMA  */
  TDOT = 287,                    /* TDOT  */
  TSEMICOLON = 288,              /* TSEMICOLON  */
  TLBRACKET = 289,               /* TLBRACKET  */
  TRBRACKET = 290,               /* TRBRACKET  */
  TQUOTATION = 291,              /* TQUOTATION  */
  TPLUS = 292,                   /* TPLUS  */
  TMINUS = 293,                  /* TMINUS  */
  TMUL = 294,                    /* TMUL  */
  TDIV = 295,                    /* TDIV  */
  TXOR = 296,                    /* TXOR  */
  TMOD = 297,                    /* TMOD  */
  TNEG = 298,                    /* TNEG  */
  TNOT = 299,                    /* TNOT  */
  TANOT = 300,                   /* TANOT  */
  TSHIFTL = 301,                 /* TSHIFTL  */
  TSHIFTR = 302,                 /* TSHIFTR  */
  TIF = 303,                     /* TIF  */
  TELSE = 304,                   /* TELSE  */
  TFOR = 305,                    /* TFOR  */
  TWHILE = 306,                  /* TWHILE  */
  TRETURN = 307,                 /* TRETURN  */
  TSTRUCT = 308,                 /* TSTRUCT  *
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
