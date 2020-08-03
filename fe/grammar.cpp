/* A Bison parser, made by GNU Bison 3.6.3.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.6.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "grammar.y"

	#include "ASTNodes.h"
	#include "basic.h"
	#include <stdio.h>
	#define register
	NBlock* programBlock;
	extern int yylex();
	extern int yyerror(char *msg, ...);

#line 81 "grammar.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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

#line 198 "grammar.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAMMAR_HPP_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TIDENTIFIER = 3,                /* TIDENTIFIER  */
  YYSYMBOL_TINTEGER = 4,                   /* TINTEGER  */
  YYSYMBOL_THEX = 5,                       /* THEX  */
  YYSYMBOL_TDOUBLE = 6,                    /* TDOUBLE  */
  YYSYMBOL_TYINT = 7,                      /* TYINT  */
  YYSYMBOL_TYDOUBLE = 8,                   /* TYDOUBLE  */
  YYSYMBOL_TYFLOAT = 9,                    /* TYFLOAT  */
  YYSYMBOL_TYCHAR = 10,                    /* TYCHAR  */
  YYSYMBOL_TYBOOL = 11,                    /* TYBOOL  */
  YYSYMBOL_TYVOID = 12,                    /* TYVOID  */
  YYSYMBOL_TYSTRING = 13,                  /* TYSTRING  */
  YYSYMBOL_TEXTERN = 14,                   /* TEXTERN  */
  YYSYMBOL_TLITERAL = 15,                  /* TLITERAL  */
  YYSYMBOL_TCEQ = 16,                      /* TCEQ  */
  YYSYMBOL_TCNE = 17,                      /* TCNE  */
  YYSYMBOL_TCLT = 18,                      /* TCLT  */
  YYSYMBOL_TCLE = 19,                      /* TCLE  */
  YYSYMBOL_TCGT = 20,                      /* TCGT  */
  YYSYMBOL_TCGE = 21,                      /* TCGE  */
  YYSYMBOL_TEQUAL = 22,                    /* TEQUAL  */
  YYSYMBOL_TAND = 23,                      /* TAND  */
  YYSYMBOL_TOR = 24,                       /* TOR  */
  YYSYMBOL_TLAND = 25,                     /* TLAND  */
  YYSYMBOL_TLOR = 26,                      /* TLOR  */
  YYSYMBOL_TLPAREN = 27,                   /* TLPAREN  */
  YYSYMBOL_TRPAREN = 28,                   /* TRPAREN  */
  YYSYMBOL_TLBRACE = 29,                   /* TLBRACE  */
  YYSYMBOL_TRBRACE = 30,                   /* TRBRACE  */
  YYSYMBOL_TCOMMA = 31,                    /* TCOMMA  */
  YYSYMBOL_TDOT = 32,                      /* TDOT  */
  YYSYMBOL_TSEMICOLON = 33,                /* TSEMICOLON  */
  YYSYMBOL_TLBRACKET = 34,                 /* TLBRACKET  */
  YYSYMBOL_TRBRACKET = 35,                 /* TRBRACKET  */
  YYSYMBOL_TQUOTATION = 36,                /* TQUOTATION  */
  YYSYMBOL_TPLUS = 37,                     /* TPLUS  */
  YYSYMBOL_TMINUS = 38,                    /* TMINUS  */
  YYSYMBOL_TMUL = 39,                      /* TMUL  */
  YYSYMBOL_TDIV = 40,                      /* TDIV  */
  YYSYMBOL_TXOR = 41,                      /* TXOR  */
  YYSYMBOL_TMOD = 42,                      /* TMOD  */
  YYSYMBOL_TNEG = 43,                      /* TNEG  */
  YYSYMBOL_TNOT = 44,                      /* TNOT  */
  YYSYMBOL_TANOT = 45,                     /* TANOT  */
  YYSYMBOL_TSHIFTL = 46,                   /* TSHIFTL  */
  YYSYMBOL_TSHIFTR = 47,                   /* TSHIFTR  */
  YYSYMBOL_TIF = 48,                       /* TIF  */
  YYSYMBOL_TELSE = 49,                     /* TELSE  */
  YYSYMBOL_TFOR = 50,                      /* TFOR  */
  YYSYMBOL_TWHILE = 51,                    /* TWHILE  */
  YYSYMBOL_TRETURN = 52,                   /* TRETURN  */
  YYSYMBOL_TSTRUCT = 53,                   /* TSTRUCT  */
  YYSYMBOL_TCONST = 54,                    /* TCONST  */
  YYSYMBOL_YYACCEPT = 55,                  /* $accept  */
  YYSYMBOL_program = 56,                   /* program  */
  YYSYMBOL_block = 57,                     /* block  */
  YYSYMBOL_stmts = 58,                     /* stmts  */
  YYSYMBOL_basic_stmt = 59,                /* basic_stmt  */
  YYSYMBOL_stmt = 60,                      /* stmt  */
  YYSYMBOL_primary_typename = 61,          /* primary_typename  */
  YYSYMBOL_struct_typename = 62,           /* struct_typename  */
  YYSYMBOL_typename = 63,                  /* typename  */
  YYSYMBOL_basic_var_decl = 64,            /* basic_var_decl  */
  YYSYMBOL_var_decl = 65,                  /* var_decl  */
  YYSYMBOL_func_decl = 66,                 /* func_decl  */
  YYSYMBOL_func_decl_args = 67,            /* func_decl_args  */
  YYSYMBOL_ident = 68,                     /* ident  */
  YYSYMBOL_numeric = 69,                   /* numeric  */
  YYSYMBOL_expr = 70,                      /* expr  */
  YYSYMBOL_init_expr = 71,                 /* init_expr  */
  YYSYMBOL_array_index = 72,               /* array_index  */
  YYSYMBOL_assign = 73,                    /* assign  */
  YYSYMBOL_call_args = 74,                 /* call_args  */
  YYSYMBOL_comparison = 75,                /* comparison  */
  YYSYMBOL_and_comparison = 76,            /* and_comparison  */
  YYSYMBOL_or_comparison = 77,             /* or_comparison  */
  YYSYMBOL_block_or_single_stmt = 78,      /* block_or_single_stmt  */
  YYSYMBOL_if_stmt = 79,                   /* if_stmt  */
  YYSYMBOL_for_stmt = 80,                  /* for_stmt  */
  YYSYMBOL_while_stmt = 81,                /* while_stmt  */
  YYSYMBOL_struct_decl = 82,               /* struct_decl  */
  YYSYMBOL_struct_members = 83             /* struct_members  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  56
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   885

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  99
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  175

#define YYMAXUTOK   309


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    50,    50,    53,    54,    57,    58,    59,    64,    65,
      66,    67,    70,    70,    70,    71,    72,    73,    77,    78,
      79,    80,    81,    82,    83,    86,    91,    92,    93,    95,
      96,   101,   106,   113,   114,   118,   121,   127,   129,   131,
     132,   133,   136,   139,   140,   141,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   164,   168,   172,   179,   181,
     186,   187,   190,   196,   197,   198,   199,   199,   199,   199,
     199,   199,   199,   199,   200,   200,   200,   203,   205,   208,
     209,   215,   216,   217,   223,   225,   227,   229,   230,   231
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TIDENTIFIER",
  "TINTEGER", "THEX", "TDOUBLE", "TYINT", "TYDOUBLE", "TYFLOAT", "TYCHAR",
  "TYBOOL", "TYVOID", "TYSTRING", "TEXTERN", "TLITERAL", "TCEQ", "TCNE",
  "TCLT", "TCLE", "TCGT", "TCGE", "TEQUAL", "TAND", "TOR", "TLAND", "TLOR",
  "TLPAREN", "TRPAREN", "TLBRACE", "TRBRACE", "TCOMMA", "TDOT",
  "TSEMICOLON", "TLBRACKET", "TRBRACKET", "TQUOTATION", "TPLUS", "TMINUS",
  "TMUL", "TDIV", "TXOR", "TMOD", "TNEG", "TNOT", "TANOT", "TSHIFTL",
  "TSHIFTR", "TIF", "TELSE", "TFOR", "TWHILE", "TRETURN", "TSTRUCT",
  "TCONST", "$accept", "program", "block", "stmts", "basic_stmt", "stmt",
  "primary_typename", "struct_typename", "typename", "basic_var_decl",
  "var_decl", "func_decl", "func_decl_args", "ident", "numeric", "expr",
  "init_expr", "array_index", "assign", "call_args", "comparison",
  "and_comparison", "or_comparison", "block_or_single_stmt", "if_stmt",
  "for_stmt", "while_stmt", "struct_decl", "struct_members", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309
};
#endif

#define YYPACT_NINF (-131)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-55)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     359,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,
    -131,  -131,   174,  -131,   470,   470,   470,   470,   470,   -18,
      -8,   426,    19,   174,    33,   255,  -131,  -131,  -131,  -131,
      19,    -9,     5,  -131,   -11,  -131,   486,    -7,  -131,  -131,
    -131,  -131,    16,    19,    19,   518,    76,    11,    11,   151,
     470,   470,  -131,   550,    31,  -131,  -131,   307,  -131,  -131,
      -3,   390,   434,  -131,   470,   470,    19,   470,  -131,  -131,
    -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,   470,
     470,   470,   470,  -131,   470,  -131,  -131,   470,   470,   470,
     470,   470,  -131,  -131,    34,  -131,   470,  -131,  -131,    19,
      13,   582,   614,  -131,   174,  -131,   203,   174,   439,   470,
     838,    36,  -131,   646,   838,   838,    12,    57,   678,    76,
      76,    11,    11,    11,   838,   838,   838,   838,   710,   174,
     742,  -131,    98,   470,    43,  -131,   368,  -131,  -131,    28,
     439,   838,    14,    -5,  -131,  -131,  -131,   470,   470,  -131,
    -131,    50,  -131,  -131,   774,  -131,  -131,  -131,    43,   174,
      24,  -131,   439,  -131,   838,   838,  -131,   470,  -131,  -131,
    -131,    49,   806,    43,  -131
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    42,    43,    45,    44,    18,    19,    20,    21,    22,
      23,    24,     0,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,    12,     5,    26,    27,
       0,    33,     0,    13,    48,    47,     0,    50,    63,    15,
      16,    17,     0,     0,     0,     0,    54,    52,    53,     0,
       0,     0,    11,     0,    25,    28,     1,     0,     7,     6,
      29,     0,     0,     8,     0,    73,     0,     0,    78,    79,
      80,    81,    82,    83,    87,    88,    76,    77,     9,     0,
       0,     0,     0,    84,     0,    85,    86,     0,     0,     0,
       0,     0,    14,    25,     0,    49,     0,    89,    90,     0,
      91,     0,     0,    10,    97,     4,     0,    39,     0,    73,
      34,    43,    31,     0,    70,    74,     0,    51,     0,    58,
      59,    56,    57,    55,    60,    61,    62,    71,     0,    39,
      59,    29,     0,     0,     0,    98,     0,     3,    40,     0,
       0,    65,     0,     0,    30,    32,    46,     0,     0,    68,
      69,     0,    92,    93,     0,    95,    96,    99,     0,     0,
       0,    36,     0,    35,    75,    72,    38,     0,    37,    41,
      67,    66,     0,     0,    94
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -131,  -131,   -44,    26,   -41,   -13,  -131,  -131,     6,  -131,
     -90,  -131,   -45,    -2,  -131,   -14,  -130,  -131,  -131,   -24,
    -131,  -131,  -131,   -46,   -43,  -131,  -131,  -131,  -131
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    24,    58,    25,    26,    27,    28,    29,    99,    31,
      32,    33,   139,    34,    35,    36,   142,    37,    38,   116,
      87,    88,    89,   100,    39,    40,    41,    42,   136
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      45,    46,    47,    48,    49,    97,    30,    53,    98,    50,
     160,    64,    59,    61,   135,    90,    65,   138,    44,    51,
      54,    66,     1,    67,   107,    62,   147,    91,    60,    55,
     163,    30,   171,    56,    74,    75,   101,   102,    63,   138,
     146,    93,    94,   147,   161,   162,   157,   110,   113,    92,
     114,   115,    83,   118,   170,   162,   158,    85,    86,   159,
     104,   129,   132,    30,   117,   119,   120,   121,   122,   169,
     123,   144,    57,   124,   125,   126,   127,   128,   166,   148,
     162,   159,   130,   106,   151,   143,   152,     0,    97,   153,
     155,    98,     0,    59,   141,   115,     0,   131,     0,    74,
      75,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    30,    13,   168,    81,    82,    83,    84,   154,
       0,     0,    85,    86,     0,    14,   141,    57,     0,   174,
       0,     0,     0,   164,   165,     0,    15,     0,     0,     0,
       0,     0,    16,    17,     0,     0,    18,     0,   141,     0,
      21,    43,    23,   172,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,     0,    13,    68,    69,    70,
      71,    72,    73,     0,    74,    75,    76,    77,    14,     0,
      57,     5,     6,     7,     8,     9,    10,    11,    79,    96,
      81,    82,    83,    84,     0,    16,    17,    85,    86,     0,
       0,     0,     0,    21,    43,    23,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,    43,    23,     0,
      14,     0,    57,   137,     0,     0,     0,     0,     0,     0,
       0,    15,     0,     0,     0,     0,     0,    16,    17,     0,
       0,    18,     0,    19,    20,    21,    22,    23,     1,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,     0,    57,     0,     0,     0,     0,     0,
       0,     0,     0,    15,     0,     0,     0,     0,     0,    16,
      17,     0,     0,    18,     0,    19,    20,    21,    22,    23,
       1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,     0,   105,     0,     0,
       0,     0,     0,     0,     0,    15,     0,     0,     0,     0,
       0,    16,    17,     0,     0,    18,     0,    19,    20,    21,
      22,    23,     1,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,     5,     6,     7,     8,     9,
      10,    11,     0,     0,     0,     0,    14,     0,     0,     0,
       0,     0,     0,     1,     2,     3,     4,    15,   156,     0,
       0,     0,     0,    16,    17,    13,     0,    18,     0,    19,
      20,    21,    22,    23,     0,     0,     0,    14,     0,   108,
       0,    43,    23,     0,   109,     0,     0,     0,    15,     1,
       2,     3,     4,     0,    16,    17,     0,     1,   111,     3,
       4,    13,     1,     2,     3,     4,     0,     0,     0,    13,
       0,     0,     0,    14,    13,     0,     0,     0,     0,    52,
       0,    14,     0,     0,    15,     0,    14,     0,   140,   112,
      16,    17,    15,     1,     2,     3,     4,    15,    16,    17,
       0,     0,     0,    16,    17,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,     0,     0,
       0,     0,    68,    69,    70,    71,    72,    73,    15,    74,
      75,    76,    77,     0,    16,    17,     0,     0,     0,    78,
       0,     0,     0,    79,    80,    81,    82,    83,    84,     0,
       0,     0,    85,    86,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,     0,    95,     0,     0,     0,
       0,     0,     0,     0,     0,    79,    80,    81,    82,    83,
      84,     0,     0,     0,    85,    86,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,   103,     0,     0,     0,    79,    80,    81,
      82,    83,    84,     0,     0,     0,    85,    86,    68,    69,
      70,    71,    72,    73,     0,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,   133,     0,     0,     0,    79,
      80,    81,    82,    83,    84,     0,     0,     0,    85,    86,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,     0,   134,     0,     0,     0,     0,     0,     0,     0,
       0,    79,    80,    81,    82,    83,    84,     0,     0,     0,
      85,    86,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,     0,
       0,   145,     0,    79,    80,    81,    82,    83,    84,     0,
       0,     0,    85,    86,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,   149,     0,    79,    80,    81,    82,    83,
      84,     0,     0,     0,    85,    86,    68,    69,    70,    71,
      72,    73,     0,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,     0,     0,   150,     0,    79,    80,    81,
      82,    83,    84,     0,     0,     0,    85,    86,   -54,   -54,
     -54,   -54,   -54,   -54,     0,    74,    75,   -54,   -54,     0,
       0,     0,     0,     0,     0,   -54,     0,     0,     0,   -54,
     -54,    81,    82,    83,    84,     0,     0,     0,    85,    86,
      68,    69,    70,    71,    72,    73,     0,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,   167,     0,     0,
       0,    79,    80,    81,    82,    83,    84,     0,     0,     0,
      85,    86,    68,    69,    70,    71,    72,    73,     0,    74,
      75,    76,    77,     0,   173,     0,     0,     0,     0,     0,
       0,     0,     0,    79,    80,    81,    82,    83,    84,     0,
       0,     0,    85,    86,    68,    69,    70,    71,    72,    73,
       0,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    79,    80,    81,    82,    83,
      84,     0,     0,     0,    85,    86
};

static const yytype_int16 yycheck[] =
{
      14,    15,    16,    17,    18,    49,     0,    21,    49,    27,
     140,    22,    25,    22,   104,    22,    27,   107,    12,    27,
      22,    32,     3,    34,    27,    34,    31,    34,    30,    23,
      35,    25,   162,     0,    23,    24,    50,    51,    33,   129,
      28,    43,    44,    31,    30,    31,   136,    61,    62,    33,
      64,    65,    41,    67,    30,    31,    28,    46,    47,    31,
      29,    27,    49,    57,    66,    79,    80,    81,    82,   159,
      84,    35,    29,    87,    88,    89,    90,    91,    28,    22,
      31,    31,    96,    57,   129,   109,   132,    -1,   132,   132,
     134,   132,    -1,   106,   108,   109,    -1,    99,    -1,    23,
      24,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,   106,    15,   158,    39,    40,    41,    42,   133,
      -1,    -1,    46,    47,    -1,    27,   140,    29,    -1,   173,
      -1,    -1,    -1,   147,   148,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    48,    -1,   162,    -1,
      52,    53,    54,   167,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    18,
      19,    20,    21,    -1,    23,    24,    25,    26,    27,    -1,
      29,     7,     8,     9,    10,    11,    12,    13,    37,    38,
      39,    40,    41,    42,    -1,    44,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      27,    -1,    29,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,     7,     8,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,    38,    30,    -1,
      -1,    -1,    -1,    44,    45,    15,    -1,    48,    -1,    50,
      51,    52,    53,    54,    -1,    -1,    -1,    27,    -1,    29,
      -1,    53,    54,    -1,    34,    -1,    -1,    -1,    38,     3,
       4,     5,     6,    -1,    44,    45,    -1,     3,     4,     5,
       6,    15,     3,     4,     5,     6,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    27,    15,    -1,    -1,    -1,    -1,    33,
      -1,    27,    -1,    -1,    38,    -1,    27,    -1,    29,    35,
      44,    45,    38,     3,     4,     5,     6,    38,    44,    45,
      -1,    -1,    -1,    44,    45,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    16,    17,    18,    19,    20,    21,    38,    23,
      24,    25,    26,    -1,    44,    45,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    46,    47,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    26,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    46,    47,    16,    17,    18,    19,
      20,    21,    -1,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    46,    47,    16,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    46,    47,
      16,    17,    18,    19,    20,    21,    -1,    23,    24,    25,
      26,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      46,    47,    16,    17,    18,    19,    20,    21,    -1,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    -1,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    46,    47,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    46,    47,    16,    17,    18,    19,
      20,    21,    -1,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    46,    47,    16,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    46,    47,
      16,    17,    18,    19,    20,    21,    -1,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      46,    47,    16,    17,    18,    19,    20,    21,    -1,    23,
      24,    25,    26,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    46,    47,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    46,    47
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    27,    38,    44,    45,    48,    50,
      51,    52,    53,    54,    56,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    68,    69,    70,    72,    73,    79,
      80,    81,    82,    53,    63,    70,    70,    70,    70,    70,
      27,    27,    33,    70,    68,    63,     0,    29,    57,    60,
      68,    22,    34,    33,    22,    27,    32,    34,    16,    17,
      18,    19,    20,    21,    23,    24,    25,    26,    33,    37,
      38,    39,    40,    41,    42,    46,    47,    75,    76,    77,
      22,    34,    33,    68,    68,    28,    38,    57,    59,    63,
      78,    70,    70,    33,    29,    30,    58,    27,    29,    34,
      70,     4,    35,    70,    70,    70,    74,    68,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    27,
      70,    68,    49,    33,    28,    65,    83,    30,    65,    67,
      29,    70,    71,    74,    35,    35,    28,    31,    22,    35,
      35,    67,    78,    79,    70,    57,    30,    65,    28,    31,
      71,    30,    31,    35,    70,    70,    28,    33,    57,    65,
      30,    71,    70,    28,    57
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    55,    56,    57,    57,    58,    58,    58,    59,    59,
      59,    59,    60,    60,    60,    60,    60,    60,    61,    61,
      61,    61,    61,    61,    61,    62,    63,    63,    63,    64,
      64,    64,    64,    65,    65,    65,    65,    66,    66,    67,
      67,    67,    68,    69,    69,    69,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    71,    71,    71,    72,    72,
      73,    73,    73,    74,    74,    74,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    76,    77,    78,
      78,    79,    79,    79,    80,    81,    82,    83,    83,    83
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     2,     1,     2,     2,     2,     2,
       3,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     2,     2,
       4,     3,     4,     1,     3,     5,     5,     6,     6,     0,
       1,     3,     1,     1,     1,     1,     4,     1,     1,     3,
       1,     3,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     3,     3,     4,     4,
       3,     3,     5,     0,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     5,     5,     9,     5,     5,     0,     1,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yystacksize = YYINITDEPTH;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;


  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

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
#line 50 "grammar.y"
                { programBlock = (yyvsp[0].block); }
#line 1531 "grammar.cpp"
    break;

  case 3:
#line 53 "grammar.y"
                              { (yyval.block) = (yyvsp[-1].block); }
#line 1537 "grammar.cpp"
    break;

  case 4:
#line 54 "grammar.y"
                          { (yyval.block) = new NBlock(); }
#line 1543 "grammar.cpp"
    break;

  case 5:
#line 57 "grammar.y"
             { (yyval.block) = new NBlock(); (yyval.block)->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt))); }
#line 1549 "grammar.cpp"
    break;

  case 6:
#line 58 "grammar.y"
                                     { (yyvsp[-1].block)->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt))); }
#line 1555 "grammar.cpp"
    break;

  case 7:
#line 59 "grammar.y"
                                      {
			    shared_ptr<NExpressionStatement> blk = std::make_shared<NExpressionStatement>(shared_ptr<NExpression>((yyvsp[0].block)));
			    (yyvsp[-1].block)->child->push_back(blk);
			}
#line 1564 "grammar.cpp"
    break;

  case 8:
#line 64 "grammar.y"
                                 { (yyval.stmt) = (yyvsp[-1].stmt); }
#line 1570 "grammar.cpp"
    break;

  case 9:
#line 65 "grammar.y"
                               { (yyval.stmt) = new NExpressionStatement(shared_ptr<NExpression>((yyvsp[-1].expr))); }
#line 1576 "grammar.cpp"
    break;

  case 10:
#line 66 "grammar.y"
                                       { (yyval.stmt) = new NReturnStatement(shared_ptr<NExpression>((yyvsp[-1].expr))); }
#line 1582 "grammar.cpp"
    break;

  case 11:
#line 67 "grammar.y"
                                  { (yyval.stmt) = new NReturnStatement(); }
#line 1588 "grammar.cpp"
    break;

  case 18:
#line 77 "grammar.y"
                         { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true;  delete (yyvsp[0].string); }
#line 1594 "grammar.cpp"
    break;

  case 19:
#line 78 "grammar.y"
                                                   { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1600 "grammar.cpp"
    break;

  case 20:
#line 79 "grammar.y"
                                                  { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1606 "grammar.cpp"
    break;

  case 21:
#line 80 "grammar.y"
                                                 { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1612 "grammar.cpp"
    break;

  case 22:
#line 81 "grammar.y"
                                                 { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1618 "grammar.cpp"
    break;

  case 23:
#line 82 "grammar.y"
                                                 { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1624 "grammar.cpp"
    break;

  case 24:
#line 83 "grammar.y"
                                                   { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1630 "grammar.cpp"
    break;

  case 25:
#line 86 "grammar.y"
                                {
				(yyvsp[0].ident)->isType = true;
				(yyval.ident) = (yyvsp[0].ident);
			}
#line 1639 "grammar.cpp"
    break;

  case 26:
#line 91 "grammar.y"
                            { (yyval.ident) = (yyvsp[0].ident); }
#line 1645 "grammar.cpp"
    break;

  case 27:
#line 92 "grammar.y"
                           { (yyval.ident) = (yyvsp[0].ident); }
#line 1651 "grammar.cpp"
    break;

  case 28:
#line 93 "grammar.y"
                           { (yyval.ident) = (yyvsp[0].ident); (yyvsp[0].ident)->is_const = true; }
#line 1657 "grammar.cpp"
    break;

  case 29:
#line 95 "grammar.y"
                                { (yyval.var_decl) = new NVariableDeclaration(shared_ptr<NIdentifier>((yyvsp[-1].ident)), shared_ptr<NIdentifier>((yyvsp[0].ident)), nullptr); }
#line 1663 "grammar.cpp"
    break;

  case 30:
#line 96 "grammar.y"
                                                      {
          		(yyvsp[-3].var_decl)->type->arraySize->push_back(make_shared<NInteger>(atol((yyvsp[-1].string)->c_str())));
          		(yyvsp[-3].var_decl)->type->isArray = true;
          		(yyval.var_decl) = (yyvsp[-3].var_decl);
        }
#line 1673 "grammar.cpp"
    break;

  case 31:
#line 101 "grammar.y"
                                             {
			(yyvsp[-2].var_decl)->type->arraySize->push_back(make_shared<NIdentifier>("no-val"));
			(yyvsp[-2].var_decl)->type->isArray = true;
			(yyval.var_decl) = (yyvsp[-2].var_decl);
	}
#line 1683 "grammar.cpp"
    break;

  case 32:
#line 106 "grammar.y"
                                                  {
			(yyvsp[-3].var_decl)->type->arraySize->push_back(shared_ptr<NExpression>((yyvsp[-1].expr)));
			(yyvsp[-3].var_decl)->type->isArray = true;
			(yyval.var_decl) = (yyvsp[-3].var_decl);
        }
#line 1693 "grammar.cpp"
    break;

  case 33:
#line 113 "grammar.y"
                          { (yyval.stmt) = (yyvsp[0].var_decl); }
#line 1699 "grammar.cpp"
    break;

  case 34:
#line 114 "grammar.y"
                                      {
	 	(yyvsp[-2].var_decl)->assignmentExpr = shared_ptr<NExpression>((yyvsp[0].expr));
	 	(yyval.stmt) = (yyvsp[-2].var_decl);
	 }
#line 1708 "grammar.cpp"
    break;

  case 35:
#line 118 "grammar.y"
                                                               {
		 (yyval.stmt) = new NArrayInitialization(shared_ptr<NVariableDeclaration>((yyvsp[-4].var_decl)), shared_ptr<ExpressionList>((yyvsp[-1].exprvec)));
	 }
#line 1716 "grammar.cpp"
    break;

  case 36:
#line 121 "grammar.y"
                                                           {
	 	(yyvsp[-4].var_decl)->assignmentExpr = shared_ptr<NExpression>((yyvsp[-1].expr));
                (yyval.stmt) = (yyvsp[-4].var_decl);
	 }
#line 1725 "grammar.cpp"
    break;

  case 37:
#line 128 "grammar.y"
                                { (yyval.stmt) = new NFunctionDeclaration(shared_ptr<NIdentifier>((yyvsp[-5].ident)), shared_ptr<NIdentifier>((yyvsp[-4].ident)), shared_ptr<VariableList>((yyvsp[-2].varvec)), shared_ptr<NBlock>((yyvsp[0].block)));  }
#line 1731 "grammar.cpp"
    break;

  case 38:
#line 129 "grammar.y"
                                                                                { (yyval.stmt) = new NFunctionDeclaration(shared_ptr<NIdentifier>((yyvsp[-4].ident)), shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<VariableList>((yyvsp[-1].varvec)), nullptr, true); }
#line 1737 "grammar.cpp"
    break;

  case 39:
#line 131 "grammar.y"
                             { (yyval.varvec) = new VariableList(); }
#line 1743 "grammar.cpp"
    break;

  case 40:
#line 132 "grammar.y"
                                                                    { (yyval.varvec) = new VariableList(); (yyval.varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 1749 "grammar.cpp"
    break;

  case 41:
#line 133 "grammar.y"
                                                                                          { (yyvsp[-2].varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 1755 "grammar.cpp"
    break;

  case 42:
#line 136 "grammar.y"
                    { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); delete (yyvsp[0].string); }
#line 1761 "grammar.cpp"
    break;

  case 43:
#line 139 "grammar.y"
                   { (yyval.expr) = new NInteger(atol((yyvsp[0].string)->c_str())); }
#line 1767 "grammar.cpp"
    break;

  case 44:
#line 140 "grammar.y"
                   { (yyval.expr) = new NDouble(atof((yyvsp[0].string)->c_str())); }
#line 1773 "grammar.cpp"
    break;

  case 45:
#line 141 "grammar.y"
                { (yyval.expr) = new NInteger(strtol((yyvsp[0].string)->c_str(), NULL, 16)); }
#line 1779 "grammar.cpp"
    break;

  case 46:
#line 143 "grammar.y"
                                         { (yyval.expr) = new NMethodCall(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<ExpressionList>((yyvsp[-1].exprvec))); }
#line 1785 "grammar.cpp"
    break;

  case 48:
#line 145 "grammar.y"
                         { (yyval.ident) = (yyvsp[0].ident); }
#line 1791 "grammar.cpp"
    break;

  case 49:
#line 146 "grammar.y"
                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 1797 "grammar.cpp"
    break;

  case 50:
#line 147 "grammar.y"
                               { (yyval.expr) = (yyvsp[0].index); }
#line 1803 "grammar.cpp"
    break;

  case 51:
#line 148 "grammar.y"
                                    { (yyval.expr) = new NStructMember(shared_ptr<NIdentifier>((yyvsp[-2].ident)), shared_ptr<NIdentifier>((yyvsp[0].ident))); }
#line 1809 "grammar.cpp"
    break;

  case 52:
#line 149 "grammar.y"
                             { (yyval.expr) = new NUnaryOperator((yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1815 "grammar.cpp"
    break;

  case 53:
#line 150 "grammar.y"
                              { (yyval.expr) = new NUnaryOperator((yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1821 "grammar.cpp"
    break;

  case 54:
#line 151 "grammar.y"
                               { (yyval.expr) = new NUnaryOperator((yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1827 "grammar.cpp"
    break;

  case 55:
#line 152 "grammar.y"
                                  { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1833 "grammar.cpp"
    break;

  case 56:
#line 153 "grammar.y"
                                  { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1839 "grammar.cpp"
    break;

  case 57:
#line 154 "grammar.y"
                                  { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1845 "grammar.cpp"
    break;

  case 58:
#line 155 "grammar.y"
                                   { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1851 "grammar.cpp"
    break;

  case 59:
#line 156 "grammar.y"
                                    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1857 "grammar.cpp"
    break;

  case 60:
#line 157 "grammar.y"
                                        { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1863 "grammar.cpp"
    break;

  case 61:
#line 158 "grammar.y"
                                            { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1869 "grammar.cpp"
    break;

  case 62:
#line 159 "grammar.y"
                                           { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1875 "grammar.cpp"
    break;

  case 63:
#line 160 "grammar.y"
                          { (yyval.expr) = (yyvsp[0].expr); }
#line 1881 "grammar.cpp"
    break;

  case 64:
#line 161 "grammar.y"
                            { (yyval.expr) = new NLiteral(*(yyvsp[0].string)); delete (yyvsp[0].string); }
#line 1887 "grammar.cpp"
    break;

  case 65:
#line 164 "grammar.y"
             {
		(yyval.expr) = new NInitializeExpr() ;
           	((NInitializeExpr *)(yyval.expr))->Append(shared_ptr<NExpression>((yyvsp[0].expr)));
	}
#line 1896 "grammar.cpp"
    break;

  case 66:
#line 168 "grammar.y"
                                     {
		(yyval.expr) = (yyvsp[-2].expr);
		((NInitializeExpr *)(yyval.expr))->Append(shared_ptr<NExpression>((yyvsp[0].expr)));
	}
#line 1905 "grammar.cpp"
    break;

  case 67:
#line 172 "grammar.y"
                                    {
		(yyval.expr) = new NInitializeExpr() ;
        	((NInitializeExpr *)(yyval.expr))->Add_child(shared_ptr<NExpression>((yyvsp[-1].expr)));
	}
#line 1914 "grammar.cpp"
    break;

  case 68:
#line 180 "grammar.y"
                                { (yyval.index) = new NArrayIndex(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<NExpression>((yyvsp[-1].expr))); }
#line 1920 "grammar.cpp"
    break;

  case 69:
#line 182 "grammar.y"
                                        { 	
						(yyvsp[-3].index)->expressions->push_back(shared_ptr<NExpression>((yyvsp[-1].expr)));
						(yyval.index) = (yyvsp[-3].index);
					}
#line 1929 "grammar.cpp"
    break;

  case 70:
#line 186 "grammar.y"
                           { (yyval.expr) = new NAssignment(shared_ptr<NIdentifier>((yyvsp[-2].ident)), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1935 "grammar.cpp"
    break;

  case 71:
#line 187 "grammar.y"
                                                  {
				(yyval.expr) = new NArrayAssignment(shared_ptr<NArrayIndex>((yyvsp[-2].index)), shared_ptr<NExpression>((yyvsp[0].expr)));
			}
#line 1943 "grammar.cpp"
    break;

  case 72:
#line 190 "grammar.y"
                                                       {
				auto member = make_shared<NStructMember>(shared_ptr<NIdentifier>((yyvsp[-4].ident)), shared_ptr<NIdentifier>((yyvsp[-2].ident))); 
				(yyval.expr) = new NStructAssignment(member, shared_ptr<NExpression>((yyvsp[0].expr))); 
			}
#line 1952 "grammar.cpp"
    break;

  case 73:
#line 196 "grammar.y"
                        { (yyval.exprvec) = new ExpressionList(); }
#line 1958 "grammar.cpp"
    break;

  case 74:
#line 197 "grammar.y"
                                               { (yyval.exprvec) = new ExpressionList(); (yyval.exprvec)->push_back(shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1964 "grammar.cpp"
    break;

  case 75:
#line 198 "grammar.y"
                                                                { (yyvsp[-2].exprvec)->push_back(shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1970 "grammar.cpp"
    break;

  case 89:
#line 208 "grammar.y"
                             { (yyval.block) = (yyvsp[0].block); }
#line 1976 "grammar.cpp"
    break;

  case 90:
#line 209 "grammar.y"
                                  {
		       (yyval.block) = new NBlock();
		       (yyval.block)->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt)));
		     }
#line 1985 "grammar.cpp"
    break;

  case 91:
#line 215 "grammar.y"
                                        { (yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[-1].expr)), shared_ptr<NBlock>((yyvsp[0].block))); }
#line 1991 "grammar.cpp"
    break;

  case 92:
#line 216 "grammar.y"
                                                                           { (yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[-3].expr)), shared_ptr<NBlock>((yyvsp[-2].block)), shared_ptr<NBlock>((yyvsp[0].block))); }
#line 1997 "grammar.cpp"
    break;

  case 93:
#line 217 "grammar.y"
                                                              {
			auto blk = new NBlock(); 
			blk->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt)));
			(yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[-3].expr)), shared_ptr<NBlock>((yyvsp[-2].block)), shared_ptr<NBlock>(blk)); 
		}
#line 2007 "grammar.cpp"
    break;

  case 94:
#line 223 "grammar.y"
                                                                           { (yyval.stmt) = new NForStatement(shared_ptr<NBlock>((yyvsp[0].block)), shared_ptr<NExpression>((yyvsp[-6].expr)), shared_ptr<NExpression>((yyvsp[-4].expr)), shared_ptr<NExpression>((yyvsp[-2].expr))); }
#line 2013 "grammar.cpp"
    break;

  case 95:
#line 225 "grammar.y"
                                               { (yyval.stmt) = new NForStatement(shared_ptr<NBlock>((yyvsp[0].block)), nullptr, shared_ptr<NExpression>((yyvsp[-2].expr)), nullptr); }
#line 2019 "grammar.cpp"
    break;

  case 96:
#line 227 "grammar.y"
                                                           {(yyval.stmt) = new NStructDeclaration(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<VariableList>((yyvsp[-1].varvec))); }
#line 2025 "grammar.cpp"
    break;

  case 97:
#line 229 "grammar.y"
                             { (yyval.varvec) = new VariableList(); }
#line 2031 "grammar.cpp"
    break;

  case 98:
#line 230 "grammar.y"
                                           { (yyval.varvec) = new VariableList(); (yyval.varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 2037 "grammar.cpp"
    break;

  case 99:
#line 231 "grammar.y"
                                                          { (yyvsp[-1].varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 2043 "grammar.cpp"
    break;


#line 2047 "grammar.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

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


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 233 "grammar.y"

