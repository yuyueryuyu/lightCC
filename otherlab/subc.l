%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"  /* 由YACC/Bison生成的头文件 */

void yyerror(const char *s);
int line_num = 1;
%}

%option yylineno
%option noyywrap

/* 定义正则表达式 */
digit       [0-9]
letter      [a-zA-Z]
whitespace  [ \t\r\f]

%%

"int"           { return INT; }
"float"         { return FLOAT; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"return"        { return RETURN; }
"input"         { return INPUT; }
"print"         { return PRINT; }
"void"          { return VOID; }

"+"             { return ADD; }
"-"             { return SUB; }
"*"             { return MUL; }
"/"             { return DIV; }
"%"             { return MOD; }

"=="            { return EQ; }
"="             { return ASG; }

"<"             { return LT; }
"<="            { return LE; }

">"             { return GT; }
">="            { return GE; }

"!="            { return NE; }

"&&"            { return ANDAND; }
"||"            { return OROR; }

"("             { return LPA; }
")"             { return RPA; }
"["             { return LBK; }
"]"             { return RBK; }
"{"             { return LBR; }
"}"             { return RBR; }
","             { return CMA; }
";"             { return SCO; }

[+-]?{digit}+   { 
                  yylval.ival = atoi(yytext);
                  return NUM; 
                }

[+-]?{digit}*\.{digit}+|{digit}+\.{digit}* { 
                  yylval.fval = atof(yytext);
                  return FLO; 
                }

{letter}({letter}|{digit})* { 
                  yylval.sval = strdup(yytext);
                  return ID; 
                }

\n              { line_num++; }
{whitespace}+   { /* 忽略空白字符 */ }

.               { yyerror("Unrecognized character"); }

%%
