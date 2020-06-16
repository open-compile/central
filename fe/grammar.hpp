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
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 10 "grammar.y"

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

#line 126 "grammar.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_HPP_INCLUDED  */
