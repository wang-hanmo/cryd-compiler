/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"

    #include <iostream>
    #include <cstdio>
    #include <cstring>
    #include <ast.h>
    #include <lexer.h>
    #include <type_define.h>
    void yyerror(char * msg);
    void yy_warning(char *msg);
    extern int line_no;

#line 83 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"

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

#include "parser.yy.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INTCONST = 3,                   /* INTCONST  */
  YYSYMBOL_FLOATCONST = 4,                 /* FLOATCONST  */
  YYSYMBOL_IDENT = 5,                      /* IDENT  */
  YYSYMBOL_IF = 6,                         /* IF  */
  YYSYMBOL_ELSE = 7,                       /* ELSE  */
  YYSYMBOL_WHILE = 8,                      /* WHILE  */
  YYSYMBOL_BREAK = 9,                      /* BREAK  */
  YYSYMBOL_CONTINUE = 10,                  /* CONTINUE  */
  YYSYMBOL_RETURN = 11,                    /* RETURN  */
  YYSYMBOL_INT = 12,                       /* INT  */
  YYSYMBOL_FLOAT = 13,                     /* FLOAT  */
  YYSYMBOL_VOID = 14,                      /* VOID  */
  YYSYMBOL_CONST = 15,                     /* CONST  */
  YYSYMBOL_L_PAREN = 16,                   /* L_PAREN  */
  YYSYMBOL_R_PAREN = 17,                   /* R_PAREN  */
  YYSYMBOL_L_BRACK = 18,                   /* L_BRACK  */
  YYSYMBOL_R_BRACK = 19,                   /* R_BRACK  */
  YYSYMBOL_L_BRACE = 20,                   /* L_BRACE  */
  YYSYMBOL_R_BRACE = 21,                   /* R_BRACE  */
  YYSYMBOL_SEMI = 22,                      /* SEMI  */
  YYSYMBOL_COMMA = 23,                     /* COMMA  */
  YYSYMBOL_ASSIGN = 24,                    /* ASSIGN  */
  YYSYMBOL_ADD = 25,                       /* ADD  */
  YYSYMBOL_SUB = 26,                       /* SUB  */
  YYSYMBOL_MUL = 27,                       /* MUL  */
  YYSYMBOL_DIV = 28,                       /* DIV  */
  YYSYMBOL_MOD = 29,                       /* MOD  */
  YYSYMBOL_AND = 30,                       /* AND  */
  YYSYMBOL_OR = 31,                        /* OR  */
  YYSYMBOL_NOT = 32,                       /* NOT  */
  YYSYMBOL_GT = 33,                        /* GT  */
  YYSYMBOL_LT = 34,                        /* LT  */
  YYSYMBOL_LE = 35,                        /* LE  */
  YYSYMBOL_GE = 36,                        /* GE  */
  YYSYMBOL_EQ = 37,                        /* EQ  */
  YYSYMBOL_NEQ = 38,                       /* NEQ  */
  YYSYMBOL_YYACCEPT = 39,                  /* $accept  */
  YYSYMBOL_CompUnit = 40,                  /* CompUnit  */
  YYSYMBOL_Decl = 41,                      /* Decl  */
  YYSYMBOL_ConstDecl = 42,                 /* ConstDecl  */
  YYSYMBOL_BType = 43,                     /* BType  */
  YYSYMBOL_ConstDef = 44,                  /* ConstDef  */
  YYSYMBOL_ConstArrayIdent = 45,           /* ConstArrayIdent  */
  YYSYMBOL_ConstInitValArray = 46,         /* ConstInitValArray  */
  YYSYMBOL_ConstInitValArrayList = 47,     /* ConstInitValArrayList  */
  YYSYMBOL_VarDecl = 48,                   /* VarDecl  */
  YYSYMBOL_VarDef = 49,                    /* VarDef  */
  YYSYMBOL_InitValArray = 50,              /* InitValArray  */
  YYSYMBOL_InitValArrayList = 51,          /* InitValArrayList  */
  YYSYMBOL_FuncDef = 52,                   /* FuncDef  */
  YYSYMBOL_FuncFParams = 53,               /* FuncFParams  */
  YYSYMBOL_FuncFParam = 54,                /* FuncFParam  */
  YYSYMBOL_FuncFParamArray = 55,           /* FuncFParamArray  */
  YYSYMBOL_Block = 56,                     /* Block  */
  YYSYMBOL_BlockItemList = 57,             /* BlockItemList  */
  YYSYMBOL_BlockItem = 58,                 /* BlockItem  */
  YYSYMBOL_Stmt = 59,                      /* Stmt  */
  YYSYMBOL_Exp = 60,                       /* Exp  */
  YYSYMBOL_Cond = 61,                      /* Cond  */
  YYSYMBOL_LVal = 62,                      /* LVal  */
  YYSYMBOL_ArrayIdent = 63,                /* ArrayIdent  */
  YYSYMBOL_PrimaryExp = 64,                /* PrimaryExp  */
  YYSYMBOL_Number = 65,                    /* Number  */
  YYSYMBOL_UnaryExp = 66,                  /* UnaryExp  */
  YYSYMBOL_UnaryOp = 67,                   /* UnaryOp  */
  YYSYMBOL_FuncRParams = 68,               /* FuncRParams  */
  YYSYMBOL_MulExp = 69,                    /* MulExp  */
  YYSYMBOL_AddExp = 70,                    /* AddExp  */
  YYSYMBOL_RelExp = 71,                    /* RelExp  */
  YYSYMBOL_EqExp = 72,                     /* EqExp  */
  YYSYMBOL_LAndExp = 73,                   /* LAndExp  */
  YYSYMBOL_LOrExp = 74,                    /* LOrExp  */
  YYSYMBOL_ConstExp = 75                   /* ConstExp  */
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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   275

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  39
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  186

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   293


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
      35,    36,    37,    38
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    55,    55,    58,    61,    62,    66,    67,    71,    75,
      82,    83,    87,    88,    92,    95,   100,   101,   105,   107,
     109,   111,   116,   120,   127,   128,   129,   130,   134,   135,
     139,   141,   143,   145,   150,   152,   154,   156,   161,   162,
     166,   167,   172,   173,   174,   179,   180,   184,   185,   189,
     190,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   208,   212,   216,   217,   221,   222,   226,   227,
     228,   232,   233,   237,   238,   239,   240,   244,   245,   246,
     250,   251,   255,   256,   257,   258,   262,   263,   264,   268,
     269,   270,   271,   272,   276,   277,   278,   282,   283,   287,
     288,   292
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
  "\"end of file\"", "error", "\"invalid token\"", "INTCONST",
  "FLOATCONST", "IDENT", "IF", "ELSE", "WHILE", "BREAK", "CONTINUE",
  "RETURN", "INT", "FLOAT", "VOID", "CONST", "L_PAREN", "R_PAREN",
  "L_BRACK", "R_BRACK", "L_BRACE", "R_BRACE", "SEMI", "COMMA", "ASSIGN",
  "ADD", "SUB", "MUL", "DIV", "MOD", "AND", "OR", "NOT", "GT", "LT", "LE",
  "GE", "EQ", "NEQ", "$accept", "CompUnit", "Decl", "ConstDecl", "BType",
  "ConstDef", "ConstArrayIdent", "ConstInitValArray",
  "ConstInitValArrayList", "VarDecl", "VarDef", "InitValArray",
  "InitValArrayList", "FuncDef", "FuncFParams", "FuncFParam",
  "FuncFParamArray", "Block", "BlockItemList", "BlockItem", "Stmt", "Exp",
  "Cond", "LVal", "ArrayIdent", "PrimaryExp", "Number", "UnaryExp",
  "UnaryOp", "FuncRParams", "MulExp", "AddExp", "RelExp", "EqExp",
  "LAndExp", "LOrExp", "ConstExp", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-151)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     119,  -151,  -151,     5,    91,    17,  -151,   135,    62,   141,
    -151,    67,    82,  -151,  -151,  -151,  -151,    82,     0,    -4,
    -151,  -151,   108,   111,    18,  -151,    36,  -151,   113,   241,
     241,   241,    97,    48,  -151,   101,   122,    54,  -151,   129,
     241,   109,   101,    63,  -151,  -151,     7,   241,  -151,  -151,
    -151,  -151,   136,  -151,  -151,  -151,   241,    69,   163,   148,
    -151,   163,   160,    90,  -151,   140,  -151,   176,   101,    91,
     241,  -151,   179,  -151,  -151,   101,    23,   241,   142,   241,
    -151,   241,   241,   241,   241,   241,  -151,  -151,  -151,  -151,
      12,  -151,   180,   205,   209,   210,   198,  -151,  -151,  -151,
     108,  -151,   165,  -151,  -151,   211,   174,   203,  -151,  -151,
     215,  -151,  -151,    86,  -151,  -151,  -151,  -151,    65,   217,
    -151,   218,  -151,  -151,  -151,    69,    69,  -151,   222,   241,
     241,  -151,  -151,  -151,   221,  -151,  -151,  -151,   241,  -151,
     230,  -151,  -151,   236,  -151,   241,  -151,  -151,  -151,  -151,
     233,   163,   182,   155,   223,   220,   238,  -151,   237,  -151,
    -151,  -151,  -151,    53,   241,   241,   241,   241,   241,   241,
     241,   241,    53,  -151,   251,   163,   163,   163,   163,   182,
     182,   155,   223,  -151,    53,  -151
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    10,    11,     0,     0,     0,     2,     0,     0,     0,
       3,     0,     0,     1,     4,     5,     6,     0,    24,    25,
      22,     7,     0,     0,     0,     8,     0,     9,     0,     0,
       0,     0,     0,    24,    23,     0,     0,     0,    38,    41,
       0,     0,     0,     0,    71,    72,    64,     0,    77,    78,
      79,    69,    65,    73,    70,    82,     0,    86,   101,     0,
      26,    62,     0,     0,    27,     0,    35,    40,     0,     0,
       0,    12,     0,    13,    34,     0,     0,     0,     0,     0,
      76,     0,     0,     0,     0,     0,    14,    15,    28,    30,
       0,    32,     0,     0,     0,     0,     0,    45,    52,    49,
       0,    54,     0,    47,    50,     0,    69,     0,    37,    39,
       0,    16,    18,     0,    20,    36,    74,    80,     0,     0,
      68,     0,    83,    84,    85,    87,    88,    29,     0,     0,
       0,    58,    59,    60,     0,    46,    48,    53,     0,    42,
       0,    44,    17,     0,    75,     0,    66,    67,    31,    33,
       0,    89,    94,    97,    99,    63,     0,    61,     0,    43,
      19,    21,    81,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,    55,    91,    90,    92,    93,    95,
      96,    98,   100,    57,     0,    56
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -151,  -151,    41,  -151,     3,   243,    72,   -67,  -151,  -151,
     242,   -54,  -151,   258,   244,   196,  -151,   -23,  -151,   167,
    -150,   -26,   144,   -64,  -151,  -151,  -151,     9,  -151,  -151,
     125,   -29,    44,   100,   104,  -151,   -25
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     5,    99,     7,    36,    25,    19,    73,   113,     9,
      20,    64,    90,    10,    37,    38,    39,   101,   102,   103,
     104,   105,   150,    51,    52,    53,    54,    55,    56,   118,
      57,    61,   152,   153,   154,   155,    59
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      58,   106,    58,     8,    60,   112,    62,    12,     8,    89,
      11,    58,    66,   174,    31,    71,    28,    13,    29,    74,
      32,    78,   183,    76,    30,    77,    44,    45,    46,     1,
       2,     3,     4,   127,   185,   128,    29,    91,   106,    47,
     116,     6,    40,    58,   110,   108,    14,   114,    48,    49,
     117,   119,   115,   121,    31,    50,    44,    45,    46,    92,
      41,    93,    94,    95,    96,    80,    29,    18,   100,    47,
     134,    68,    30,    65,   148,    98,   160,    69,    48,    49,
      75,   140,   144,    23,    26,    50,    69,    24,   145,    26,
     122,   123,   124,    44,    45,    46,    81,    82,    83,   106,
     151,   151,   149,     1,     2,   100,    47,   142,   106,   143,
      63,    88,   158,    33,    58,    48,    49,    63,   161,   162,
     106,    65,    50,     1,     2,     1,     2,    67,    35,    72,
      42,     1,     2,     3,     4,   175,   176,   177,   178,   151,
     151,   151,   151,    44,    45,    46,    92,    70,    93,    94,
      95,    96,     1,     2,    79,     4,    47,    16,    17,   120,
      65,    97,    98,    21,    22,    48,    49,    86,    44,    45,
      46,    92,    50,    93,    94,    95,    96,     1,     2,    87,
       4,    47,    44,    45,    46,    65,   135,    98,    84,    85,
      48,    49,   168,   169,   107,    47,   129,    50,   138,    72,
     111,    44,    45,    46,    48,    49,    44,    45,    46,   125,
     126,    50,   179,   180,    47,   164,   165,   166,   167,    47,
     133,   130,   139,    48,    49,    44,    45,    46,    48,    49,
      50,   131,   132,   137,   141,    50,   146,   147,    47,    44,
      45,    46,    63,   157,    44,    45,    46,    48,    49,   159,
     163,   171,    47,   170,    50,   172,    72,    47,   184,   173,
      27,    48,    49,    15,    34,   109,    48,    49,    50,   136,
     181,     0,    43,    50,   156,   182
};

static const yytype_int16 yycheck[] =
{
      29,    65,    31,     0,    30,    72,    31,     4,     5,    63,
       5,    40,    35,   163,    18,    40,    16,     0,    18,    42,
      24,    47,   172,    16,    24,    18,     3,     4,     5,    12,
      13,    14,    15,    21,   184,    23,    18,    63,   102,    16,
      17,     0,    24,    72,    70,    68,     5,    72,    25,    26,
      76,    77,    75,    79,    18,    32,     3,     4,     5,     6,
      24,     8,     9,    10,    11,    56,    18,     5,    65,    16,
      96,    17,    24,    20,   128,    22,   143,    23,    25,    26,
      17,   107,    17,    16,    12,    32,    23,     5,    23,    17,
      81,    82,    83,     3,     4,     5,    27,    28,    29,   163,
     129,   130,   128,    12,    13,   102,    16,    21,   172,    23,
      20,    21,   138,     5,   143,    25,    26,    20,   143,   145,
     184,    20,    32,    12,    13,    12,    13,     5,    17,    20,
      17,    12,    13,    14,    15,   164,   165,   166,   167,   168,
     169,   170,   171,     3,     4,     5,     6,    18,     8,     9,
      10,    11,    12,    13,    18,    15,    16,    22,    23,    17,
      20,    21,    22,    22,    23,    25,    26,    19,     3,     4,
       5,     6,    32,     8,     9,    10,    11,    12,    13,    19,
      15,    16,     3,     4,     5,    20,    21,    22,    25,    26,
      25,    26,    37,    38,    18,    16,    16,    32,    24,    20,
      21,     3,     4,     5,    25,    26,     3,     4,     5,    84,
      85,    32,   168,   169,    16,    33,    34,    35,    36,    16,
      22,    16,    19,    25,    26,     3,     4,     5,    25,    26,
      32,    22,    22,    22,    19,    32,    19,    19,    16,     3,
       4,     5,    20,    22,     3,     4,     5,    25,    26,    19,
      17,    31,    16,    30,    32,    17,    20,    16,     7,    22,
      17,    25,    26,     5,    22,    69,    25,    26,    32,   102,
     170,    -1,    28,    32,   130,   171
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    12,    13,    14,    15,    40,    41,    42,    43,    48,
      52,     5,    43,     0,    41,    52,    22,    23,     5,    45,
      49,    22,    23,    16,     5,    44,    45,    44,    16,    18,
      24,    18,    24,     5,    49,    17,    43,    53,    54,    55,
      24,    24,    17,    53,     3,     4,     5,    16,    25,    26,
      32,    62,    63,    64,    65,    66,    67,    69,    70,    75,
      60,    70,    75,    20,    50,    20,    56,     5,    17,    23,
      18,    75,    20,    46,    56,    17,    16,    18,    60,    18,
      66,    27,    28,    29,    25,    26,    19,    19,    21,    50,
      51,    60,     6,     8,     9,    10,    11,    21,    22,    41,
      43,    56,    57,    58,    59,    60,    62,    18,    56,    54,
      60,    21,    46,    47,    75,    56,    17,    60,    68,    60,
      17,    60,    66,    66,    66,    69,    69,    21,    23,    16,
      16,    22,    22,    22,    60,    21,    58,    22,    24,    19,
      60,    19,    21,    23,    17,    23,    19,    19,    50,    60,
      61,    70,    71,    72,    73,    74,    61,    22,    60,    19,
      46,    75,    60,    17,    33,    34,    35,    36,    37,    38,
      30,    31,    17,    22,    59,    70,    70,    70,    70,    71,
      71,    72,    73,    59,     7,    59
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    39,    40,    40,    40,    40,    41,    41,    42,    42,
      43,    43,    44,    44,    45,    45,    46,    46,    47,    47,
      47,    47,    48,    48,    49,    49,    49,    49,    50,    50,
      51,    51,    51,    51,    52,    52,    52,    52,    53,    53,
      54,    54,    55,    55,    55,    56,    56,    57,    57,    58,
      58,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    60,    61,    62,    62,    63,    63,    64,    64,
      64,    65,    65,    66,    66,    66,    66,    67,    67,    67,
      68,    68,    69,    69,    69,    69,    70,    70,    70,    71,
      71,    71,    71,    71,    72,    72,    72,    73,    73,    74,
      74,    75
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     2,     2,     2,     3,     3,
       1,     1,     3,     3,     4,     4,     2,     3,     1,     3,
       1,     3,     2,     3,     1,     1,     3,     3,     2,     3,
       1,     3,     1,     3,     5,     5,     6,     6,     1,     3,
       2,     1,     4,     5,     4,     2,     3,     1,     2,     1,
       1,     4,     1,     2,     1,     5,     7,     5,     2,     2,
       2,     3,     1,     1,     1,     1,     4,     4,     3,     1,
       1,     1,     1,     1,     3,     4,     2,     1,     1,     1,
       1,     3,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     1,
       3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
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
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
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
  case 2: /* CompUnit: Decl  */
#line 55 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_comp_unit();
                                                            (yyval.ast_node)->add_child((yyvsp[0].ast_node));
                                                            g_ast_root = (yyval.ast_node);}
#line 1292 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 3: /* CompUnit: FuncDef  */
#line 58 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_comp_unit();
                                                            (yyval.ast_node)->add_child((yyvsp[0].ast_node));
                                                            g_ast_root = (yyval.ast_node);}
#line 1300 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 4: /* CompUnit: CompUnit Decl  */
#line 61 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1306 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 5: /* CompUnit: CompUnit FuncDef  */
#line 62 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1312 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 6: /* Decl: ConstDecl SEMI  */
#line 66 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1318 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 7: /* Decl: VarDecl SEMI  */
#line 67 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1324 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 8: /* ConstDecl: CONST BType ConstDef  */
#line 71 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_decl_stmt(ValueType((BasicType)(yyvsp[-1].value_type)));
                                                         (yyval.ast_node)->add_child(ASTNode::create_const_decl(ValueType((BasicType)(yyvsp[-1].value_type)),(yyvsp[0].ast_node)->get_var_name(),(yyvsp[0].ast_node)->get_value_type().is_array(), (yyvsp[0].ast_node)->get_child()));
                                                         delete (yyvsp[0].ast_node);
                                                        }
#line 1333 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 9: /* ConstDecl: ConstDecl COMMA ConstDef  */
#line 75 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node);
                                                         (yyval.ast_node)->add_child(ASTNode::create_const_decl((yyvsp[-2].ast_node)->get_value_type(),(yyvsp[0].ast_node)->get_var_name(),(yyvsp[0].ast_node)->get_value_type().is_array(), (yyvsp[0].ast_node)->get_child()));
                                                         delete (yyvsp[0].ast_node);
                                                        }
#line 1342 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 10: /* BType: INT  */
#line 82 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.value_type) = (int)BasicType::Int;}
#line 1348 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 11: /* BType: FLOAT  */
#line 83 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.value_type) = (int)BasicType::Float;}
#line 1354 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 12: /* ConstDef: IDENT ASSIGN ConstExp  */
#line 87 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_const_decl(ValueType(BasicType::Uncertain),*(yyvsp[-2].string_const_val), false, {(yyvsp[0].ast_node)});}
#line 1360 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 13: /* ConstDef: ConstArrayIdent ASSIGN ConstInitValArray  */
#line 88 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_const_decl(ValueType(BasicType::Uncertain),(yyvsp[-2].ast_node)->get_var_name(), true, {(yyvsp[-2].ast_node),(yyvsp[0].ast_node)});}
#line 1366 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 14: /* ConstArrayIdent: IDENT L_BRACK ConstExp R_BRACK  */
#line 92 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_size();
                                                         (yyval.ast_node)->set_var_name(*(yyvsp[-3].string_const_val));
                                                         (yyval.ast_node)->add_child((yyvsp[-1].ast_node));}
#line 1374 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 15: /* ConstArrayIdent: ConstArrayIdent L_BRACK ConstExp R_BRACK  */
#line 95 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-3].ast_node);
                                                         (yyval.ast_node)->add_child((yyvsp[-1].ast_node));}
#line 1381 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 16: /* ConstInitValArray: L_BRACE R_BRACE  */
#line 100 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain));}
#line 1387 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 17: /* ConstInitValArray: L_BRACE ConstInitValArrayList R_BRACE  */
#line 101 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1393 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 18: /* ConstInitValArrayList: ConstInitValArray  */
#line 105 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1400 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 19: /* ConstInitValArrayList: ConstInitValArrayList COMMA ConstInitValArray  */
#line 107 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1407 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 20: /* ConstInitValArrayList: ConstExp  */
#line 109 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1414 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 21: /* ConstInitValArrayList: ConstInitValArrayList COMMA ConstExp  */
#line 111 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1421 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 22: /* VarDecl: BType VarDef  */
#line 116 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_decl_stmt(ValueType((BasicType)(yyvsp[-1].value_type)));
                                                         (yyval.ast_node)->add_child(ASTNode::create_var_decl(ValueType((BasicType)(yyvsp[-1].value_type)),(yyvsp[0].ast_node)->get_var_name(), (yyvsp[0].ast_node)->get_value_type().is_array(), (yyvsp[0].ast_node)->get_child()));
                                                         delete (yyvsp[0].ast_node);
                                                        }
#line 1430 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 23: /* VarDecl: VarDecl COMMA VarDef  */
#line 120 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node);
                                                         (yyval.ast_node)->add_child(ASTNode::create_var_decl((yyvsp[-2].ast_node)->get_value_type(),(yyvsp[0].ast_node)->get_var_name(), (yyvsp[0].ast_node)->get_value_type().is_array(), (yyvsp[0].ast_node)->get_child()));
                                                         delete (yyvsp[0].ast_node);
                                                        }
#line 1439 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 24: /* VarDef: IDENT  */
#line 127 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_var_decl(ValueType(BasicType::Uncertain), *(yyvsp[0].string_const_val), false, {});}
#line 1445 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 25: /* VarDef: ConstArrayIdent  */
#line 128 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_var_decl(ValueType(BasicType::Uncertain),(yyvsp[0].ast_node)->get_var_name(), true, {(yyvsp[0].ast_node)});}
#line 1451 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 26: /* VarDef: IDENT ASSIGN Exp  */
#line 129 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_var_decl(ValueType(BasicType::Uncertain), *(yyvsp[-2].string_const_val), false, {(yyvsp[0].ast_node)});}
#line 1457 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 27: /* VarDef: ConstArrayIdent ASSIGN InitValArray  */
#line 130 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_var_decl(ValueType(BasicType::Uncertain),(yyvsp[-2].ast_node)->get_var_name(), true, {(yyvsp[-2].ast_node),(yyvsp[0].ast_node)});}
#line 1463 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 28: /* InitValArray: L_BRACE R_BRACE  */
#line 134 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain));}
#line 1469 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 29: /* InitValArray: L_BRACE InitValArrayList R_BRACE  */
#line 135 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1475 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 30: /* InitValArrayList: InitValArray  */
#line 139 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1482 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 31: /* InitValArrayList: InitValArrayList COMMA InitValArray  */
#line 141 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1489 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 32: /* InitValArrayList: Exp  */
#line 143 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1496 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 33: /* InitValArrayList: InitValArrayList COMMA Exp  */
#line 145 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node); 
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1503 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 34: /* FuncDef: BType IDENT L_PAREN R_PAREN Block  */
#line 150 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_def(ValueType((BasicType)(yyvsp[-4].value_type)), *(yyvsp[-3].string_const_val));
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1510 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 35: /* FuncDef: VOID IDENT L_PAREN R_PAREN Block  */
#line 152 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_def(ValueType(BasicType::Void), *(yyvsp[-3].string_const_val));
                                                         (yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1517 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 36: /* FuncDef: BType IDENT L_PAREN FuncFParams R_PAREN Block  */
#line 154 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_def(ValueType((BasicType)(yyvsp[-5].value_type)), *(yyvsp[-4].string_const_val));
                                                         (yyval.ast_node)->add_child({(yyvsp[-2].ast_node),(yyvsp[0].ast_node)});}
#line 1524 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 37: /* FuncDef: VOID IDENT L_PAREN FuncFParams R_PAREN Block  */
#line 156 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_def(ValueType(BasicType::Void), *(yyvsp[-4].string_const_val));
                                                         (yyval.ast_node)->add_child({(yyvsp[-2].ast_node),(yyvsp[0].ast_node)});}
#line 1531 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 38: /* FuncFParams: FuncFParam  */
#line 161 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_f_params();(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1537 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 39: /* FuncFParams: FuncFParams COMMA FuncFParam  */
#line 162 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node);(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1543 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 40: /* FuncFParam: BType IDENT  */
#line 166 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_f_param(ValueType((BasicType)(yyvsp[-1].value_type)), *(yyvsp[0].string_const_val), false, false, {});}
#line 1549 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 41: /* FuncFParam: FuncFParamArray  */
#line 167 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_f_param(ValueType((yyvsp[0].ast_node)->get_value_type().basic()), (yyvsp[0].ast_node)->get_var_name(), true, false, (yyvsp[0].ast_node)->get_child());
                                                         delete (yyvsp[0].ast_node);}
#line 1556 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 42: /* FuncFParamArray: BType IDENT L_BRACK R_BRACK  */
#line 172 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_f_param(ValueType((BasicType)(yyvsp[-3].value_type)), *(yyvsp[-2].string_const_val), true, false, {});}
#line 1562 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 43: /* FuncFParamArray: BType IDENT L_BRACK Exp R_BRACK  */
#line 173 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {yy_warning((char*)"FParam Array has value at first dim");(yyval.ast_node) = ASTNode::create_func_f_param(ValueType((BasicType)(yyvsp[-4].value_type)), *(yyvsp[-3].string_const_val), true, false, {});}
#line 1568 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 44: /* FuncFParamArray: FuncFParamArray L_BRACK Exp R_BRACK  */
#line 174 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-3].ast_node);
                                                         (yyval.ast_node)->add_child((yyvsp[-1].ast_node));}
#line 1575 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 45: /* Block: L_BRACE R_BRACE  */
#line 179 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_block();}
#line 1581 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 46: /* Block: L_BRACE BlockItemList R_BRACE  */
#line 180 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1587 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 47: /* BlockItemList: BlockItem  */
#line 184 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_block();(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1593 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 48: /* BlockItemList: BlockItemList BlockItem  */
#line 185 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1599 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 49: /* BlockItem: Decl  */
#line 189 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1605 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 50: /* BlockItem: Stmt  */
#line 190 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1611 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 51: /* Stmt: LVal ASSIGN Exp SEMI  */
#line 194 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Assign,(yyvsp[-3].ast_node)->get_value_type(),(yyvsp[-3].ast_node),(yyvsp[-1].ast_node));}
#line 1617 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 52: /* Stmt: SEMI  */
#line 195 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_null_stmt();}
#line 1623 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 53: /* Stmt: Exp SEMI  */
#line 196 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1629 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 54: /* Stmt: Block  */
#line 197 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1635 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 55: /* Stmt: IF L_PAREN Cond R_PAREN Stmt  */
#line 198 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_if_stmt((yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1641 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 56: /* Stmt: IF L_PAREN Cond R_PAREN Stmt ELSE Stmt  */
#line 199 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_if_stmt((yyvsp[-4].ast_node),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1647 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 57: /* Stmt: WHILE L_PAREN Cond R_PAREN Stmt  */
#line 200 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_while_stmt((yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1653 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 58: /* Stmt: BREAK SEMI  */
#line 201 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_break_stmt();}
#line 1659 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 59: /* Stmt: CONTINUE SEMI  */
#line 202 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_continue_stmt();}
#line 1665 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 60: /* Stmt: RETURN SEMI  */
#line 203 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_return_stmt();}
#line 1671 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 61: /* Stmt: RETURN Exp SEMI  */
#line 204 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_return_stmt((yyvsp[-1].ast_node));}
#line 1677 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 62: /* Exp: AddExp  */
#line 208 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1683 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 63: /* Cond: LOrExp  */
#line 212 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1689 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 64: /* LVal: IDENT  */
#line 216 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_ident(ValueType(BasicType::Uncertain), *(yyvsp[0].string_const_val), true);}
#line 1695 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 65: /* LVal: ArrayIdent  */
#line 217 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1701 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 66: /* ArrayIdent: IDENT L_BRACK Exp R_BRACK  */
#line 221 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_visit(ValueType(BasicType::Uncertain),ASTNode::create_ident(ValueType(BasicType::Uncertain), *(yyvsp[-3].string_const_val), true),(yyvsp[-1].ast_node));}
#line 1707 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 67: /* ArrayIdent: ArrayIdent L_BRACK Exp R_BRACK  */
#line 222 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_array_visit(ValueType(BasicType::Uncertain),(yyvsp[-3].ast_node),(yyvsp[-1].ast_node));}
#line 1713 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 68: /* PrimaryExp: L_PAREN Exp R_PAREN  */
#line 226 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-1].ast_node);}
#line 1719 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 69: /* PrimaryExp: LVal  */
#line 227 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1725 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 70: /* PrimaryExp: Number  */
#line 228 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1731 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 71: /* Number: INTCONST  */
#line 232 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_const_value(ValueType(BasicType::Int),BasicValue::create_int((yyvsp[0].int_const_val))); }
#line 1737 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 72: /* Number: FLOATCONST  */
#line 233 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_const_value(ValueType(BasicType::Float),BasicValue::create_float((yyvsp[0].float_const_val))); }
#line 1743 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 73: /* UnaryExp: PrimaryExp  */
#line 237 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1749 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 74: /* UnaryExp: IDENT L_PAREN R_PAREN  */
#line 238 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_call(*(yyvsp[-2].string_const_val));}
#line 1755 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 75: /* UnaryExp: IDENT L_PAREN FuncRParams R_PAREN  */
#line 239 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_call(*(yyvsp[-3].string_const_val));(yyval.ast_node)->add_child((yyvsp[-1].ast_node));}
#line 1761 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 76: /* UnaryExp: UnaryOp UnaryExp  */
#line 240 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_unary_op((UnaryOpFunc)(yyvsp[-1].unary_op),(yyvsp[0].ast_node)->get_value_type(),(yyvsp[0].ast_node), false);}
#line 1767 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 77: /* UnaryOp: ADD  */
#line 244 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.unary_op) = (int)UnaryOpFunc::Positive;}
#line 1773 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 78: /* UnaryOp: SUB  */
#line 245 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.unary_op) = (int)UnaryOpFunc::Negative;}
#line 1779 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 79: /* UnaryOp: NOT  */
#line 246 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.unary_op) = (int)UnaryOpFunc::Not;}
#line 1785 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 80: /* FuncRParams: Exp  */
#line 250 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_func_r_params();(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1791 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 81: /* FuncRParams: FuncRParams COMMA Exp  */
#line 251 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[-2].ast_node);(yyval.ast_node)->add_child((yyvsp[0].ast_node));}
#line 1797 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 82: /* MulExp: UnaryExp  */
#line 255 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1803 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 83: /* MulExp: MulExp MUL UnaryExp  */
#line 256 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Mul,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1809 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 84: /* MulExp: MulExp DIV UnaryExp  */
#line 257 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Div,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1815 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 85: /* MulExp: MulExp MOD UnaryExp  */
#line 258 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Mod,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1821 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 86: /* AddExp: MulExp  */
#line 262 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1827 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 87: /* AddExp: AddExp ADD MulExp  */
#line 263 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Add,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1833 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 88: /* AddExp: AddExp SUB MulExp  */
#line 264 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Sub,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1839 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 89: /* RelExp: AddExp  */
#line 268 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1845 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 90: /* RelExp: RelExp LT AddExp  */
#line 269 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Less,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1851 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 91: /* RelExp: RelExp GT AddExp  */
#line 270 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Great,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1857 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 92: /* RelExp: RelExp LE AddExp  */
#line 271 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::LessEqual,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1863 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 93: /* RelExp: RelExp GE AddExp  */
#line 272 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::GreatEqual,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1869 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 94: /* EqExp: RelExp  */
#line 276 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1875 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 95: /* EqExp: EqExp EQ RelExp  */
#line 277 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Equal,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1881 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 96: /* EqExp: EqExp NEQ RelExp  */
#line 278 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::NotEqual,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1887 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 97: /* LAndExp: EqExp  */
#line 282 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1893 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 98: /* LAndExp: LAndExp AND EqExp  */
#line 283 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::And,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1899 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 99: /* LOrExp: LAndExp  */
#line 287 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1905 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 100: /* LOrExp: LOrExp OR LAndExp  */
#line 288 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = ASTNode::create_binary_op(BinaryOpFunc::Or,(yyvsp[-2].ast_node)->get_value_type(),(yyvsp[-2].ast_node),(yyvsp[0].ast_node));}
#line 1911 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;

  case 101: /* ConstExp: AddExp  */
#line 292 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"
                                                        {(yyval.ast_node) = (yyvsp[0].ast_node);}
#line 1917 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"
    break;


#line 1921 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.cpp"

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
  ++yynerrs;

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
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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

#line 294 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"

void yyerror(char * msg)
{
    std::cerr<<"[Syntax Error] line "<<line_no<<" : "<<msg << " ." << std::endl;
    exit(ErrorCode::SYNTAX_ERROR);
}
void yy_warning(char *msg)
{
    std::cerr<<"[Syntax Warning] line "<<line_no<<" : "<<msg << " ." << std::endl;
}
