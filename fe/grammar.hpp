/* A Bison parser, made by GNU Bison 3.6.3.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_GRAMMAR_HPP_INCLUDED
# define YY_YY_GRAMMAR_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
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
    TSTRUCT = 308,                 /* TSTRUCT  */
    TCONST = 309                   /* TCONST  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 11 "grammar.y"

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

#line 131 "grammar.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_HPP_INCLUDED  */
