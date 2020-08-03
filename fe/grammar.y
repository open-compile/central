%{
	#include "ASTNodes.h"
	#include "basic.h"
	#include <stdio.h>
	#define register
	NBlock* programBlock;
	extern int yylex();
	extern int yyerror(char *msg, ...);
%}
%union
{
	NBlock* block;
	NExpression* expr;
	NStatement* stmt;
	NIdentifier* ident;
	NVariableDeclaration* var_decl;
	NArrayIndex* index;
	std::vector<shared_ptr<NVariableDeclaration>>* varvec;
	std::vector<shared_ptr<NExpression>>* exprvec;
	std::string* string;
	int token;
}

%token <string> TIDENTIFIER TINTEGER THEX TDOUBLE TYINT TYDOUBLE TYFLOAT TYCHAR TYBOOL TYVOID TYSTRING TEXTERN TLITERAL
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL TAND TOR TLAND TLOR
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TSEMICOLON TLBRACKET TRBRACKET TQUOTATION
%token <token> TPLUS TMINUS TMUL TDIV TXOR TMOD TNEG TNOT TANOT TSHIFTL TSHIFTR
%token <token> TIF TELSE TFOR TWHILE TRETURN TSTRUCT TCONST

%type <index> array_index
%type <ident> ident primary_typename struct_typename typename
%type <expr> numeric expr assign init_expr
%type <varvec> func_decl_args struct_members
%type <exprvec> call_args
%type <block> program stmts block block_or_single_stmt
%type <stmt> stmt var_decl func_decl basic_stmt struct_decl if_stmt for_stmt while_stmt
%type <token> comparison and_comparison or_comparison
%type <var_decl> basic_var_decl

%left TLOR
%left ORDING_COMP
%left TLAND
%left ANDING_COMP
%left TCEQ TCNE TCLT TCLE TCGT TCGE TXOR TSHIFTL TSHIFTR
%left PREC_COMP
%left TPLUS TMINUS
%left BIN_COMP
%left TMUL TDIV TMOD
%left EAGER_BIN_COMP
%right TNOT TANOT
%left HIGHEST_BIN_COMP

%start program

%%
program : stmts { programBlock = $1; }
				;

block : TLBRACE stmts TRBRACE { $$ = $2; }
	| TLBRACE TRBRACE { $$ = new NBlock(); }
	;

stmts : stmt { $$ = new NBlock(); $$->child->push_back(shared_ptr<NStatement>($1)); }
			| stmts stmt { $1->child->push_back(shared_ptr<NStatement>($2)); }
			| stmts block {
			    shared_ptr<NExpressionStatement> blk = std::make_shared<NExpressionStatement>(shared_ptr<NExpression>($2));
			    $1->child->push_back(blk);
			}
			;
basic_stmt : var_decl TSEMICOLON { $$ = $1; }
             | expr TSEMICOLON { $$ = new NExpressionStatement(shared_ptr<NExpression>($1)); }
             | TRETURN expr TSEMICOLON { $$ = new NReturnStatement(shared_ptr<NExpression>($2)); }
             | TRETURN TSEMICOLON { $$ = new NReturnStatement(); }
             ;

stmt :  basic_stmt | func_decl | struct_decl TSEMICOLON
		 | if_stmt
		 | for_stmt
		 | while_stmt
		 ;


primary_typename : TYINT { $$ = new NIdentifier(*$1); $$->isType = true;  delete $1; }
					| TYDOUBLE { $$ = new NIdentifier(*$1); $$->isType = true; delete $1; }
					| TYFLOAT { $$ = new NIdentifier(*$1); $$->isType = true; delete $1; }
					| TYCHAR { $$ = new NIdentifier(*$1); $$->isType = true; delete $1; }
					| TYBOOL { $$ = new NIdentifier(*$1); $$->isType = true; delete $1; }
					| TYVOID { $$ = new NIdentifier(*$1); $$->isType = true; delete $1; }
					| TYSTRING { $$ = new NIdentifier(*$1); $$->isType = true; delete $1; }


struct_typename : TSTRUCT ident {
				$2->isType = true;
				$$ = $2;
			}

typename : primary_typename { $$ = $1; }
	 | struct_typename { $$ = $1; }
	 | TCONST typename { $$ = $2; $2->is_const = true; }

basic_var_decl : typename ident { $$ = new NVariableDeclaration(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($2), nullptr); }
	| basic_var_decl TLBRACKET TINTEGER TRBRACKET {
          		$1->type->arraySize->push_back(make_shared<NInteger>(atol($3->c_str())));
          		$1->type->isArray = true;
          		$$ = $1;
        }
        | basic_var_decl TLBRACKET TRBRACKET {
			$1->type->arraySize->push_back(make_shared<NIdentifier>("no-val"));
			$1->type->isArray = true;
			$$ = $1;
	}
	| basic_var_decl TLBRACKET expr TRBRACKET {
			$1->type->arraySize->push_back(shared_ptr<NExpression>($3));
			$1->type->isArray = true;
			$$ = $1;
        }
        ;

var_decl : basic_var_decl { $$ = $1; }
	 | basic_var_decl TEQUAL expr {
	 	$1->assignmentExpr = shared_ptr<NExpression>($3);
	 	$$ = $1;
	 }
	 | basic_var_decl TEQUAL TLBRACKET call_args TRBRACKET {
		 $$ = new NArrayInitialization(shared_ptr<NVariableDeclaration>($1), shared_ptr<ExpressionList>($4));
	 }
	 | basic_var_decl TEQUAL TLBRACE init_expr TRBRACE {
	 	$1->assignmentExpr = shared_ptr<NExpression>($4);
                $$ = $1;
	 }
	 ;

func_decl : typename ident TLPAREN func_decl_args TRPAREN block
				{ $$ = new NFunctionDeclaration(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($2), shared_ptr<VariableList>($4), shared_ptr<NBlock>($6));  }
			| TEXTERN typename ident TLPAREN func_decl_args TRPAREN { $$ = new NFunctionDeclaration(shared_ptr<NIdentifier>($2), shared_ptr<NIdentifier>($3), shared_ptr<VariableList>($5), nullptr, true); }

func_decl_args : /* blank */ { $$ = new VariableList(); }
							 | var_decl { $$ = new VariableList(); $$->push_back(shared_ptr<NVariableDeclaration>($<var_decl>1)); }
							 | func_decl_args TCOMMA var_decl { $1->push_back(shared_ptr<NVariableDeclaration>($<var_decl>3)); }
							 ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
			;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); }
	 | TDOUBLE { $$ = new NDouble(atof($1->c_str())); }
	 | THEX { $$ = new NInteger(strtol($1->c_str(), NULL, 16)); }
	 ;
expr : 	 ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(shared_ptr<NIdentifier>($1), shared_ptr<ExpressionList>($3)); }
		 | numeric
		 | ident { $<ident>$ = $1; }
		 | TLPAREN expr TRPAREN { $$ = $2; }
		 | array_index { $$ = $1; }
		 | ident TDOT ident { $$ = new NStructMember(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($3)); }
		 | TNOT expr %prec HIGHEST_BIN_COMP { $$ = new NUnaryOperator($1, shared_ptr<NExpression>($2)); }
		 | TANOT expr %prec HIGHEST_BIN_COMP { $$ = new NUnaryOperator($1, shared_ptr<NExpression>($2)); }
		 | TMINUS expr %prec HIGHEST_BIN_COMP { $$ = new NUnaryOperator($1, shared_ptr<NExpression>($2)); }
		 | expr TMOD expr %prec EAGER_BIN_COMP { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr TMUL expr %prec EAGER_BIN_COMP { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr TDIV expr %prec EAGER_BIN_COMP { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr TPLUS expr %prec BIN_COMP { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr TMINUS expr %prec BIN_COMP { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr comparison expr  %prec PREC_COMP
		   { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr and_comparison expr %prec ANDING_COMP
		  { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | expr or_comparison expr %prec ORDING_COMP
		  { $$ = new NBinaryOperator(shared_ptr<NExpression>($1), $2, shared_ptr<NExpression>($3)); }
		 | assign { $$ = $1; }
		 | TLITERAL { $$ = new NLiteral(*$1); delete $1; }
		 ;
init_expr :
	expr {
		$$ = new NInitializeExpr() ;
           	((NInitializeExpr *)$$)->Append(shared_ptr<NExpression>($1));
	}
	| init_expr TCOMMA init_expr {
		$$ = $1;
		((NInitializeExpr *)$$)->Append(shared_ptr<NExpression>($3));
	}
	| TLBRACE init_expr TRBRACE {
		$$ = new NInitializeExpr() ;
        	((NInitializeExpr *)$$)->Add_child(shared_ptr<NExpression>($2));
	}
	;


array_index : ident TLBRACKET expr TRBRACKET 
				{ $$ = new NArrayIndex(shared_ptr<NIdentifier>($1), shared_ptr<NExpression>($3)); }
				| array_index TLBRACKET expr TRBRACKET 
					{ 	
						$1->expressions->push_back(shared_ptr<NExpression>($3));
						$$ = $1;
					}
assign : ident TEQUAL expr { $$ = new NAssignment(shared_ptr<NIdentifier>($1), shared_ptr<NExpression>($3)); }
			| array_index TEQUAL expr {
				$$ = new NArrayAssignment(shared_ptr<NArrayIndex>($1), shared_ptr<NExpression>($3));
			}
			| ident TDOT ident TEQUAL expr {
				auto member = make_shared<NStructMember>(shared_ptr<NIdentifier>($1), shared_ptr<NIdentifier>($3)); 
				$$ = new NStructAssignment(member, shared_ptr<NExpression>($5)); 
			}
			;

call_args : /* blank */ { $$ = new ExpressionList(); }
					| expr { $$ = new ExpressionList(); $$->push_back(shared_ptr<NExpression>($1)); }
					| call_args TCOMMA expr { $1->push_back(shared_ptr<NExpression>($3)); }
comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
	   | TXOR | TSHIFTL | TSHIFTR
	   ;

and_comparison: TLAND
		;
or_comparison: TLOR
		;

block_or_single_stmt : block { $$ = $1; }
		     | basic_stmt {
		       $$ = new NBlock();
		       $$->child->push_back(shared_ptr<NStatement>($1));
		     }
		     ;

if_stmt : TIF expr block_or_single_stmt { $$ = new NIfStatement(shared_ptr<NExpression>($2), shared_ptr<NBlock>($3)); }
		| TIF expr block_or_single_stmt TELSE block_or_single_stmt { $$ = new NIfStatement(shared_ptr<NExpression>($2), shared_ptr<NBlock>($3), shared_ptr<NBlock>($5)); }
		| TIF expr block_or_single_stmt TELSE if_stmt {
			auto blk = new NBlock(); 
			blk->child->push_back(shared_ptr<NStatement>($5));
			$$ = new NIfStatement(shared_ptr<NExpression>($2), shared_ptr<NBlock>($3), shared_ptr<NBlock>(blk)); 
		}

for_stmt : TFOR TLPAREN expr TSEMICOLON expr TSEMICOLON expr TRPAREN block { $$ = new NForStatement(shared_ptr<NBlock>($9), shared_ptr<NExpression>($3), shared_ptr<NExpression>($5), shared_ptr<NExpression>($7)); }
		
while_stmt : TWHILE TLPAREN expr TRPAREN block { $$ = new NForStatement(shared_ptr<NBlock>($5), nullptr, shared_ptr<NExpression>($3), nullptr); }

struct_decl : TSTRUCT ident TLBRACE struct_members TRBRACE {$$ = new NStructDeclaration(shared_ptr<NIdentifier>($2), shared_ptr<VariableList>($4)); }

struct_members : /* blank */ { $$ = new VariableList(); }
				| var_decl { $$ = new VariableList(); $$->push_back(shared_ptr<NVariableDeclaration>($<var_decl>1)); }
				| struct_members var_decl { $1->push_back(shared_ptr<NVariableDeclaration>($<var_decl>2)); }

%%
