//
// Created by xc5 on 2020/6/20.
//

#ifndef OCC_FE_MAIN_H
#define OCC_FE_MAIN_H


enum FEOPCODE {
  TIDENTIFIER = 258,             /* TIDENTIFIER  */
  TINTEGER = 259,                /* TINTEGER  */
  TDOUBLE = 260,                 /* TDOUBLE  */
  TYINT = 261,                   /* TYINT  */
  TYDOUBLE = 262,                /* TYDOUBLE  */
  TYFLOAT = 263,                 /* TYFLOAT  */
  TYCHAR = 264,                  /* TYCHAR  */
  TYBOOL = 265,                  /* TYBOOL  */
  TYVOID = 266,                  /* TYVOID  */
  TYSTRING = 267,                /* TYSTRING  */
  TEXTERN = 268,                 /* TEXTERN  */
  TLITERAL = 269,                /* TLITERAL  */
  TCEQ = 270,                    /* TCEQ  */
  TCNE = 271,                    /* TCNE  */
  TCLT = 272,                    /* TCLT  */
  TCLE = 273,                    /* TCLE  */
  TCGT = 274,                    /* TCGT  */
  TCGE = 275,                    /* TCGE  */
  TEQUAL = 276,                  /* TEQUAL  */
  TLPAREN = 277,                 /* TLPAREN  */
  TRPAREN = 278,                 /* TRPAREN  */
  TLBRACE = 279,                 /* TLBRACE  */
  TRBRACE = 280,                 /* TRBRACE  */
  TCOMMA = 281,                  /* TCOMMA  */
  TDOT = 282,                    /* TDOT  */
  TSEMICOLON = 283,              /* TSEMICOLON  */
  TLBRACKET = 284,               /* TLBRACKET  */
  TRBRACKET = 285,               /* TRBRACKET  */
  TQUOTATION = 286,              /* TQUOTATION  */
  TPLUS = 287,                   /* TPLUS  */
  TMINUS = 288,                  /* TMINUS  */
  TMUL = 289,                    /* TMUL  */
  TDIV = 290,                    /* TDIV  */
  TAND = 291,                    /* TAND  */
  TOR = 292,                     /* TOR  */
  TXOR = 293,                    /* TXOR  */
  TMOD = 294,                    /* TMOD  */
  TNEG = 295,                    /* TNEG  */
  TNOT = 296,                    /* TNOT  */
  TSHIFTL = 297,                 /* TSHIFTL  */
  TSHIFTR = 298,                 /* TSHIFTR  */
  TIF = 299,                     /* TIF  */
  TELSE = 300,                   /* TELSE  */
  TFOR = 301,                    /* TFOR  */
  TWHILE = 302,                  /* TWHILE  */
  TRETURN = 303,                 /* TRETURN  */
  TSTRUCT = 304                  /* TSTRUCT  */
};

typedef struct BIN_OP_TO_OPCODE_T {
  const char *_name;
  FEOPCODE    _fe_opcode;
  OPERATOR    _irnode_opcode;
} BIN_OP_TO_OPR;

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

IR_ITER visitAssignmentStmt(TREE *tree, IR_ITER parent, int level,
                            const shared_ptr<NAssignment> &stmt);

IR_ITER visitIfStmt(TREE *tree, IR_ITER parent, int level,
                    const shared_ptr<NIfStatement> &stmt);

IR_ITER visitExpression(TREE *tree, IR_ITER parent, int level,
                        shared_ptr<NExpression> expr);

OPERATOR Get_op_by_token(FEOPCODE op);

#endif //OCC_FE_MAIN_H
