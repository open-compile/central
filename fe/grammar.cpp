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
	NBlock* programBlock;
	extern int yylex();
	extern int yyerror(char *msg, ...);

#line 80 "grammar.cpp"

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

#line 192 "grammar.cpp"

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
  YYSYMBOL_TDOUBLE = 5,                    /* TDOUBLE  */
  YYSYMBOL_TYINT = 6,                      /* TYINT  */
  YYSYMBOL_TYDOUBLE = 7,                   /* TYDOUBLE  */
  YYSYMBOL_TYFLOAT = 8,                    /* TYFLOAT  */
  YYSYMBOL_TYCHAR = 9,                     /* TYCHAR  */
  YYSYMBOL_TYBOOL = 10,                    /* TYBOOL  */
  YYSYMBOL_TYVOID = 11,                    /* TYVOID  */
  YYSYMBOL_TYSTRING = 12,                  /* TYSTRING  */
  YYSYMBOL_TEXTERN = 13,                   /* TEXTERN  */
  YYSYMBOL_TLITERAL = 14,                  /* TLITERAL  */
  YYSYMBOL_TCEQ = 15,                      /* TCEQ  */
  YYSYMBOL_TCNE = 16,                      /* TCNE  */
  YYSYMBOL_TCLT = 17,                      /* TCLT  */
  YYSYMBOL_TCLE = 18,                      /* TCLE  */
  YYSYMBOL_TCGT = 19,                      /* TCGT  */
  YYSYMBOL_TCGE = 20,                      /* TCGE  */
  YYSYMBOL_TEQUAL = 21,                    /* TEQUAL  */
  YYSYMBOL_TLPAREN = 22,                   /* TLPAREN  */
  YYSYMBOL_TRPAREN = 23,                   /* TRPAREN  */
  YYSYMBOL_TLBRACE = 24,                   /* TLBRACE  */
  YYSYMBOL_TRBRACE = 25,                   /* TRBRACE  */
  YYSYMBOL_TCOMMA = 26,                    /* TCOMMA  */
  YYSYMBOL_TDOT = 27,                      /* TDOT  */
  YYSYMBOL_TSEMICOLON = 28,                /* TSEMICOLON  */
  YYSYMBOL_TLBRACKET = 29,                 /* TLBRACKET  */
  YYSYMBOL_TRBRACKET = 30,                 /* TRBRACKET  */
  YYSYMBOL_TQUOTATION = 31,                /* TQUOTATION  */
  YYSYMBOL_TPLUS = 32,                     /* TPLUS  */
  YYSYMBOL_TMINUS = 33,                    /* TMINUS  */
  YYSYMBOL_TMUL = 34,                      /* TMUL  */
  YYSYMBOL_TDIV = 35,                      /* TDIV  */
  YYSYMBOL_TAND = 36,                      /* TAND  */
  YYSYMBOL_TOR = 37,                       /* TOR  */
  YYSYMBOL_TXOR = 38,                      /* TXOR  */
  YYSYMBOL_TMOD = 39,                      /* TMOD  */
  YYSYMBOL_TNEG = 40,                      /* TNEG  */
  YYSYMBOL_TNOT = 41,                      /* TNOT  */
  YYSYMBOL_TSHIFTL = 42,                   /* TSHIFTL  */
  YYSYMBOL_TSHIFTR = 43,                   /* TSHIFTR  */
  YYSYMBOL_TIF = 44,                       /* TIF  */
  YYSYMBOL_TELSE = 45,                     /* TELSE  */
  YYSYMBOL_TFOR = 46,                      /* TFOR  */
  YYSYMBOL_TWHILE = 47,                    /* TWHILE  */
  YYSYMBOL_TRETURN = 48,                   /* TRETURN  */
  YYSYMBOL_TSTRUCT = 49,                   /* TSTRUCT  */
  YYSYMBOL_YYACCEPT = 50,                  /* $accept  */
  YYSYMBOL_program = 51,                   /* program  */
  YYSYMBOL_stmts = 52,                     /* stmts  */
  YYSYMBOL_stmt = 53,                      /* stmt  */
  YYSYMBOL_block = 54,                     /* block  */
  YYSYMBOL_primary_typename = 55,          /* primary_typename  */
  YYSYMBOL_array_typename = 56,            /* array_typename  */
  YYSYMBOL_struct_typename = 57,           /* struct_typename  */
  YYSYMBOL_typename = 58,                  /* typename  */
  YYSYMBOL_var_decl = 59,                  /* var_decl  */
  YYSYMBOL_func_decl = 60,                 /* func_decl  */
  YYSYMBOL_func_decl_args = 61,            /* func_decl_args  */
  YYSYMBOL_ident = 62,                     /* ident  */
  YYSYMBOL_numeric = 63,                   /* numeric  */
  YYSYMBOL_expr = 64,                      /* expr  */
  YYSYMBOL_array_index = 65,               /* array_index  */
  YYSYMBOL_assign = 66,                    /* assign  */
  YYSYMBOL_call_args = 67,                 /* call_args  */
  YYSYMBOL_comparison = 68,                /* comparison  */
  YYSYMBOL_if_stmt = 69,                   /* if_stmt  */
  YYSYMBOL_for_stmt = 70,                  /* for_stmt  */
  YYSYMBOL_while_stmt = 71,                /* while_stmt  */
  YYSYMBOL_struct_decl = 72,               /* struct_decl  */
  YYSYMBOL_struct_members = 73             /* struct_members  */
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
#define YYFINAL  47
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   595

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  24
/* YYNRULES -- Number of rules.  */
#define YYNRULES  81
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  146

#define YYMAXUTOK   304


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
      45,    46,    47,    48,    49
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    44,    44,    46,    47,    49,    49,    49,    50,    51,
      52,    53,    54,    57,    58,    61,    62,    63,    64,    65,
      66,    67,    69,    74,    79,    84,    85,    86,    88,    89,
      90,    95,    97,    99,   100,   101,   104,   107,   108,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   127,   129,   134,   135,   138,   144,
     145,   146,   147,   147,   147,   147,   147,   147,   148,   148,
     148,   148,   148,   150,   151,   152,   158,   160,   162,   164,
     165,   166
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
  "TINTEGER", "TDOUBLE", "TYINT", "TYDOUBLE", "TYFLOAT", "TYCHAR",
  "TYBOOL", "TYVOID", "TYSTRING", "TEXTERN", "TLITERAL", "TCEQ", "TCNE",
  "TCLT", "TCLE", "TCGT", "TCGE", "TEQUAL", "TLPAREN", "TRPAREN",
  "TLBRACE", "TRBRACE", "TCOMMA", "TDOT", "TSEMICOLON", "TLBRACKET",
  "TRBRACKET", "TQUOTATION", "TPLUS", "TMINUS", "TMUL", "TDIV", "TAND",
  "TOR", "TXOR", "TMOD", "TNEG", "TNOT", "TSHIFTL", "TSHIFTR", "TIF",
  "TELSE", "TFOR", "TWHILE", "TRETURN", "TSTRUCT", "$accept", "program",
  "stmts", "stmt", "block", "primary_typename", "array_typename",
  "struct_typename", "typename", "var_decl", "func_decl", "func_decl_args",
  "ident", "numeric", "expr", "array_index", "assign", "call_args",
  "comparison", "if_stmt", "for_stmt", "while_stmt", "struct_decl",
  "struct_members", YY_NULLPTR
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304
};
#endif

#define YYPACT_NINF (-98)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     201,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   135,   -98,    64,    64,    64,    -9,     1,    64,    26,
       9,   201,   -98,    31,    43,   -98,    26,    18,   -98,    28,
     -98,   223,    13,   -98,   -98,   -98,   -98,    49,    26,    26,
     252,   523,   281,    64,    64,   310,    55,   -98,   -98,    76,
      77,    17,   -98,    64,    64,    26,    64,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,    64,    64,    64,    64,   -98,   -98,
     -98,    64,   -98,   -98,    64,    64,    64,   -98,   -98,    60,
     -98,   107,    40,   339,   368,   -98,   135,    57,    58,    61,
     135,   513,   513,    22,    70,   397,   523,   523,   552,   552,
     552,   513,   513,   426,   135,   -98,   154,   -18,    64,    68,
      26,   -98,    10,   -98,   -98,    64,   513,   -98,    47,   -98,
      64,    64,   -98,   -98,    48,   -98,   -98,   -98,   455,   -98,
      72,   -98,   -98,     6,    68,   135,   513,   513,   -98,    64,
     -98,   -98,   -98,   484,    68,   -98
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    36,    37,    38,    15,    16,    17,    18,    19,    20,
      21,     0,    53,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     3,    25,    26,    27,     0,     0,     6,    41,
      43,     0,    52,    39,    10,    11,    12,     0,     0,     0,
       0,    51,     0,     0,     0,     0,    24,     1,     4,     0,
       0,    28,     5,     0,    59,     0,     0,    62,    63,    64,
      65,    66,    67,     8,     0,     0,     0,     0,    68,    69,
      70,     0,    71,    72,     0,     0,     0,     7,    24,     0,
      50,     0,    73,     0,     0,     9,    79,     0,     0,     0,
      33,    56,    60,     0,    42,     0,    48,    49,    46,    47,
      45,    44,    57,     0,    33,    14,     0,     0,     0,     0,
       0,    80,     0,    22,    23,    59,    29,    34,     0,    40,
       0,     0,    54,    55,     0,    13,    74,    75,     0,    77,
      28,    78,    81,     0,     0,     0,    61,    58,    32,     0,
      30,    31,    35,     0,     0,    76
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -98,   -98,    15,   -17,   -97,   -98,   -98,   -98,     3,   -79,
     -98,    -6,   -11,   -98,   -13,   -98,   -98,   -15,   -98,    -4,
     -98,   -98,   -98,   -98
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    20,    21,    22,    82,    23,    24,    25,   110,    27,
      28,   118,    29,    30,    31,    32,    33,    93,    74,    34,
      35,    36,    37,   112
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      40,    41,    42,    26,    48,    45,    81,   111,    46,    47,
     126,   117,   129,    43,    39,    51,     4,     5,     6,     7,
       8,     9,    10,    44,    26,   117,    15,    78,    79,     1,
      83,    84,   120,   132,    75,   131,   140,   141,    89,    90,
      91,    92,    76,    95,    94,   119,    52,   145,   120,    53,
      54,    96,    97,    98,    99,    55,   142,    56,   100,    38,
      49,   101,   102,   103,     1,     2,     3,     1,     2,     3,
     134,   138,    50,   135,   135,    12,   116,    77,    12,    86,
      87,    88,   104,    13,    26,   107,    13,   113,   114,    48,
     115,   121,    81,    89,    14,   128,   106,    14,   124,   130,
     133,     0,    92,   127,     0,     0,     0,   136,   137,    26,
       1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,     0,     0,     0,     0,   143,     0,     0,    13,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
      14,     4,     5,     6,     7,     8,     9,    10,     0,     0,
       0,    15,     0,    16,    17,    18,    19,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,     0,
       0,     0,     0,     0,     0,     0,    13,     0,     0,   125,
       0,     0,     0,     0,    38,     0,     0,    14,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    15,     0,
      16,    17,    18,    19,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,     0,     0,     0,
       0,     0,     0,    13,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,     0,     0,     0,    57,    58,
      59,    60,    61,    62,     0,    15,     0,    16,    17,    18,
      19,    63,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,    72,    73,    57,    58,    59,
      60,    61,    62,     0,     0,    80,     0,     0,     0,     0,
       0,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,    71,     0,     0,    72,    73,    57,    58,    59,    60,
      61,    62,     0,     0,     0,    81,     0,     0,     0,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,    72,    73,    57,    58,    59,    60,    61,
      62,     0,     0,     0,     0,     0,     0,     0,    85,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,    71,
       0,     0,    72,    73,    57,    58,    59,    60,    61,    62,
       0,     0,     0,     0,     0,     0,     0,   108,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,    72,    73,    57,    58,    59,    60,    61,    62,     0,
       0,   109,     0,     0,     0,     0,     0,     0,     0,     0,
      64,    65,    66,    67,    68,    69,    70,    71,     0,     0,
      72,    73,    57,    58,    59,    60,    61,    62,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   122,     0,    64,
      65,    66,    67,    68,    69,    70,    71,     0,     0,    72,
      73,    57,    58,    59,    60,    61,    62,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   123,     0,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,    72,    73,
      57,    58,    59,    60,    61,    62,     0,     0,     0,     0,
       0,     0,     0,   139,     0,     0,     0,    64,    65,    66,
      67,    68,    69,    70,    71,     0,     0,    72,    73,    57,
      58,    59,    60,    61,    62,     0,     0,   144,     0,     0,
       0,     0,     0,     0,     0,     0,    64,    65,    66,    67,
      68,    69,    70,    71,     0,     0,    72,    73,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,    57,    58,
      59,    60,    61,    62,     0,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,    72,    73,    66,    67,    68,
      69,    70,    71,     0,     0,    72,    73,    57,    58,    59,
      60,    61,    62,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    68,    69,
      70,     0,     0,     0,    72,    73
};

static const yytype_int16 yycheck[] =
{
      13,    14,    15,     0,    21,    18,    24,    86,    19,     0,
     107,    90,   109,    22,    11,    26,     6,     7,     8,     9,
      10,    11,    12,    22,    21,   104,    44,    38,    39,     3,
      43,    44,    26,   112,    21,    25,    30,   134,    21,    22,
      53,    54,    29,    56,    55,    23,    28,   144,    26,    21,
      22,    64,    65,    66,    67,    27,   135,    29,    71,    49,
      29,    74,    75,    76,     3,     4,     5,     3,     4,     5,
      23,    23,    29,    26,    26,    14,    89,    28,    14,    24,
       4,     4,    22,    22,    81,    45,    22,    30,    30,   106,
      29,    21,    24,    21,    33,   108,    81,    33,   104,   110,
     115,    -1,   115,   107,    -1,    -1,    -1,   120,   121,   106,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    -1,    -1,    -1,    -1,   139,    -1,    -1,    22,
      -1,    -1,    25,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      33,     6,     7,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    44,    -1,    46,    47,    48,    49,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    25,
      -1,    -1,    -1,    -1,    49,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,
      46,    47,    48,    49,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    20,    -1,    44,    -1,    46,    47,    48,
      49,    28,    -1,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    42,    43,    15,    16,    17,
      18,    19,    20,    -1,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    33,    34,    35,    36,    37,
      38,    39,    -1,    -1,    42,    43,    15,    16,    17,    18,
      19,    20,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    42,    43,    15,    16,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    42,    43,    15,    16,    17,    18,    19,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    42,    43,    15,    16,    17,    18,    19,    20,    -1,
      -1,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      42,    43,    15,    16,    17,    18,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    42,
      43,    15,    16,    17,    18,    19,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    -1,    -1,    42,    43,
      15,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,    42,    43,    15,
      16,    17,    18,    19,    20,    -1,    -1,    23,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    42,    43,    15,    16,
      17,    18,    19,    20,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    20,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    42,    43,    34,    35,    36,
      37,    38,    39,    -1,    -1,    42,    43,    15,    16,    17,
      18,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      38,    -1,    -1,    -1,    42,    43
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    22,    33,    44,    46,    47,    48,    49,
      51,    52,    53,    55,    56,    57,    58,    59,    60,    62,
      63,    64,    65,    66,    69,    70,    71,    72,    49,    58,
      64,    64,    64,    22,    22,    64,    62,     0,    53,    29,
      29,    62,    28,    21,    22,    27,    29,    15,    16,    17,
      18,    19,    20,    28,    32,    33,    34,    35,    36,    37,
      38,    39,    42,    43,    68,    21,    29,    28,    62,    62,
      23,    24,    54,    64,    64,    28,    24,     4,     4,    21,
      22,    64,    64,    67,    62,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    22,    25,    52,    45,    28,    23,
      58,    59,    73,    30,    30,    29,    64,    59,    61,    23,
      26,    21,    30,    30,    61,    25,    54,    69,    64,    54,
      62,    25,    59,    67,    23,    26,    64,    64,    23,    28,
      30,    54,    59,    64,    23,    54
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    50,    51,    52,    52,    53,    53,    53,    53,    53,
      53,    53,    53,    54,    54,    55,    55,    55,    55,    55,
      55,    55,    56,    56,    57,    58,    58,    58,    59,    59,
      59,    60,    60,    61,    61,    61,    62,    63,    63,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    65,    65,    66,    66,    66,    67,
      67,    67,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    69,    69,    69,    70,    71,    72,    73,
      73,    73
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     2,     1,     2,     2,     3,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       1,     1,     4,     4,     2,     1,     1,     1,     2,     4,
       6,     6,     6,     0,     1,     3,     1,     1,     1,     1,
       4,     1,     3,     1,     3,     3,     3,     3,     3,     3,
       3,     2,     1,     1,     4,     4,     3,     3,     5,     0,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     5,     5,     9,     5,     5,     0,
       1,     2
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
#line 44 "grammar.y"
                { programBlock = (yyvsp[0].block); }
#line 1443 "grammar.cpp"
    break;

  case 3:
#line 46 "grammar.y"
             { (yyval.block) = new NBlock(); (yyval.block)->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt))); }
#line 1449 "grammar.cpp"
    break;

  case 4:
#line 47 "grammar.y"
                                     { (yyvsp[-1].block)->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt))); }
#line 1455 "grammar.cpp"
    break;

  case 8:
#line 50 "grammar.y"
                                   { (yyval.stmt) = new NExpressionStatement(shared_ptr<NExpression>((yyvsp[-1].expr))); }
#line 1461 "grammar.cpp"
    break;

  case 9:
#line 51 "grammar.y"
                                           { (yyval.stmt) = new NReturnStatement(shared_ptr<NExpression>((yyvsp[-1].expr))); }
#line 1467 "grammar.cpp"
    break;

  case 13:
#line 57 "grammar.y"
                              { (yyval.block) = (yyvsp[-1].block); }
#line 1473 "grammar.cpp"
    break;

  case 14:
#line 58 "grammar.y"
                                          { (yyval.block) = new NBlock(); }
#line 1479 "grammar.cpp"
    break;

  case 15:
#line 61 "grammar.y"
                         { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true;  delete (yyvsp[0].string); }
#line 1485 "grammar.cpp"
    break;

  case 16:
#line 62 "grammar.y"
                                                   { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1491 "grammar.cpp"
    break;

  case 17:
#line 63 "grammar.y"
                                                  { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1497 "grammar.cpp"
    break;

  case 18:
#line 64 "grammar.y"
                                                 { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1503 "grammar.cpp"
    break;

  case 19:
#line 65 "grammar.y"
                                                 { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1509 "grammar.cpp"
    break;

  case 20:
#line 66 "grammar.y"
                                                 { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1515 "grammar.cpp"
    break;

  case 21:
#line 67 "grammar.y"
                                                   { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); (yyval.ident)->isType = true; delete (yyvsp[0].string); }
#line 1521 "grammar.cpp"
    break;

  case 22:
#line 69 "grammar.y"
                                                               { 
					(yyvsp[-3].ident)->isArray = true; 
					(yyvsp[-3].ident)->arraySize->push_back(make_shared<NInteger>(atol((yyvsp[-1].string)->c_str()))); 
					(yyval.ident) = (yyvsp[-3].ident); 
				}
#line 1531 "grammar.cpp"
    break;

  case 23:
#line 74 "grammar.y"
                                                                              {
					(yyvsp[-3].ident)->arraySize->push_back(make_shared<NInteger>(atol((yyvsp[-1].string)->c_str())));
					(yyval.ident) = (yyvsp[-3].ident);
				}
#line 1540 "grammar.cpp"
    break;

  case 24:
#line 79 "grammar.y"
                                {
				(yyvsp[0].ident)->isType = true;
				(yyval.ident) = (yyvsp[0].ident);
			}
#line 1549 "grammar.cpp"
    break;

  case 25:
#line 84 "grammar.y"
                            { (yyval.ident) = (yyvsp[0].ident); }
#line 1555 "grammar.cpp"
    break;

  case 26:
#line 85 "grammar.y"
                                         { (yyval.ident) = (yyvsp[0].ident); }
#line 1561 "grammar.cpp"
    break;

  case 27:
#line 86 "grammar.y"
                                          { (yyval.ident) = (yyvsp[0].ident); }
#line 1567 "grammar.cpp"
    break;

  case 28:
#line 88 "grammar.y"
                          { (yyval.stmt) = new NVariableDeclaration(shared_ptr<NIdentifier>((yyvsp[-1].ident)), shared_ptr<NIdentifier>((yyvsp[0].ident)), nullptr); }
#line 1573 "grammar.cpp"
    break;

  case 29:
#line 89 "grammar.y"
                                                              { (yyval.stmt) = new NVariableDeclaration(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<NIdentifier>((yyvsp[-2].ident)), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1579 "grammar.cpp"
    break;

  case 30:
#line 90 "grammar.y"
                                                                                       {
					 (yyval.stmt) = new NArrayInitialization(make_shared<NVariableDeclaration>(shared_ptr<NIdentifier>((yyvsp[-5].ident)), shared_ptr<NIdentifier>((yyvsp[-4].ident)), nullptr), shared_ptr<ExpressionList>((yyvsp[-1].exprvec)));
				 }
#line 1587 "grammar.cpp"
    break;

  case 31:
#line 96 "grammar.y"
                                { (yyval.stmt) = new NFunctionDeclaration(shared_ptr<NIdentifier>((yyvsp[-5].ident)), shared_ptr<NIdentifier>((yyvsp[-4].ident)), shared_ptr<VariableList>((yyvsp[-2].varvec)), shared_ptr<NBlock>((yyvsp[0].block)));  }
#line 1593 "grammar.cpp"
    break;

  case 32:
#line 97 "grammar.y"
                                                                                { (yyval.stmt) = new NFunctionDeclaration(shared_ptr<NIdentifier>((yyvsp[-4].ident)), shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<VariableList>((yyvsp[-1].varvec)), nullptr, true); }
#line 1599 "grammar.cpp"
    break;

  case 33:
#line 99 "grammar.y"
                             { (yyval.varvec) = new VariableList(); }
#line 1605 "grammar.cpp"
    break;

  case 34:
#line 100 "grammar.y"
                                                                    { (yyval.varvec) = new VariableList(); (yyval.varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 1611 "grammar.cpp"
    break;

  case 35:
#line 101 "grammar.y"
                                                                                          { (yyvsp[-2].varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 1617 "grammar.cpp"
    break;

  case 36:
#line 104 "grammar.y"
                    { (yyval.ident) = new NIdentifier(*(yyvsp[0].string)); delete (yyvsp[0].string); }
#line 1623 "grammar.cpp"
    break;

  case 37:
#line 107 "grammar.y"
                   { (yyval.expr) = new NInteger(atol((yyvsp[0].string)->c_str())); }
#line 1629 "grammar.cpp"
    break;

  case 38:
#line 108 "grammar.y"
                                          { (yyval.expr) = new NDouble(atof((yyvsp[0].string)->c_str())); }
#line 1635 "grammar.cpp"
    break;

  case 39:
#line 110 "grammar.y"
               { (yyval.expr) = (yyvsp[0].expr); }
#line 1641 "grammar.cpp"
    break;

  case 40:
#line 111 "grammar.y"
                                                   { (yyval.expr) = new NMethodCall(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<ExpressionList>((yyvsp[-1].exprvec))); }
#line 1647 "grammar.cpp"
    break;

  case 41:
#line 112 "grammar.y"
                         { (yyval.ident) = (yyvsp[0].ident); }
#line 1653 "grammar.cpp"
    break;

  case 42:
#line 113 "grammar.y"
                                    { (yyval.expr) = new NStructMember(shared_ptr<NIdentifier>((yyvsp[-2].ident)), shared_ptr<NIdentifier>((yyvsp[0].ident))); }
#line 1659 "grammar.cpp"
    break;

  case 44:
#line 115 "grammar.y"
                                        { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1665 "grammar.cpp"
    break;

  case 45:
#line 116 "grammar.y"
                                  { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1671 "grammar.cpp"
    break;

  case 46:
#line 117 "grammar.y"
                                  { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1677 "grammar.cpp"
    break;

  case 47:
#line 118 "grammar.y"
                                  { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1683 "grammar.cpp"
    break;

  case 48:
#line 119 "grammar.y"
                                   { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1689 "grammar.cpp"
    break;

  case 49:
#line 120 "grammar.y"
                                    { (yyval.expr) = new NBinaryOperator(shared_ptr<NExpression>((yyvsp[-2].expr)), (yyvsp[-1].token), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1695 "grammar.cpp"
    break;

  case 50:
#line 121 "grammar.y"
                                        { (yyval.expr) = (yyvsp[-1].expr); }
#line 1701 "grammar.cpp"
    break;

  case 51:
#line 122 "grammar.y"
                               { (yyval.expr) = nullptr; /* TODO */ }
#line 1707 "grammar.cpp"
    break;

  case 52:
#line 123 "grammar.y"
                               { (yyval.expr) = (yyvsp[0].index); }
#line 1713 "grammar.cpp"
    break;

  case 53:
#line 124 "grammar.y"
                            { (yyval.expr) = new NLiteral(*(yyvsp[0].string)); delete (yyvsp[0].string); }
#line 1719 "grammar.cpp"
    break;

  case 54:
#line 128 "grammar.y"
                                { (yyval.index) = new NArrayIndex(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<NExpression>((yyvsp[-1].expr))); }
#line 1725 "grammar.cpp"
    break;

  case 55:
#line 130 "grammar.y"
                                        { 	
						(yyvsp[-3].index)->expressions->push_back(shared_ptr<NExpression>((yyvsp[-1].expr)));
						(yyval.index) = (yyvsp[-3].index);
					}
#line 1734 "grammar.cpp"
    break;

  case 56:
#line 134 "grammar.y"
                           { (yyval.expr) = new NAssignment(shared_ptr<NIdentifier>((yyvsp[-2].ident)), shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1740 "grammar.cpp"
    break;

  case 57:
#line 135 "grammar.y"
                                                  {
				(yyval.expr) = new NArrayAssignment(shared_ptr<NArrayIndex>((yyvsp[-2].index)), shared_ptr<NExpression>((yyvsp[0].expr)));
			}
#line 1748 "grammar.cpp"
    break;

  case 58:
#line 138 "grammar.y"
                                                       {
				auto member = make_shared<NStructMember>(shared_ptr<NIdentifier>((yyvsp[-4].ident)), shared_ptr<NIdentifier>((yyvsp[-2].ident))); 
				(yyval.expr) = new NStructAssignment(member, shared_ptr<NExpression>((yyvsp[0].expr))); 
			}
#line 1757 "grammar.cpp"
    break;

  case 59:
#line 144 "grammar.y"
                        { (yyval.exprvec) = new ExpressionList(); }
#line 1763 "grammar.cpp"
    break;

  case 60:
#line 145 "grammar.y"
                                               { (yyval.exprvec) = new ExpressionList(); (yyval.exprvec)->push_back(shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1769 "grammar.cpp"
    break;

  case 61:
#line 146 "grammar.y"
                                                                { (yyvsp[-2].exprvec)->push_back(shared_ptr<NExpression>((yyvsp[0].expr))); }
#line 1775 "grammar.cpp"
    break;

  case 73:
#line 150 "grammar.y"
                         { (yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[-1].expr)), shared_ptr<NBlock>((yyvsp[0].block))); }
#line 1781 "grammar.cpp"
    break;

  case 74:
#line 151 "grammar.y"
                                             { (yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[-3].expr)), shared_ptr<NBlock>((yyvsp[-2].block)), shared_ptr<NBlock>((yyvsp[0].block))); }
#line 1787 "grammar.cpp"
    break;

  case 75:
#line 152 "grammar.y"
                                               { 
			auto blk = new NBlock(); 
			blk->child->push_back(shared_ptr<NStatement>((yyvsp[0].stmt)));
			(yyval.stmt) = new NIfStatement(shared_ptr<NExpression>((yyvsp[-3].expr)), shared_ptr<NBlock>((yyvsp[-2].block)), shared_ptr<NBlock>(blk)); 
		}
#line 1797 "grammar.cpp"
    break;

  case 76:
#line 158 "grammar.y"
                                                                           { (yyval.stmt) = new NForStatement(shared_ptr<NBlock>((yyvsp[0].block)), shared_ptr<NExpression>((yyvsp[-6].expr)), shared_ptr<NExpression>((yyvsp[-4].expr)), shared_ptr<NExpression>((yyvsp[-2].expr))); }
#line 1803 "grammar.cpp"
    break;

  case 77:
#line 160 "grammar.y"
                                               { (yyval.stmt) = new NForStatement(shared_ptr<NBlock>((yyvsp[0].block)), nullptr, shared_ptr<NExpression>((yyvsp[-2].expr)), nullptr); }
#line 1809 "grammar.cpp"
    break;

  case 78:
#line 162 "grammar.y"
                                                           {(yyval.stmt) = new NStructDeclaration(shared_ptr<NIdentifier>((yyvsp[-3].ident)), shared_ptr<VariableList>((yyvsp[-1].varvec))); }
#line 1815 "grammar.cpp"
    break;

  case 79:
#line 164 "grammar.y"
                             { (yyval.varvec) = new VariableList(); }
#line 1821 "grammar.cpp"
    break;

  case 80:
#line 165 "grammar.y"
                                           { (yyval.varvec) = new VariableList(); (yyval.varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 1827 "grammar.cpp"
    break;

  case 81:
#line 166 "grammar.y"
                                                          { (yyvsp[-1].varvec)->push_back(shared_ptr<NVariableDeclaration>((yyvsp[0].var_decl))); }
#line 1833 "grammar.cpp"
    break;


#line 1837 "grammar.cpp"

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

#line 168 "grammar.y"

