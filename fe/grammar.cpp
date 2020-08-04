/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 1 "grammar.y"

	#include "ASTNodes.h"
	#include "basic.h"
	#include <stdio.h>
	#define register
	NBlock* programBlock;
	extern int yylex();
	extern int yyerror(char *msg, ...);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

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
/* Line 193 of yacc.c.  */
#line 239 "grammar.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 252 "grammar.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  56
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   943

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  61
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  31
/* YYNRULES -- Number of rules.  */
#define YYNRULES  98
/* YYNRULES -- Number of states.  */
#define YYNSTATES  174

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   315

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    12,    14,    17,    20,    23,
      26,    30,    33,    35,    37,    40,    42,    44,    46,    48,
      50,    52,    54,    56,    58,    60,    63,    65,    67,    69,
      73,    78,    80,    84,    89,    95,    98,   101,   105,   108,
     115,   122,   123,   125,   129,   131,   133,   135,   137,   142,
     144,   146,   150,   152,   156,   159,   162,   165,   169,   173,
     177,   181,   185,   189,   193,   197,   199,   201,   203,   207,
     211,   216,   221,   225,   229,   235,   236,   238,   242,   244,
     246,   248,   250,   252,   254,   256,   258,   260,   262,   264,
     266,   268,   272,   278,   288,   294,   300,   301,   303
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      62,     0,    -1,    64,    -1,    29,    64,    30,    -1,    29,
      30,    -1,    66,    -1,    64,    66,    -1,    64,    63,    -1,
      72,    33,    -1,    78,    33,    -1,    52,    78,    33,    -1,
      52,    33,    -1,    65,    -1,    74,    -1,    90,    33,    -1,
      87,    -1,    88,    -1,    89,    -1,     7,    -1,     8,    -1,
       9,    -1,    10,    -1,    11,    -1,    12,    -1,    13,    -1,
      53,    76,    -1,    67,    -1,    68,    -1,    76,    -1,    71,
      34,    35,    -1,    71,    34,    78,    35,    -1,    70,    -1,
      71,    22,    78,    -1,    71,    22,    29,    30,    -1,    71,
      22,    29,    79,    30,    -1,    69,    71,    -1,    54,    72,
      -1,    72,    31,    71,    -1,    69,    70,    -1,    69,    76,
      27,    75,    28,    63,    -1,    14,    69,    76,    27,    75,
      28,    -1,    -1,    73,    -1,    75,    31,    73,    -1,     3,
      -1,     4,    -1,     6,    -1,     5,    -1,    76,    27,    82,
      28,    -1,    77,    -1,    76,    -1,    27,    78,    28,    -1,
      80,    -1,    76,    32,    76,    -1,    44,    78,    -1,    45,
      78,    -1,    38,    78,    -1,    78,    42,    78,    -1,    78,
      39,    78,    -1,    78,    40,    78,    -1,    78,    37,    78,
      -1,    78,    38,    78,    -1,    78,    83,    78,    -1,    78,
      84,    78,    -1,    78,    85,    78,    -1,    81,    -1,    15,
      -1,    78,    -1,    79,    31,    79,    -1,    29,    79,    30,
      -1,    76,    34,    78,    35,    -1,    80,    34,    78,    35,
      -1,    76,    22,    78,    -1,    80,    22,    78,    -1,    76,
      32,    76,    22,    78,    -1,    -1,    78,    -1,    82,    31,
      78,    -1,    16,    -1,    17,    -1,    18,    -1,    19,    -1,
      20,    -1,    21,    -1,    41,    -1,    46,    -1,    47,    -1,
      25,    -1,    26,    -1,    63,    -1,    66,    -1,    48,    78,
      86,    -1,    48,    78,    86,    49,    86,    -1,    50,    27,
      78,    33,    78,    33,    78,    28,    63,    -1,    51,    27,
      78,    28,    63,    -1,    53,    76,    29,    91,    30,    -1,
      -1,    72,    -1,    91,    72,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    56,    56,    59,    60,    63,    64,    65,    70,    71,
      72,    73,    76,    76,    76,    77,    78,    79,    83,    84,
      85,    86,    87,    88,    89,    92,    97,    98,   100,   108,
     113,   120,   121,   125,   129,   136,   141,   145,   151,   156,
     158,   160,   161,   162,   165,   168,   169,   170,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   188,   190,   192,   193,   196,   200,   204,
     211,   213,   218,   219,   222,   228,   229,   230,   231,   231,
     231,   231,   231,   231,   232,   232,   232,   235,   237,   240,
     241,   247,   248,   251,   253,   255,   257,   258,   259
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TIDENTIFIER", "TINTEGER", "THEX",
  "TDOUBLE", "TYINT", "TYDOUBLE", "TYFLOAT", "TYCHAR", "TYBOOL", "TYVOID",
  "TYSTRING", "TEXTERN", "TLITERAL", "TCEQ", "TCNE", "TCLT", "TCLE",
  "TCGT", "TCGE", "TEQUAL", "TAND", "TOR", "TLAND", "TLOR", "TLPAREN",
  "TRPAREN", "TLBRACE", "TRBRACE", "TCOMMA", "TDOT", "TSEMICOLON",
  "TLBRACKET", "TRBRACKET", "TQUOTATION", "TPLUS", "TMINUS", "TMUL",
  "TDIV", "TXOR", "TMOD", "TNEG", "TNOT", "TANOT", "TSHIFTL", "TSHIFTR",
  "TIF", "TELSE", "TFOR", "TWHILE", "TRETURN", "TSTRUCT", "TCONST",
  "ORDING_COMP", "ANDING_COMP", "PREC_COMP", "BIN_COMP", "EAGER_BIN_COMP",
  "HIGHEST_BIN_COMP", "$accept", "program", "block", "stmts", "basic_stmt",
  "stmt", "primary_typename", "struct_typename", "typename",
  "basic_non_init_def", "basic_var_def", "var_decl", "single_var_decl",
  "func_decl", "func_decl_args", "ident", "numeric", "expr", "init_expr",
  "array_index", "assign", "call_args", "comparison", "and_comparison",
  "or_comparison", "block_or_single_stmt", "if_stmt", "for_stmt",
  "while_stmt", "struct_decl", "struct_members", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    61,    62,    63,    63,    64,    64,    64,    65,    65,
      65,    65,    66,    66,    66,    66,    66,    66,    67,    67,
      67,    67,    67,    67,    67,    68,    69,    69,    70,    70,
      70,    71,    71,    71,    71,    72,    72,    72,    73,    74,
      74,    75,    75,    75,    76,    77,    77,    77,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    79,    79,    79,
      80,    80,    81,    81,    81,    82,    82,    82,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    84,    85,    86,
      86,    87,    87,    88,    89,    90,    91,    91,    91
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     2,     1,     2,     2,     2,     2,
       3,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     3,
       4,     1,     3,     4,     5,     2,     2,     3,     2,     6,
       6,     0,     1,     3,     1,     1,     1,     1,     4,     1,
       1,     3,     1,     3,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     3,     3,
       4,     4,     3,     3,     5,     0,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     5,     9,     5,     5,     0,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    44,    45,    47,    46,    18,    19,    20,    21,    22,
      23,    24,     0,    66,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,    12,     5,    26,    27,
       0,     0,    13,    50,    49,     0,    52,    65,    15,    16,
      17,     0,     0,     0,     0,    56,    54,    55,     0,     0,
       0,    11,     0,    25,     0,    36,     1,     0,     7,     6,
      31,    35,    28,     0,     8,     0,    75,     0,     0,    78,
      79,    80,    81,    82,    83,    87,    88,     9,     0,     0,
       0,     0,    84,     0,    85,    86,     0,     0,     0,     0,
       0,    14,    25,     0,    51,     0,    89,    90,    91,     0,
       0,    10,    96,    28,     4,     0,     0,     0,    41,    37,
      72,    76,     0,    53,     0,    60,    61,    58,    59,    57,
      62,    63,    64,    73,     0,    41,    61,     0,     0,     0,
      97,     0,     3,     0,    32,    29,     0,     0,    42,     0,
      48,     0,     0,    70,    71,     0,    92,     0,    94,    95,
      98,     0,    33,    67,     0,    30,    31,     0,     0,     0,
      77,    74,    40,     0,     0,    34,     0,    39,    43,     0,
      69,    68,     0,    93
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    24,    58,    25,    26,    27,    28,    29,    30,    60,
      61,    31,   138,    32,   139,    33,    34,    35,   154,    36,
      37,   112,    86,    87,    88,    98,    38,    39,    40,    41,
     131
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -140
static const yytype_int16 yypact[] =
{
     363,  -140,  -140,  -140,  -140,  -140,  -140,  -140,  -140,  -140,
    -140,  -140,    31,  -140,   471,   471,   471,   471,   471,   -13,
     -12,   419,     8,   132,    24,   259,  -140,  -140,  -140,  -140,
       8,   -11,  -140,    -4,  -140,   501,    -5,  -140,  -140,  -140,
    -140,    14,     8,     8,   533,  -140,  -140,  -140,   155,   471,
     471,  -140,   565,    20,     8,    22,  -140,   311,  -140,  -140,
    -140,    -3,    43,     8,  -140,   471,   471,     8,   471,  -140,
    -140,  -140,  -140,  -140,  -140,  -140,  -140,  -140,   471,   471,
     471,   471,  -140,   471,  -140,  -140,   471,   471,   471,   471,
     471,  -140,  -140,    52,  -140,   471,  -140,  -140,    39,   597,
     629,  -140,   132,  -140,  -140,   207,   427,   435,    31,    -3,
     853,   853,    17,    68,   661,    57,    57,  -140,  -140,  -140,
      70,   896,   864,   853,   693,    31,   725,   259,   471,    62,
      22,    48,  -140,   391,   853,  -140,   757,     8,  -140,    40,
    -140,   471,   471,  -140,  -140,    49,  -140,   789,  -140,  -140,
      22,   463,  -140,   853,     3,  -140,    55,    -3,    62,    31,
     853,   853,  -140,   471,    32,  -140,   463,  -140,  -140,   821,
    -140,    63,    62,  -140
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -140,  -140,   -42,    41,  -140,   -16,  -140,  -140,    -2,   -34,
     -55,    -7,   -54,  -140,   -21,   -17,  -140,   -14,  -139,  -140,
    -140,  -140,  -140,  -140,  -140,   -10,  -140,  -140,  -140,  -140,
    -140
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -57
static const yytype_int16 yytable[] =
{
      44,    45,    46,    47,    48,    53,    96,    52,   109,    59,
      43,     1,   164,    62,    49,    50,    55,    89,    65,   106,
      63,    54,    64,    66,    56,    92,    93,   171,    67,    90,
      68,   107,    97,   165,   166,    99,   100,   103,     5,     6,
       7,     8,     9,    10,    11,   140,   103,    91,   141,   102,
     113,   110,   111,    63,   114,     5,     6,     7,     8,     9,
      10,    11,   170,   166,   115,   116,   117,   118,   158,   119,
     108,   159,   120,   121,   122,   123,   124,   162,   149,   125,
     159,   126,   157,   -38,    42,    96,   -38,   148,   127,    59,
     142,    57,   134,   136,   166,   130,    80,    81,   105,    83,
      54,    42,    23,   156,   145,   168,   137,    78,    79,    80,
      81,    97,    83,     0,   147,     0,   167,   146,     0,   153,
     103,     0,     0,   137,   150,     0,     0,   160,   161,    54,
     173,     0,     0,     0,     0,     0,     0,   153,     0,     5,
       6,     7,     8,     9,    10,    11,     0,     0,     0,   169,
       0,     0,   153,     0,     0,     0,     0,   137,     1,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    69,    70,    71,    72,    73,    74,     0,     0,     0,
      75,    76,    14,     0,    57,    42,    23,     0,     0,     0,
       0,     0,    78,    95,    80,    81,    82,    83,     0,    16,
      17,    84,    85,    18,     0,    19,    20,    21,    22,    23,
       1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,    57,   132,     0,     0,
       0,     0,     0,     0,     0,    15,     0,     0,     0,     0,
       0,    16,    17,     0,     0,    18,     0,    19,    20,    21,
      22,    23,     1,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    57,     0,
       0,     0,     0,     0,     0,     0,     0,    15,     0,     0,
       0,     0,     0,    16,    17,     0,     0,    18,     0,    19,
      20,    21,    22,    23,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
       0,   104,     0,     0,     0,     0,     0,     0,     0,    15,
       0,     0,     0,     0,     0,    16,    17,     0,     0,    18,
       0,    19,    20,    21,    22,    23,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,     0,     0,     1,     2,     3,     4,     0,     0,
       0,    15,     0,     0,     0,     0,    13,    16,    17,     0,
       0,    18,     0,    19,    20,    21,    22,    23,    14,     0,
     151,   152,     1,     2,     3,     4,     0,     0,     0,    15,
       1,     2,     3,     4,    13,    16,    17,     0,     1,     2,
       3,     4,    13,     0,     0,     0,    14,     0,     0,     0,
      13,     0,    51,     0,    14,     0,   133,    15,     0,     0,
       0,     0,    14,    16,    17,    15,     1,     2,     3,     4,
     135,    16,    17,    15,     1,     2,     3,     4,    13,    16,
      17,     0,     0,     0,     0,     0,    13,     0,     0,     0,
      14,     0,   151,     0,     0,     0,     0,     0,    14,     0,
       0,    15,     0,     0,     0,     0,     0,    16,    17,    15,
       0,     0,     0,     0,     0,    16,    17,    69,    70,    71,
      72,    73,    74,     0,     0,     0,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,    78,    79,
      80,    81,    82,    83,     0,     0,     0,    84,    85,    69,
      70,    71,    72,    73,    74,     0,     0,     0,    75,    76,
       0,    94,     0,     0,     0,     0,     0,     0,     0,     0,
      78,    79,    80,    81,    82,    83,     0,     0,     0,    84,
      85,    69,    70,    71,    72,    73,    74,     0,     0,     0,
      75,    76,     0,     0,     0,     0,     0,     0,   101,     0,
       0,     0,    78,    79,    80,    81,    82,    83,     0,     0,
       0,    84,    85,    69,    70,    71,    72,    73,    74,     0,
       0,     0,    75,    76,     0,     0,     0,     0,     0,     0,
     128,     0,     0,     0,    78,    79,    80,    81,    82,    83,
       0,     0,     0,    84,    85,    69,    70,    71,    72,    73,
      74,     0,     0,     0,    75,    76,     0,   129,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    79,    80,    81,
      82,    83,     0,     0,     0,    84,    85,    69,    70,    71,
      72,    73,    74,     0,     0,     0,    75,    76,     0,     0,
       0,     0,     0,     0,     0,     0,   143,     0,    78,    79,
      80,    81,    82,    83,     0,     0,     0,    84,    85,    69,
      70,    71,    72,    73,    74,     0,     0,     0,    75,    76,
       0,     0,     0,     0,     0,     0,     0,     0,   144,     0,
      78,    79,    80,    81,    82,    83,     0,     0,     0,    84,
      85,   -56,   -56,   -56,   -56,   -56,   -56,     0,     0,     0,
     -56,   -56,     0,     0,     0,     0,     0,     0,   -56,     0,
       0,     0,   -56,   -56,   -56,   -56,   -56,   -56,     0,     0,
       0,   -56,   -56,    69,    70,    71,    72,    73,    74,     0,
       0,     0,    75,    76,     0,     0,     0,     0,     0,     0,
       0,     0,   155,     0,    78,    79,    80,    81,    82,    83,
       0,     0,     0,    84,    85,    69,    70,    71,    72,    73,
      74,     0,     0,     0,    75,    76,     0,     0,     0,     0,
       0,     0,   163,     0,     0,     0,    78,    79,    80,    81,
      82,    83,     0,     0,     0,    84,    85,    69,    70,    71,
      72,    73,    74,     0,     0,     0,    75,    76,     0,   172,
       0,     0,     0,     0,     0,     0,     0,     0,    78,    79,
      80,    81,    82,    83,     0,     0,     0,    84,    85,    69,
      70,    71,    72,    73,    74,     0,     0,     0,    75,    76,
      69,    70,    71,    72,    73,    74,     0,     0,     0,    75,
      78,    79,    80,    81,    82,    83,     0,     0,     0,    84,
      85,    78,    79,    80,    81,    82,    83,     0,     0,     0,
      84,    85,    69,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    78,    79,    80,    81,    82,    83,     0,
       0,     0,    84,    85
};

static const yytype_int16 yycheck[] =
{
      14,    15,    16,    17,    18,    22,    48,    21,    63,    25,
      12,     3,   151,    30,    27,    27,    23,    22,    22,    22,
      31,    23,    33,    27,     0,    42,    43,   166,    32,    34,
      34,    34,    48,    30,    31,    49,    50,    54,     7,     8,
       9,    10,    11,    12,    13,    28,    63,    33,    31,    29,
      67,    65,    66,    31,    68,     7,     8,     9,    10,    11,
      12,    13,    30,    31,    78,    79,    80,    81,    28,    83,
      27,    31,    86,    87,    88,    89,    90,    28,    30,    27,
      31,    95,   137,    28,    53,   127,    31,   129,    49,   105,
      22,    29,   106,   107,    31,   102,    39,    40,    57,    42,
     102,    53,    54,   137,   125,   159,   108,    37,    38,    39,
      40,   127,    42,    -1,   128,    -1,   158,   127,    -1,   133,
     137,    -1,    -1,   125,   131,    -1,    -1,   141,   142,   131,
     172,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,   159,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    -1,
      25,    26,    27,    -1,    29,    53,    54,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    -1,    44,
      45,    46,    47,    48,    -1,    50,    51,    52,    53,    54,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    29,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    15,    44,    45,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    27,    -1,
      29,    30,     3,     4,     5,     6,    -1,    -1,    -1,    38,
       3,     4,     5,     6,    15,    44,    45,    -1,     3,     4,
       5,     6,    15,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      15,    -1,    33,    -1,    27,    -1,    29,    38,    -1,    -1,
      -1,    -1,    27,    44,    45,    38,     3,     4,     5,     6,
      35,    44,    45,    38,     3,     4,     5,     6,    15,    44,
      45,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,    38,
      -1,    -1,    -1,    -1,    -1,    44,    45,    16,    17,    18,
      19,    20,    21,    -1,    -1,    -1,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    46,    47,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    25,    26,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    46,
      47,    16,    17,    18,    19,    20,    21,    -1,    -1,    -1,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    46,    47,    16,    17,    18,    19,    20,    21,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    46,    47,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    25,    26,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    46,    47,    16,    17,    18,
      19,    20,    21,    -1,    -1,    -1,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    37,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    46,    47,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    46,
      47,    16,    17,    18,    19,    20,    21,    -1,    -1,    -1,
      25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    -1,    -1,
      -1,    46,    47,    16,    17,    18,    19,    20,    21,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    37,    38,    39,    40,    41,    42,
      -1,    -1,    -1,    46,    47,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    25,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    33,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    -1,    -1,    -1,    46,    47,    16,    17,    18,
      19,    20,    21,    -1,    -1,    -1,    25,    26,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    -1,    -1,    -1,    46,    47,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    25,    26,
      16,    17,    18,    19,    20,    21,    -1,    -1,    -1,    25,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    46,
      47,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      46,    47,    16,    17,    18,    19,    20,    21,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    46,    47
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    27,    38,    44,    45,    48,    50,
      51,    52,    53,    54,    62,    64,    65,    66,    67,    68,
      69,    72,    74,    76,    77,    78,    80,    81,    87,    88,
      89,    90,    53,    69,    78,    78,    78,    78,    78,    27,
      27,    33,    78,    76,    69,    72,     0,    29,    63,    66,
      70,    71,    76,    31,    33,    22,    27,    32,    34,    16,
      17,    18,    19,    20,    21,    25,    26,    33,    37,    38,
      39,    40,    41,    42,    46,    47,    83,    84,    85,    22,
      34,    33,    76,    76,    28,    38,    63,    66,    86,    78,
      78,    33,    29,    76,    30,    64,    22,    34,    27,    71,
      78,    78,    82,    76,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    27,    78,    49,    33,    28,
      72,    91,    30,    29,    78,    35,    78,    69,    73,    75,
      28,    31,    22,    35,    35,    75,    86,    78,    63,    30,
      72,    29,    30,    78,    79,    35,    70,    71,    28,    31,
      78,    78,    28,    33,    79,    30,    31,    63,    73,    78,
      30,    79,    28,    63
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 56 "grammar.y"
    { programBlock = (yyvsp[(1) - (1)].block); ;}
    break;

  case 3:
#line 59 "grammar.y"
    { (yyval.block) = (yyvsp[(2) - (3)].block); ;}
    break;

  case 4:
#line 60 "grammar.y"
    { (yyval.block) = new NBlock(); ;}
    break;

  case 5:
#line 63 "grammar.y"
    { (yyval.block) = new NBlock(); (yyval.block)->child->push_back(shared_ptr<NStatement>((yyvsp[(1) - (1)].stmt))); ;}
    break;

  case 6:
#line 64 "grammar.y"
    { (yyvsp[(1) - (2)].block)->child->push_back(shared_ptr<NStatement>((yyvsp[(2) - (2)].stmt))); ;}
    break;

  case 7:
#line 65 "grammar.y"
    {
			    shared_ptr<NExpressionStatement> blk = std::make_shared<NExpressionStatement>(shared_ptr<NExpression>((yyvsp[(2) - (2)].block)));
			    (yyvsp[(1) - (2)].block)->child->push_back(blk);
			;}
    break;

  case 8:
#line 70 "grammar.y"
    { (yyval.stmt) = (yyvsp[(1) - (2)].stmt); ;}
    break;

  case 9:
#line 71 "grammar.y"
    { (yyval.stmt) = new NExpressionStatement(shared_ptr<NExpression>((yyvsp[(1) - (2)].expr))); ;}
    break;

  case 10:
#line 72 "grammar.y"
    { (yyval.stmt) = new NReturnStatement(shared_ptr<NExpression>((yyvsp[(2) - (3)].expr))); ;}
    break;

  case 11:
#line 73 "grammar.y"
    { (yyval.stmt) = new NReturnStatement(); ;}
    break;

  case 18:
#line 83 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true;  delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 19:
#line 84 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true; delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 20:
#line 85 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true; delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 21:
#line 86 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true; delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 22:
#line 87 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true; delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 23:
#line 88 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true; delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 24:
#line 89 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); (yyval.ident)->isType = true; delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 25:
#line 92 "grammar.y"
    {
				(yyvsp[(2) - (2)].ident)->isType = true;
				(yyval.ident) = (yyvsp[(2) - (2)].ident);
			;}
    break;

  case 26:
#line 97 "grammar.y"
    { (yyval.ident) = (yyvsp[(1) - (1)].ident); ;}
    break;

  case 27:
#line 98 "grammar.y"
    { (yyval.ident) = (yyvsp[(1) - (1)].ident); ;}
    break;

  case 28:
#line 100 "grammar.y"
    {
       NIdentifier *ident = new NIdentifier("int");
       ident->isType = true;
       (yyval.var_decl) = new NVariableDeclaration(
	     shared_ptr<NIdentifier>(ident),
	     shared_ptr<NIdentifier>((yyvsp[(1) - (1)].ident)),
	     nullptr);
     ;}
    break;

  case 29:
#line 108 "grammar.y"
    {
	(yyvsp[(1) - (3)].var_decl)->type->arraySize->push_back(make_shared<NIdentifier>("no-val"));
	(yyvsp[(1) - (3)].var_decl)->type->isArray = true;
	(yyval.var_decl) = (yyvsp[(1) - (3)].var_decl);
     ;}
    break;

  case 30:
#line 113 "grammar.y"
    {
	(yyvsp[(1) - (4)].var_decl)->type->arraySize->push_back(shared_ptr<NExpression>((yyvsp[(3) - (4)].expr)));
	(yyvsp[(1) - (4)].var_decl)->type->isArray = true;
	(yyval.var_decl) = (yyvsp[(1) - (4)].var_decl);
     ;}
    break;

  case 32:
#line 121 "grammar.y"
    {
   	(yyvsp[(1) - (3)].var_decl)->assignmentExpr = shared_ptr<NExpression>((yyvsp[(3) - (3)].expr));
   	(yyval.var_decl) = (yyvsp[(1) - (3)].var_decl);
;}
    break;

  case 33:
#line 125 "grammar.y"
    {
	(yyvsp[(1) - (4)].var_decl)->assignmentExpr = shared_ptr<NExpression>(new NInitializeExpr());
	(yyval.var_decl) = (yyvsp[(1) - (4)].var_decl);
;}
    break;

  case 34:
#line 129 "grammar.y"
    {
	(yyvsp[(1) - (5)].var_decl)->assignmentExpr = shared_ptr<NExpression>((yyvsp[(4) - (5)].expr));
	(yyval.var_decl) = (yyvsp[(1) - (5)].var_decl);
;}
    break;

  case 35:
#line 136 "grammar.y"
    {
	(yyval.stmt) = (yyvsp[(2) - (2)].var_decl);
	((NVariableDeclaration *) (yyvsp[(2) - (2)].var_decl))->type->setName((yyvsp[(1) - (2)].ident)->name);
	((NVariableDeclaration *) (yyvsp[(2) - (2)].var_decl))->type->isType = true;
;}
    break;

  case 36:
#line 141 "grammar.y"
    {
	(yyval.stmt) = (yyvsp[(2) - (2)].stmt);
	((NVariableDeclaration *) (yyvsp[(2) - (2)].stmt))->type->is_const = true;
;}
    break;

  case 37:
#line 145 "grammar.y"
    {
	(yyval.stmt) = (yyvsp[(1) - (3)].stmt);
	((NVariableDeclaration *) (yyvsp[(1) - (3)].stmt))->Add_decl(shared_ptr<NVariableDeclaration>((yyvsp[(3) - (3)].var_decl)));
;}
    break;

  case 38:
#line 151 "grammar.y"
    {
	(yyval.var_decl) = (yyvsp[(2) - (2)].var_decl);
	((NVariableDeclaration *) (yyvsp[(2) - (2)].var_decl))->type->setName((yyvsp[(1) - (2)].ident)->name);
;}
    break;

  case 39:
#line 157 "grammar.y"
    { (yyval.stmt) = new NFunctionDeclaration(shared_ptr<NIdentifier>((yyvsp[(1) - (6)].ident)), shared_ptr<NIdentifier>((yyvsp[(2) - (6)].ident)), shared_ptr<VariableList>((yyvsp[(4) - (6)].varvec)), shared_ptr<NBlock>((yyvsp[(6) - (6)].block)));  ;}
    break;

  case 40:
#line 158 "grammar.y"
    { (yyval.stmt) = new NFunctionDeclaration(shared_ptr<NIdentifier>((yyvsp[(2) - (6)].ident)), shared_ptr<NIdentifier>((yyvsp[(3) - (6)].ident)), shared_ptr<VariableList>((yyvsp[(5) - (6)].varvec)), nullptr, true); ;}
    break;

  case 41:
#line 160 "grammar.y"
    { (yyval.varvec) = new VariableList(); ;}
    break;

  case 42:
#line 161 "grammar.y"
    { (yyval.varvec) = new VariableList(); (yyval.varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[(1) - (1)].var_decl))); ;}
    break;

  case 43:
#line 162 "grammar.y"
    { (yyvsp[(1) - (3)].varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[(3) - (3)].var_decl))); ;}
    break;

  case 44:
#line 165 "grammar.y"
    { (yyval.ident) = new NIdentifier(*(yyvsp[(1) - (1)].string)); delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 45:
#line 168 "grammar.y"
    { (yyval.expr) = new NInteger(atol((yyvsp[(1) - (1)].string)->c_str())); ;}
    break;

  case 46:
#line 169 "grammar.y"
    { (yyval.expr) = new NDouble(atof((yyvsp[(1) - (1)].string)->c_str())); ;}
    break;

  case 47:
#line 170 "grammar.y"
    { (yyval.expr) = new NInteger(strtol((yyvsp[(1) - (1)].string)->c_str(), NULL, 16)); ;}
    break;

  case 48:
#line 172 "grammar.y"
    { (yyval.expr) = new NMethodCall(shared_ptr<NIdentifier>((yyvsp[(1) - (4)].ident)), shared_ptr<ExpressionList>((yyvsp[(3) - (4)].exprvec))); ;}
    break;

  case 50:
#line 174 "grammar.y"
    { (yyval.ident) = (yyvsp[(1) - (1)].ident); ;}
    break;

  case 51:
#line 175 "grammar.y"
    { (yyval.expr) = (yyvsp[(2) - (3)].expr); ;}
    break;

  case 52:
#line 176 "grammar.y"
    { (yyval.expr) = (yyvsp[(1) - (1)].index); ;}
    break;

  case 53:
#line 177 "grammar.y"
    { (yyval.expr) = new NStructMember(shared_ptr<NIdentifier>((yyvsp[(1) - (3)].ident)), shared_ptr<NIdentifier>((yyvsp[(3) - (3)].ident))); ;}
    break;

  case 54:
#line 178 "grammar.y"
    { (yyval.expr) = new NUnaryOperator((yyvsp[(1) - (2)].token), shared_ptr<NExpression>((yyvsp[(2) - (2)].expr))); ;}
    break;

  case 55:
#line 179 "grammar.y"
    { (yyval.expr) = new NUnaryOperator((yyvsp[(1) - (2)].token), shared_ptr<NExpression>((yyvsp[(2) - (2)].expr))); ;}
    break;

  case 56:
#line 180 "grammar.y"
    { (yyval.expr) = new NUnaryOperator((yyvsp[(1) - (2)].token), shared_ptr<NExpression>((yyvsp[(2) - (2)].expr))); ;}
    break;

  case 57:
#line 181 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 58:
#line 182 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 59:
#line 183 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 60:
#line 184 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 61:
#line 185 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 62:
#line 187 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 63:
#line 189 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 64:
#line 191 "grammar.y"
    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[(1) - (3)].expr)), (yyvsp[(2) - (3)].token), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 65:
#line 192 "grammar.y"
    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 66:
#line 193 "grammar.y"
    { (yyval.expr) = new NLiteral(*(yyvsp[(1) - (1)].string)); delete (yyvsp[(1) - (1)].string); ;}
    break;

  case 67:
#line 196 "grammar.y"
    {
		(yyval.expr) = new NInitializeExpr() ;
           	((NInitializeExpr *)(yyval.expr))->Append(shared_ptr<NExpression>((yyvsp[(1) - (1)].expr)));
	;}
    break;

  case 68:
#line 200 "grammar.y"
    {
		(yyval.expr) = (yyvsp[(1) - (3)].expr);
		((NInitializeExpr *)(yyval.expr))->Append(shared_ptr<NExpression>((yyvsp[(3) - (3)].expr)));
	;}
    break;

  case 69:
#line 204 "grammar.y"
    {
		(yyval.expr) = new NInitializeExpr() ;
        	((NInitializeExpr *)(yyval.expr))->Add_child(shared_ptr<NExpression>((yyvsp[(2) - (3)].expr)));
	;}
    break;

  case 70:
#line 212 "grammar.y"
    { (yyval.index) = new NArrayIndex(shared_ptr<NIdentifier>((yyvsp[(1) - (4)].ident)), shared_ptr<NExpression>((yyvsp[(3) - (4)].expr))); ;}
    break;

  case 71:
#line 214 "grammar.y"
    { 	
						(yyvsp[(1) - (4)].index)->expressions->push_back(shared_ptr<NExpression>((yyvsp[(3) - (4)].expr)));
						(yyval.index) = (yyvsp[(1) - (4)].index);
					;}
    break;

  case 72:
#line 218 "grammar.y"
    { (yyval.expr) = new NAssignment(shared_ptr<NIdentifier>((yyvsp[(1) - (3)].ident)), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 73:
#line 219 "grammar.y"
    {
				(yyval.expr) = new NArrayAssignment(shared_ptr<NArrayIndex>((yyvsp[(1) - (3)].index)), shared_ptr<NExpression>((yyvsp[(3) - (3)].expr)));
			;}
    break;

  case 74:
#line 222 "grammar.y"
    {
				auto member = make_shared<NStructMember>(shared_ptr<NIdentifier>((yyvsp[(1) - (5)].ident)), shared_ptr<NIdentifier>((yyvsp[(3) - (5)].ident))); 
				(yyval.expr) = new NStructAssignment(member, shared_ptr<NExpression>((yyvsp[(5) - (5)].expr))); 
			;}
    break;

  case 75:
#line 228 "grammar.y"
    { (yyval.exprvec) = new ExpressionList(); ;}
    break;

  case 76:
#line 229 "grammar.y"
    { (yyval.exprvec) = new ExpressionList(); (yyval.exprvec)->push_back(shared_ptr<NExpression>((yyvsp[(1) - (1)].expr))); ;}
    break;

  case 77:
#line 230 "grammar.y"
    { (yyvsp[(1) - (3)].exprvec)->push_back(shared_ptr<NExpression>((yyvsp[(3) - (3)].expr))); ;}
    break;

  case 89:
#line 240 "grammar.y"
    { (yyval.block) = (yyvsp[(1) - (1)].block); ;}
    break;

  case 90:
#line 241 "grammar.y"
    {
		       (yyval.block) = new NBlock();
		       (yyval.block)->child->push_back(shared_ptr<NStatement>((yyvsp[(1) - (1)].stmt)));
		     ;}
    break;

  case 91:
#line 247 "grammar.y"
    { (yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[(2) - (3)].expr)), shared_ptr<NBlock>((yyvsp[(3) - (3)].block))); ;}
    break;

  case 92:
#line 248 "grammar.y"
    { (yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[(2) - (5)].expr)), shared_ptr<NBlock>((yyvsp[(3) - (5)].block)), shared_ptr<NBlock>((yyvsp[(5) - (5)].block))); ;}
    break;

  case 93:
#line 251 "grammar.y"
    { (yyval.stmt) = new NForStatement(shared_ptr<NBlock>((yyvsp[(9) - (9)].block)), shared_ptr<NExpression>((yyvsp[(3) - (9)].expr)), shared_ptr<NExpression>((yyvsp[(5) - (9)].expr)), shared_ptr<NExpression>((yyvsp[(7) - (9)].expr))); ;}
    break;

  case 94:
#line 253 "grammar.y"
    { (yyval.stmt) = new NForStatement(shared_ptr<NBlock>((yyvsp[(5) - (5)].block)), nullptr, shared_ptr<NExpression>((yyvsp[(3) - (5)].expr)), nullptr); ;}
    break;

  case 95:
#line 255 "grammar.y"
    {(yyval.stmt) = new NStructDeclaration(shared_ptr<NIdentifier>((yyvsp[(2) - (5)].ident)), shared_ptr<VariableList>((yyvsp[(4) - (5)].varvec))); ;}
    break;

  case 96:
#line 257 "grammar.y"
    { (yyval.varvec) = new VariableList(); ;}
    break;

  case 97:
#line 258 "grammar.y"
    { (yyval.varvec) = new VariableList(); (yyval.varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[(1) - (1)].var_decl))); ;}
    break;

  case 98:
#line 259 "grammar.y"
    { (yyvsp[(1) - (2)].varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[(2) - (2)].var_decl))); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2221 "grammar.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 261 "grammar.y"


