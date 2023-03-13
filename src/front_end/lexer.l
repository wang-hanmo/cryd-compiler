%{
    #include<iostream>
    #include<parser.yy.hpp>
    #include<type_define.h>
    int line_no=1;
%}
%option noyywrap
%option never-interactive
%x COMMENT
%x SINGLELINECOMMENT
/************************************
*	MACRO DEFINITION	*
*************************************/
intconst_10         (0|[1-9][0-9]*)
intconst_8          0[0-7]*
intconst_16         (0x|0X)[0-9a-fA-F]+
floatconst_10          ([0-9]*\.?[0-9]+|[0-9]+\.)((e|E)[-+]?[0-9]+)?
floatconst_16       (0x|0X)([0-9a-fA-F]*\.?[0-9a-fA-F]+|[0-9a-fA-F]+\.)((p|P)[-+]?[0-9a-fA-F]+)?
ident               [_a-zA-Z][_a-zA-Z0-9]*
whitespace		    [\040\t\r]
newline             [\n]
/*****************************************
*	RECOGNIION RULES	         *
*****************************************/
%%

"/*"		                {BEGIN(COMMENT);}
<COMMENT>"*/"               {BEGIN(INITIAL);}
<COMMENT>\n     {line_no++;}
<COMMENT>([^*])+|.
<COMMENT><<EOF>>            {BEGIN(INITIAL);}
"//"                        {BEGIN(SINGLELINECOMMENT);}
<SINGLELINECOMMENT>.*
<SINGLELINECOMMENT>\n       {line_no++;BEGIN(INITIAL);}
<SINGLELINECOMMENT><<EOF>>  {BEGIN(INITIAL);}

"if"	            { return IF;}
"else"	            { return ELSE;}
"while"	            { return WHILE;}
"break"	            { return BREAK;}
"continue"      	{ return CONTINUE;}
"return"	        { return RETURN;}
"int"	            { return INT;}
"float"	            { return FLOAT;}
"void"	            { return VOID;}
"const"	            { return CONST;}

"("			        { return L_PAREN;}
")"			        { return R_PAREN;}
"["                 { return L_BRACK;}
"]"                 { return R_BRACK;}
"{"                 { return L_BRACE;}
"}"                 { return R_BRACE;}
";"                 { return SEMI;}
","                 { return COMMA;}
"="                 { return ASSIGN;}

"+"			        { return ADD;}
"-"			        { return SUB;}
"*"			        { return MUL;}
"/"			        { return DIV;}
"%"			        { return MOD;}

"&&"                { return AND;}
"||"                { return OR;}
"!"                 { return NOT;}

">"                 { return GT;}
"<"                 { return LT;}
"<="                { return LE;}
">="                { return GE;}
"=="                { return EQ;}
"!="                { return NEQ;}


{intconst_10}	    {
                        yylval.int_const_val=(int)strtol(yytext, (char **)NULL, 10);
                        return INTCONST;
                    }
{intconst_8}	    {
                        yylval.int_const_val = (int)strtol(yytext, (char **)NULL, 8);
                        return INTCONST;
                    }
{intconst_16}	    {
                        yylval.int_const_val = (int)strtol(yytext, (char **)NULL, 16);
                        return INTCONST;
                    }
{floatconst_10}	    {
                        yylval.float_const_val = (float)strtof(yytext, (char **)NULL);
                        return FLOATCONST;
                    }
{floatconst_16}	    {
                        yylval.float_const_val = (float)strtof(yytext, (char **)NULL);
                        return FLOATCONST;
                    }
{ident}             {
                        yylval.string_const_val = new std::string(yytext, yyleng);
                        return IDENT;
                    }

{whitespace}+	    {}
{newline}           {line_no++;}
.			        {
                        std::cout<<"[Lexical Error] : Invalid character \'"<<yytext << "\' ."<<std::endl;
                        exit(ErrorCode::LEXICAL_ERROR);
                    }

%%