/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_D_SEMESTER6_COMPILER2022_CRYD_GEN_SRC_PARSER_YY_HPP_INCLUDED
# define YY_YY_D_SEMESTER6_COMPILER2022_CRYD_GEN_SRC_PARSER_YY_HPP_INCLUDED
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
    INTCONST = 258,                /* INTCONST  */
    FLOATCONST = 259,              /* FLOATCONST  */
    IDENT = 260,                   /* IDENT  */
    IF = 261,                      /* IF  */
    ELSE = 262,                    /* ELSE  */
    WHILE = 263,                   /* WHILE  */
    BREAK = 264,                   /* BREAK  */
    CONTINUE = 265,                /* CONTINUE  */
    RETURN = 266,                  /* RETURN  */
    INT = 267,                     /* INT  */
    FLOAT = 268,                   /* FLOAT  */
    VOID = 269,                    /* VOID  */
    CONST = 270,                   /* CONST  */
    L_PAREN = 271,                 /* L_PAREN  */
    R_PAREN = 272,                 /* R_PAREN  */
    L_BRACK = 273,                 /* L_BRACK  */
    R_BRACK = 274,                 /* R_BRACK  */
    L_BRACE = 275,                 /* L_BRACE  */
    R_BRACE = 276,                 /* R_BRACE  */
    SEMI = 277,                    /* SEMI  */
    COMMA = 278,                   /* COMMA  */
    ASSIGN = 279,                  /* ASSIGN  */
    ADD = 280,                     /* ADD  */
    SUB = 281,                     /* SUB  */
    MUL = 282,                     /* MUL  */
    DIV = 283,                     /* DIV  */
    MOD = 284,                     /* MOD  */
    AND = 285,                     /* AND  */
    OR = 286,                      /* OR  */
    NOT = 287,                     /* NOT  */
    GT = 288,                      /* GT  */
    LT = 289,                      /* LT  */
    LE = 290,                      /* LE  */
    GE = 291,                      /* GE  */
    EQ = 292,                      /* EQ  */
    NEQ = 293                      /* NEQ  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 14 "D:/semester6/compiler2022-cryd/src/front_end/parser.y"

    class ASTNode* ast_node;
    int int_const_val;
    float float_const_val;
    std::string* string_const_val;
    int value_type;
    int unary_op;

#line 111 "D:/semester6/compiler2022-cryd/gen_src/parser.yy.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_D_SEMESTER6_COMPILER2022_CRYD_GEN_SRC_PARSER_YY_HPP_INCLUDED  */
