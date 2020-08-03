/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TIDENTIFIER = 258,
     TINTEGER = 259,
     THEX = 260,
     TDOUBLE = 261,
     TYINT = 262,
     TYDOUBLE = 263,
     TYFLOAT = 264,
     TYCHAR = 265,
     TYBOOL = 266,
     TYVOID = 267,
     TYSTRING = 268,
     TEXTERN = 269,
     TLITERAL = 270,
     TCEQ = 271,
     TCNE = 272,
     TCLT = 273,
     TCLE = 274,
     TCGT = 275,
     TCGE = 276,
     TEQUAL = 277,
     TAND = 278,
     TOR = 279,
     TLAND = 280,
     TLOR = 281,
     TLPAREN = 282,
     TRPAREN = 283,
     TLBRACE = 284,
     TRBRACE = 285,
     TCOMMA = 286,
     TDOT = 287,
     TSEMICOLON = 288,
     TLBRACKET = 289,
     TRBRACKET = 290,
     TQUOTATION = 291,
     TPLUS = 292,
     TMINUS = 293,
     TMUL = 294,
     TDIV = 295,
     TXOR = 296,
     TMOD = 297,
     TNEG = 298,
     TNOT = 299,
     TANOT = 300,
     TSHIFTL = 301,
     TSHIFTR = 302,
     TIF = 303,
     TELSE = 304,
     TFOR = 305,
     TWHILE = 306,
     TRETURN = 307,
     TSTRUCT = 308,
     TCONST = 309,
     ORDING_COMP = 310,
     ANDING_COMP = 311,
     PREC_COMP = 312,
     BIN_COMP = 313,
     EAGER_BIN_COMP = 314,
     HIGHEST_BIN_COMP = 315
   };
#endif
/* Tokens.  */
#define TIDENTIFIER 258
#define TINTEGER 259
#define THEX 260
#define TDOUBLE 261
#define TYINT 262
#define TYDOUBLE 263
#define TYFLOAT 264
#define TYCHAR 265
#define TYBOOL 266
#define TYVOID 267
#define TYSTRING 268
#define TEXTERN 269
#define TLITERAL 270
#define TCEQ 271
#define TCNE 272
#define TCLT 273
#define TCLE 274
#define TCGT 275
#define TCGE 276
#define TEQUAL 277
#define TAND 278
#define TOR 279
#define TLAND 280
#define TLOR 281
#define TLPAREN 282
#define TRPAREN 283
#define TLBRACE 284
#define TRBRACE 285
#define TCOMMA 286
#define TDOT 287
#define TSEMICOLON 288
#define TLBRACKET 289
#define TRBRACKET 290
#define TQUOTATION 291
#define TPLUS 292
#define TMINUS 293
#define TMUL 294
#define TDIV 295
#define TXOR 296
#define TMOD 297
#define TNEG 298
#define TNOT 299
#define TANOT 300
#define TSHIFTL 301
#define TSHIFTR 302
#define TIF 303
#define TELSE 304
#define TFOR 305
#define TWHILE 306
#define TRETURN 307
#define TSTRUCT 308
#define TCONST 309
#define ORDING_COMP 310
#define ANDING_COMP 311
#define PREC_COMP 312
#define BIN_COMP 313
#define EAGER_BIN_COMP 314
#define HIGHEST_BIN_COMP 315




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 11 "grammar.y"
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
/* Line 1529 of yacc.c.  */
#line 182 "grammar.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

