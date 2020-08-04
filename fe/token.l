%{
#include <stdio.h>
#include <string>
#include "ASTNodes.h"
#include "grammar.hpp"
#define SAVE_TOKEN yylval.string = new string(yytext)
#define TOKEN(t) ( yylval.token = t)
#define Debug_puts(x)

static FILE* yyparse_file_ptr;

// void yyparse_init(const char* filename)
// {
// 	yyparse_file_ptr = fopen(filename, "r");
// 	yyin = yyparse_file_ptr;
// }

// void yyparse_cleanup()
// {
// 	fclose(yyparse_file_ptr);
// }

%}

%option noyywrap

%x C_COMMENT CL_COMMENT

%%
"#".*                   ;
[ \t\r\n]				;
"if"                    Debug_puts("TIF"); return TOKEN(TIF);
"else"                  Debug_puts("TELSE"); return TOKEN(TELSE);
"return"                Debug_puts("TRETURN"); return TOKEN(TRETURN);
"for"                   Debug_puts("TFOR"); return TOKEN(TFOR);
"while"                 Debug_puts("TWHILE"); return TOKEN(TWHILE);
"struct"                Debug_puts("TSTRUCT"); return TOKEN(TSTRUCT);
"int"                   SAVE_TOKEN; Debug_puts("TYINT");  return TYINT;
"double"                SAVE_TOKEN; Debug_puts("TYDOUBLE"); return TYDOUBLE;
"float"                 SAVE_TOKEN; Debug_puts("TYFLOAT"); return TYFLOAT;
"char"                  SAVE_TOKEN; Debug_puts("TYCHAR"); return TYCHAR;
"bool"                  SAVE_TOKEN; Debug_puts("TYBOOL"); return TYBOOL;
"stringsplus"           SAVE_TOKEN; Debug_puts("TYSTRING"); return TYSTRING;
"void"                  SAVE_TOKEN; Debug_puts("TYVOID"); return TYVOID;
"const"                 SAVE_TOKEN; Debug_puts("TRBRACE"); return TCONST;
"extern"                SAVE_TOKEN; Debug_puts("TEXTERN"); return TEXTERN;
[a-zA-Z_][a-zA-Z0-9_]*	SAVE_TOKEN; Debug_puts("TIDENTIFIER"); return TIDENTIFIER;
0[xX][0-9a-fA-F]+	    SAVE_TOKEN; Debug_puts("THEX"); return THEX;
[0-9]+\.[0-9]*			SAVE_TOKEN; Debug_puts("TDOUBLE"); return TDOUBLE;
[0-9]+  				SAVE_TOKEN; Debug_puts("TINTEGER"); return TINTEGER;
\"(\\.|[^"])*\"         SAVE_TOKEN; Debug_puts("TLITERAL"); return TLITERAL;
"="						Debug_puts("TEQUAL"); return TOKEN(TEQUAL);
"=="					Debug_puts("TCEQ"); return TOKEN(TCEQ);
"!="                    Debug_puts("TCNE"); return TOKEN(TCNE);
"<"                     Debug_puts("TCLT"); return TOKEN(TCLT);
"<="                    Debug_puts("TCLE"); return TOKEN(TCLE);
">"                     Debug_puts("TCGT"); return TOKEN(TCGT);
">="                    Debug_puts("TCGE"); return TOKEN(TCGE);
"("                     Debug_puts("TLPAREN"); return TOKEN(TLPAREN);
")"                     Debug_puts("TRPAREN"); return TOKEN(TRPAREN);
"{"                     Debug_puts("TLBRACE"); return TOKEN(TLBRACE);
"}"                     Debug_puts("TRBRACE"); return TOKEN(TRBRACE);
"["                     Debug_puts("TLBRACKET"); return TOKEN(TLBRACKET);
"]"                     Debug_puts("TRBRACKET"); return TOKEN(TRBRACKET);
"."                     Debug_puts("TDOT"); return TOKEN(TDOT);
","                     Debug_puts("TCOMMA"); return TOKEN(TCOMMA);
"+"                     Debug_puts("TPLUS"); return TOKEN(TPLUS);
"-"                     Debug_puts("TMINUS"); return TOKEN(TMINUS);
"*"                     Debug_puts("TMUL"); return TOKEN(TMUL);
"/"                     Debug_puts("TDIV"); return TOKEN(TDIV);
"&"                     Debug_puts("TAND"); return TOKEN(TAND);
"~"                     Debug_puts("TANOT"); return TOKEN(TANOT);
"!"                     Debug_puts("TNOT"); return TOKEN(TNOT);
"|"                     Debug_puts("TOR"); return TOKEN(TOR);
"||"                    Debug_puts("TLOR"); return TOKEN(TLOR);
"&&"                    Debug_puts("TLAND"); return TOKEN(TLAND);
"^"                     Debug_puts("TXOR"); return TOKEN(TXOR);
"%"                     Debug_puts("TMOD"); return TOKEN(TMOD);
">>"                    Debug_puts("TSHIFTR"); return TOKEN(TSHIFTR);
"<<"                    Debug_puts("TSHIFTL"); return TOKEN(TSHIFTL);
";"                     Debug_puts("TSEMICOLON"); return TOKEN(TSEMICOLON);
.						printf("Unknown token:%s\n", yytext); yyterminate();

"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>\n   { yylineno++; }
<C_COMMENT>.    { }

"//"            { BEGIN(CL_COMMENT); }
<CL_COMMENT>\n  { BEGIN(INITIAL); yylineno++; }
<CL_COMMENT>.   { }

%%

