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
#line 7 "parse.y"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "common.h"

#line 79 "y.tab.c"

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
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
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
    FIRST = 258,                   /* FIRST  */
    SECOND = 259,                  /* SECOND  */
    THIRD = 260,                   /* THIRD  */
    FORTH = 261,                   /* FORTH  */
    FIFTH = 262,                   /* FIFTH  */
    SIXTH = 263,                   /* SIXTH  */
    SEVENTH = 264,                 /* SEVENTH  */
    EIGHTH = 265,                  /* EIGHTH  */
    NINETH = 266,                  /* NINETH  */
    TENTH = 267,                   /* TENTH  */
    LAST = 268,                    /* LAST  */
    MON = 269,                     /* MON  */
    TUE = 270,                     /* TUE  */
    WED = 271,                     /* WED  */
    THU = 272,                     /* THU  */
    FRI = 273,                     /* FRI  */
    SAT = 274,                     /* SAT  */
    SUN = 275,                     /* SUN  */
    JAN = 276,                     /* JAN  */
    FEB = 277,                     /* FEB  */
    MAR = 278,                     /* MAR  */
    APR = 279,                     /* APR  */
    MAY = 280,                     /* MAY  */
    JUN = 281,                     /* JUN  */
    JUL = 282,                     /* JUL  */
    AUG = 283,                     /* AUG  */
    SEP = 284,                     /* SEP  */
    OCT = 285,                     /* OCT  */
    NOV = 286,                     /* NOV  */
    DEC = 287,                     /* DEC  */
    DAY = 288,                     /* DAY  */
    WEEK = 289,                    /* WEEK  */
    MONTH = 290,                   /* MONTH  */
    YEAR = 291,                    /* YEAR  */
    OF = 292,                      /* OF  */
    SPECIAL = 293,                 /* SPECIAL  */
    CASCADE = 294,                 /* CASCADE  */
    IDENT = 295,                   /* IDENT  */
    INTEGER = 296,                 /* INTEGER  */
    STRING = 297,                  /* STRING  */
    PLUS = 298,                    /* PLUS  */
    MINUS = 299,                   /* MINUS  */
    LPAREN = 300,                  /* LPAREN  */
    RPAREN = 301,                  /* RPAREN  */
    NOT = 302,                     /* NOT  */
    AND = 303,                     /* AND  */
    OR = 304,                      /* OR  */
    EQ = 305,                      /* EQ  */
    NE = 306,                      /* NE  */
    TILDE = 307,                   /* TILDE  */
    EMPTY = 308,                   /* EMPTY  */
    FUNC = 309,                    /* FUNC  */
    LIST = 310,                    /* LIST  */
    TICK = 311,                    /* TICK  */
    COLON = 312                    /* COLON  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define FIRST 258
#define SECOND 259
#define THIRD 260
#define FORTH 261
#define FIFTH 262
#define SIXTH 263
#define SEVENTH 264
#define EIGHTH 265
#define NINETH 266
#define TENTH 267
#define LAST 268
#define MON 269
#define TUE 270
#define WED 271
#define THU 272
#define FRI 273
#define SAT 274
#define SUN 275
#define JAN 276
#define FEB 277
#define MAR 278
#define APR 279
#define MAY 280
#define JUN 281
#define JUL 282
#define AUG 283
#define SEP 284
#define OCT 285
#define NOV 286
#define DEC 287
#define DAY 288
#define WEEK 289
#define MONTH 290
#define YEAR 291
#define OF 292
#define SPECIAL 293
#define CASCADE 294
#define IDENT 295
#define INTEGER 296
#define STRING 297
#define PLUS 298
#define MINUS 299
#define LPAREN 300
#define RPAREN 301
#define NOT 302
#define AND 303
#define OR 304
#define EQ 305
#define NE 306
#define TILDE 307
#define EMPTY 308
#define FUNC 309
#define LIST 310
#define TICK 311
#define COLON 312

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 "parse.y"

  tree* expr;
  action act;
  int mday;
  int wday;
  int month;
  int ordinal;
  time_t date;
  char* text;

#line 257 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (tree **restree);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_FIRST = 3,                      /* FIRST  */
  YYSYMBOL_SECOND = 4,                     /* SECOND  */
  YYSYMBOL_THIRD = 5,                      /* THIRD  */
  YYSYMBOL_FORTH = 6,                      /* FORTH  */
  YYSYMBOL_FIFTH = 7,                      /* FIFTH  */
  YYSYMBOL_SIXTH = 8,                      /* SIXTH  */
  YYSYMBOL_SEVENTH = 9,                    /* SEVENTH  */
  YYSYMBOL_EIGHTH = 10,                    /* EIGHTH  */
  YYSYMBOL_NINETH = 11,                    /* NINETH  */
  YYSYMBOL_TENTH = 12,                     /* TENTH  */
  YYSYMBOL_LAST = 13,                      /* LAST  */
  YYSYMBOL_MON = 14,                       /* MON  */
  YYSYMBOL_TUE = 15,                       /* TUE  */
  YYSYMBOL_WED = 16,                       /* WED  */
  YYSYMBOL_THU = 17,                       /* THU  */
  YYSYMBOL_FRI = 18,                       /* FRI  */
  YYSYMBOL_SAT = 19,                       /* SAT  */
  YYSYMBOL_SUN = 20,                       /* SUN  */
  YYSYMBOL_JAN = 21,                       /* JAN  */
  YYSYMBOL_FEB = 22,                       /* FEB  */
  YYSYMBOL_MAR = 23,                       /* MAR  */
  YYSYMBOL_APR = 24,                       /* APR  */
  YYSYMBOL_MAY = 25,                       /* MAY  */
  YYSYMBOL_JUN = 26,                       /* JUN  */
  YYSYMBOL_JUL = 27,                       /* JUL  */
  YYSYMBOL_AUG = 28,                       /* AUG  */
  YYSYMBOL_SEP = 29,                       /* SEP  */
  YYSYMBOL_OCT = 30,                       /* OCT  */
  YYSYMBOL_NOV = 31,                       /* NOV  */
  YYSYMBOL_DEC = 32,                       /* DEC  */
  YYSYMBOL_DAY = 33,                       /* DAY  */
  YYSYMBOL_WEEK = 34,                      /* WEEK  */
  YYSYMBOL_MONTH = 35,                     /* MONTH  */
  YYSYMBOL_YEAR = 36,                      /* YEAR  */
  YYSYMBOL_OF = 37,                        /* OF  */
  YYSYMBOL_SPECIAL = 38,                   /* SPECIAL  */
  YYSYMBOL_CASCADE = 39,                   /* CASCADE  */
  YYSYMBOL_IDENT = 40,                     /* IDENT  */
  YYSYMBOL_INTEGER = 41,                   /* INTEGER  */
  YYSYMBOL_STRING = 42,                    /* STRING  */
  YYSYMBOL_PLUS = 43,                      /* PLUS  */
  YYSYMBOL_MINUS = 44,                     /* MINUS  */
  YYSYMBOL_LPAREN = 45,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 46,                    /* RPAREN  */
  YYSYMBOL_NOT = 47,                       /* NOT  */
  YYSYMBOL_AND = 48,                       /* AND  */
  YYSYMBOL_OR = 49,                        /* OR  */
  YYSYMBOL_EQ = 50,                        /* EQ  */
  YYSYMBOL_NE = 51,                        /* NE  */
  YYSYMBOL_TILDE = 52,                     /* TILDE  */
  YYSYMBOL_EMPTY = 53,                     /* EMPTY  */
  YYSYMBOL_FUNC = 54,                      /* FUNC  */
  YYSYMBOL_LIST = 55,                      /* LIST  */
  YYSYMBOL_TICK = 56,                      /* TICK  */
  YYSYMBOL_COLON = 57,                     /* COLON  */
  YYSYMBOL_YYACCEPT = 58,                  /* $accept  */
  YYSYMBOL_parse = 59,                     /* parse  */
  YYSYMBOL_lines = 60,                     /* lines  */
  YYSYMBOL_line = 61,                      /* line  */
  YYSYMBOL_expr = 62,                      /* expr  */
  YYSYMBOL_date = 63,                      /* date  */
  YYSYMBOL_ordinal = 64,                   /* ordinal  */
  YYSYMBOL_month = 65,                     /* month  */
  YYSYMBOL_wday = 66,                      /* wday  */
  YYSYMBOL_mday = 67                       /* mday  */
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
typedef yytype_int8 yy_state_t;

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
#define YYFINAL  31
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   106

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  10
/* YYNRULES -- Number of rules.  */
#define YYNRULES  51
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  68

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   312


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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int8 yyrline[] =
{
       0,    36,    36,    39,    40,    43,    44,    51,    52,    53,
      54,    55,    56,    57,    58,    61,    62,    63,    64,    65,
      66,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    96,    97,    98,    99,   100,   101,
     102,   105
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
  "\"end of file\"", "error", "\"invalid token\"", "FIRST", "SECOND",
  "THIRD", "FORTH", "FIFTH", "SIXTH", "SEVENTH", "EIGHTH", "NINETH",
  "TENTH", "LAST", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN", "JAN",
  "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV",
  "DEC", "DAY", "WEEK", "MONTH", "YEAR", "OF", "SPECIAL", "CASCADE",
  "IDENT", "INTEGER", "STRING", "PLUS", "MINUS", "LPAREN", "RPAREN", "NOT",
  "AND", "OR", "EQ", "NE", "TILDE", "EMPTY", "FUNC", "LIST", "TICK",
  "COLON", "$accept", "parse", "lines", "line", "expr", "date", "ordinal",
  "month", "wday", "mday", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-34)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      40,   -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,
     -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,   -34,
     -34,   -34,   -34,   -34,   -34,     0,   -34,   -34,    42,    20,
     -25,   -34,   -34,   -34,   -25,   -25,    33,   -34,   -34,   -34,
     -34,   -34,   -34,   -34,   -20,   -34,   -34,   -34,   -34,    -4,
     -34,   -34,    33,    33,    40,     5,    74,    33,   -32,     5,
     -24,    33,   -34,     8,   -34,   -34,     5,   -34
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    17,     0,     2,     3,     5,     0,
       0,     1,     4,    20,     0,     0,    13,    45,    46,    47,
      48,    49,    50,    44,     0,    51,    16,    18,    19,     0,
       9,     8,    13,    13,     0,     6,     0,    13,     0,    10,
       0,    13,    15,     0,    12,    14,    11,     7
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -34,   -34,   -34,    32,    31,     2,   -34,     3,   -34,   -33
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    25,    26,    27,    55,    28,    29,    30,    44,    46
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      31,    47,    48,     1,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    64,    33,    45,    56,    61,    34,
      35,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    65,    37,    38,    39,    40,    41,    42,
      43,    57,    24,     1,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    67,    61,    60,    32,    61,    62,
       0,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    49,    50,    51,     0,     0,    52,     0,
      53,    33,    24,    58,    59,    34,    35,     0,    63,     0,
      54,     0,    66,     0,    36,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23
};

static const yytype_int8 yycheck[] =
{
       0,    34,    35,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    46,    39,    41,    37,    50,    43,
      44,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    57,    14,    15,    16,    17,    18,    19,
      20,    45,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    46,    50,    54,    25,    50,    56,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    40,    41,    42,    -1,    -1,    45,    -1,
      47,    39,    42,    52,    53,    43,    44,    -1,    57,    -1,
      57,    -1,    61,    -1,    52,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    42,    59,    60,    61,    63,    64,
      65,     0,    61,    39,    43,    44,    52,    14,    15,    16,
      17,    18,    19,    20,    66,    41,    67,    67,    67,    40,
      41,    42,    45,    47,    57,    62,    37,    45,    62,    62,
      63,    50,    65,    62,    46,    57,    62,    46
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    58,    59,    60,    60,    61,    61,    62,    62,    62,
      62,    62,    62,    62,    62,    63,    63,    63,    63,    63,
      63,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    66,    66,    66,    66,    66,    66,
      66,    67
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     3,     4,     1,     1,
       2,     3,     3,     0,     3,     4,     2,     1,     3,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
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
        yyerror (restree, YY_("syntax error: cannot back up")); \
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
                  Kind, Value, restree); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, tree **restree)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (restree);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, tree **restree)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, restree);
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
                 int yyrule, tree **restree)
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
                       &yyvsp[(yyi + 1) - (yynrhs)], restree);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, restree); \
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, tree **restree)
{
  YY_USE (yyvaluep);
  YY_USE (restree);
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
yyparse (tree **restree)
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
  case 2: /* parse: lines  */
#line 36 "parse.y"
                { tree** t = restree; *t = (yyvsp[0].expr); }
#line 1357 "y.tab.c"
    break;

  case 3: /* lines: line  */
#line 39 "parse.y"
               { (yyval.expr) = NewOp(LIST, (yyvsp[0].expr), NULL); printf("New List created %p\n", (yyval.expr)); }
#line 1363 "y.tab.c"
    break;

  case 4: /* lines: parse line  */
#line 40 "parse.y"
                     { (yyval.expr) = NewOp(LIST, (yyvsp[-1].expr), (yyvsp[0].expr)); printf("List appened %p\n", (yyval.expr)); }
#line 1369 "y.tab.c"
    break;

  case 5: /* line: date  */
#line 43 "parse.y"
               { (yyval.expr) = NewOp(FUNC, "print", NewOp(FUNC, "omni", NewInt(TICK, (yyvsp[0].date)))); printf("New Tick %p\n", (yyval.expr)); }
#line 1375 "y.tab.c"
    break;

  case 6: /* line: date TILDE expr  */
#line 45 "parse.y"
        {
	  tree *t = NewInt(TICK, (yyvsp[-2].date));
	  (yyval.expr) = NewOp(TILDE,  t, (yyvsp[0].expr)); printf("TILDE %p %p %p\n", (yyval.expr), t, (yyvsp[0].expr));
	}
#line 1384 "y.tab.c"
    break;

  case 7: /* expr: IDENT LPAREN expr RPAREN  */
#line 51 "parse.y"
                                { (yyval.expr) = NewOp(FUNC, NewStr(IDENT, (yyvsp[-3].text)), (yyvsp[-1].expr)); }
#line 1390 "y.tab.c"
    break;

  case 8: /* expr: STRING  */
#line 52 "parse.y"
                 { (yyval.expr) = NewStr(STRING, (yyvsp[0].text)); }
#line 1396 "y.tab.c"
    break;

  case 9: /* expr: INTEGER  */
#line 53 "parse.y"
                  { (yyval.expr) = NewInt(INTEGER, strtol((yyvsp[0].text), NULL, 10)); }
#line 1402 "y.tab.c"
    break;

  case 10: /* expr: NOT expr  */
#line 54 "parse.y"
                   { (yyval.expr) = NewOp(NOT, (yyvsp[0].expr), NULL); }
#line 1408 "y.tab.c"
    break;

  case 11: /* expr: expr EQ expr  */
#line 55 "parse.y"
                       { (yyval.expr) = NewOp(EQ, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 1414 "y.tab.c"
    break;

  case 12: /* expr: LPAREN expr RPAREN  */
#line 56 "parse.y"
                             { (yyval.expr) = (yyvsp[-1].expr); }
#line 1420 "y.tab.c"
    break;

  case 13: /* expr: %empty  */
#line 57 "parse.y"
          { (yyval.expr) = NewInt(EMPTY, 0); }
#line 1426 "y.tab.c"
    break;

  case 14: /* expr: COLON date COLON  */
#line 58 "parse.y"
                           { (yyval.expr) = NewInt(TICK, (yyvsp[-1].date)); printf("TICK %d\n", (yyvsp[-1].date));}
#line 1432 "y.tab.c"
    break;

  case 15: /* date: ordinal wday OF month  */
#line 61 "parse.y"
                                { (yyval.date) = nthdayofweek(currentyear, (yyvsp[0].month), (yyvsp[-2].wday), (yyvsp[-3].ordinal)); }
#line 1438 "y.tab.c"
    break;

  case 16: /* date: month mday  */
#line 62 "parse.y"
                     { (yyval.date) = nthday(currentyear, (yyvsp[-1].month), (yyvsp[0].mday)); }
#line 1444 "y.tab.c"
    break;

  case 17: /* date: STRING  */
#line 63 "parse.y"
                 { (yyval.date) = special(currentyear, (yyvsp[0].text)); }
#line 1450 "y.tab.c"
    break;

  case 18: /* date: date PLUS mday  */
#line 64 "parse.y"
                         { (yyval.date) = addday((yyvsp[-2].date), (yyvsp[0].mday)); }
#line 1456 "y.tab.c"
    break;

  case 19: /* date: date MINUS mday  */
#line 65 "parse.y"
                          { (yyval.date) = addday((yyvsp[-2].date), -(yyvsp[0].mday)); }
#line 1462 "y.tab.c"
    break;

  case 20: /* date: date CASCADE  */
#line 66 "parse.y"
                       { (yyval.date) = cascade((yyvsp[-1].date)); }
#line 1468 "y.tab.c"
    break;

  case 21: /* ordinal: FIRST  */
#line 69 "parse.y"
                { (yyval.ordinal) = 0; }
#line 1474 "y.tab.c"
    break;

  case 22: /* ordinal: SECOND  */
#line 70 "parse.y"
                 { (yyval.ordinal) = 1; }
#line 1480 "y.tab.c"
    break;

  case 23: /* ordinal: THIRD  */
#line 71 "parse.y"
                { (yyval.ordinal) = 2; }
#line 1486 "y.tab.c"
    break;

  case 24: /* ordinal: FORTH  */
#line 72 "parse.y"
                { (yyval.ordinal) = 3; }
#line 1492 "y.tab.c"
    break;

  case 25: /* ordinal: FIFTH  */
#line 73 "parse.y"
                { (yyval.ordinal) = 4; }
#line 1498 "y.tab.c"
    break;

  case 26: /* ordinal: SIXTH  */
#line 74 "parse.y"
                { (yyval.ordinal) = 5; }
#line 1504 "y.tab.c"
    break;

  case 27: /* ordinal: SEVENTH  */
#line 75 "parse.y"
                  { (yyval.ordinal) = 6; }
#line 1510 "y.tab.c"
    break;

  case 28: /* ordinal: EIGHTH  */
#line 76 "parse.y"
                 { (yyval.ordinal) = 7; }
#line 1516 "y.tab.c"
    break;

  case 29: /* ordinal: NINETH  */
#line 77 "parse.y"
                 { (yyval.ordinal) = 8; }
#line 1522 "y.tab.c"
    break;

  case 30: /* ordinal: TENTH  */
#line 78 "parse.y"
                { (yyval.ordinal) = 9; }
#line 1528 "y.tab.c"
    break;

  case 31: /* ordinal: LAST  */
#line 79 "parse.y"
               { (yyval.ordinal) = -1; }
#line 1534 "y.tab.c"
    break;

  case 32: /* month: JAN  */
#line 82 "parse.y"
              { (yyval.month) = 0; }
#line 1540 "y.tab.c"
    break;

  case 33: /* month: FEB  */
#line 83 "parse.y"
              { (yyval.month) = 1; }
#line 1546 "y.tab.c"
    break;

  case 34: /* month: MAR  */
#line 84 "parse.y"
              { (yyval.month) = 2; }
#line 1552 "y.tab.c"
    break;

  case 35: /* month: APR  */
#line 85 "parse.y"
              { (yyval.month) = 3; }
#line 1558 "y.tab.c"
    break;

  case 36: /* month: MAY  */
#line 86 "parse.y"
              { (yyval.month) = 4; }
#line 1564 "y.tab.c"
    break;

  case 37: /* month: JUN  */
#line 87 "parse.y"
              { (yyval.month) = 5; }
#line 1570 "y.tab.c"
    break;

  case 38: /* month: JUL  */
#line 88 "parse.y"
              { (yyval.month) = 6; }
#line 1576 "y.tab.c"
    break;

  case 39: /* month: AUG  */
#line 89 "parse.y"
              { (yyval.month) = 7; }
#line 1582 "y.tab.c"
    break;

  case 40: /* month: SEP  */
#line 90 "parse.y"
              { (yyval.month) = 8; }
#line 1588 "y.tab.c"
    break;

  case 41: /* month: OCT  */
#line 91 "parse.y"
              { (yyval.month) = 9; }
#line 1594 "y.tab.c"
    break;

  case 42: /* month: NOV  */
#line 92 "parse.y"
              { (yyval.month) = 10; }
#line 1600 "y.tab.c"
    break;

  case 43: /* month: DEC  */
#line 93 "parse.y"
              { (yyval.month) = 11; }
#line 1606 "y.tab.c"
    break;

  case 44: /* wday: SUN  */
#line 96 "parse.y"
              { (yyval.wday) = 0; }
#line 1612 "y.tab.c"
    break;

  case 45: /* wday: MON  */
#line 97 "parse.y"
              { (yyval.wday) = 1; }
#line 1618 "y.tab.c"
    break;

  case 46: /* wday: TUE  */
#line 98 "parse.y"
              { (yyval.wday) = 2; }
#line 1624 "y.tab.c"
    break;

  case 47: /* wday: WED  */
#line 99 "parse.y"
              { (yyval.wday) = 3; }
#line 1630 "y.tab.c"
    break;

  case 48: /* wday: THU  */
#line 100 "parse.y"
              { (yyval.wday) = 4; }
#line 1636 "y.tab.c"
    break;

  case 49: /* wday: FRI  */
#line 101 "parse.y"
              { (yyval.wday) = 5; }
#line 1642 "y.tab.c"
    break;

  case 50: /* wday: SAT  */
#line 102 "parse.y"
              { (yyval.wday) = 6; }
#line 1648 "y.tab.c"
    break;

  case 51: /* mday: INTEGER  */
#line 105 "parse.y"
                  { (yyval.mday) = strtol((yyvsp[0].text), NULL, 10); }
#line 1654 "y.tab.c"
    break;


#line 1658 "y.tab.c"

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
      yyerror (restree, YY_("syntax error"));
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
                      yytoken, &yylval, restree);
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, restree);
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
  yyerror (restree, YY_("memory exhausted"));
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
                  yytoken, &yylval, restree);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, restree);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 107 "parse.y"

int yyerror(char *str)
{
  fprintf(stderr, "parse error: %s\n", str);
  exit(1);
}
