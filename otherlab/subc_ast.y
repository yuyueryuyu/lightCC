%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void yyerror(const char *s);
int yylex();
extern int yydebug;
extern int yylineno;
extern char *yytext;
extern FILE *yyin; 

/* 全局AST根节点 */
ASTNode* ast_root = NULL;
%}

/* YACC 定义 */
%union {
    int ival;
    float fval;
    char *sval;
    struct ast_node *ast;
}

/* 定义终结符的token */
%token <ival> NUM
%token <fval> FLO
%token <sval> ID
%token INT FLOAT VOID
%token IF ELSE WHILE RETURN
%token INPUT PRINT

/* 定义运算符和其他终结符 */
%token ADD SUB MUL DIV MOD
%token EQ ASG
%token LT LE GT GE NE
%token ANDAND OROR
%token LPA RPA LBK RBK LBR RBR
%token CMA SCO

/* 定义优先级和结合性 */
%left OROR
%left ANDAND
%left EQ NE
%left LT LE GT GE
%left ADD SUB
%left MUL DIV MOD
%right ASG

/* 定义非终结符类型 */
%type <ast> program declaration_list declaration 
%type <ast> type_specifier param_list param
%type <ast> statement_list statement
%type <ast> expression 
%type <ast> arg arg_list condition

%start program

%%

/* 语法规则 */

program
    : declaration_list statement_list
      {
          $$ = create_ast_node(PROGRAM_NODE);
          add_ast_child($$, $1);
          add_ast_child($$, $2);
          ast_root = $$;
      }
    ;

declaration_list
    : declaration_list declaration SCO
      {
          $$ = $1;
          add_ast_child($$, $2);
      }
    | /* empty */
      {
          $$ = create_ast_node(DECL_LIST_NODE);
      }
    ;

declaration
    : type_specifier ID
      {
          $$ = create_ast_node(VAR_DECL_NODE);
          add_ast_child($$, $1);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $2);
          add_ast_child($$, id_node);
      }
    | type_specifier ID LBK NUM RBK
      {
          $$ = create_ast_node(ARRAY_DECL_NODE);
          add_ast_child($$, $1);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $2);
          add_ast_child($$, id_node);
          ASTNode* size_node = create_ast_node(NUM_EXPR_NODE);
          set_int_value(size_node, $4);
          add_ast_child($$, size_node);
      }
    | type_specifier ID LPA param_list RPA LBR declaration_list statement_list RBR
      {
          $$ = create_ast_node(FUNC_DECL_NODE);
          add_ast_child($$, $1);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $2);
          add_ast_child($$, id_node);
          add_ast_child($$, $4);
          add_ast_child($$, $7);
          add_ast_child($$, $8);
      }
    ;

type_specifier
    : INT
      {
          $$ = create_ast_node(TYPE_SPEC_NODE);
          set_type_value($$, INT);
      }
    | FLOAT
      {
          $$ = create_ast_node(TYPE_SPEC_NODE);
          set_type_value($$, FLOAT);
      }
    | VOID
      {
          $$ = create_ast_node(TYPE_SPEC_NODE);
          set_type_value($$, VOID);
      }
    ;

param_list
    : param_list param SCO
      {
          $$ = $1;
          add_ast_child($$, $2);
      }
    | /* empty */
      {
          $$ = create_ast_node(PARAM_LIST_NODE);
      }
    ;

param
    : type_specifier ID
      {
          $$ = create_ast_node(PARAM_NODE);
          add_ast_child($$, $1);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $2);
          add_ast_child($$, id_node);
      }
    | type_specifier ID LBK RBK
      {
          $$ = create_ast_node(ARRAY_PARAM_NODE);
          add_ast_child($$, $1);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $2);
          add_ast_child($$, id_node);
      }
    | type_specifier ID LPA type_specifier RPA
      {
          $$ = create_ast_node(FUNC_PARAM_NODE);
          add_ast_child($$, $1);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $2);
          add_ast_child($$, id_node);
          add_ast_child($$, $4);
      }
    ;

statement_list
    : statement_list SCO statement
      {
          $$ = $1;
          add_ast_child($$, $3);
      }
    | statement
      {
          $$ = create_ast_node(STMT_LIST_NODE);
          add_ast_child($$, $1);
      }
    ;

statement
    : ID ASG expression
      {
          $$ = create_ast_node(ASSIGN_STMT_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
          add_ast_child($$, $3);
      }
    | ID LBR expression RBR ASG expression
      {
          $$ = create_ast_node(ARRAY_ASSIGN_STMT_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
          add_ast_child($$, $3);
          add_ast_child($$, $6);
      }
    | IF LPA condition RPA statement
      {
          $$ = create_ast_node(IF_STMT_NODE);
          add_ast_child($$, $3);
          add_ast_child($$, $5);
      }
    | IF LPA condition RPA statement ELSE statement
      {
          $$ = create_ast_node(IF_ELSE_STMT_NODE);
          add_ast_child($$, $3);
          add_ast_child($$, $5);
          add_ast_child($$, $7);
      }
    | WHILE LPA condition RPA statement
      {
          $$ = create_ast_node(WHILE_STMT_NODE);
          add_ast_child($$, $3);
          add_ast_child($$, $5);
      }
    | RETURN expression
      {
          $$ = create_ast_node(RETURN_STMT_NODE);
          add_ast_child($$, $2);
      }
    | LBR statement_list RBR
      {
          $$ = create_ast_node(COMPOUND_STMT_NODE);
          add_ast_child($$, $2);
      }
    | ID LPA arg_list RPA
      {
          $$ = create_ast_node(CALL_STMT_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
          add_ast_child($$, $3);
      }
    ;

expression
    : ID
      {
          $$ = create_ast_node(ID_EXPR_NODE);
          set_string_value($$, $1);
      }
    | FLO
      {
          $$ = create_ast_node(FLOAT_EXPR_NODE);
          set_float_value($$, $1);
      }
    | NUM
      {
          $$ = create_ast_node(NUM_EXPR_NODE);
          set_int_value($$, $1);
      }
    | ID LBK expression RBK
      {
          $$ = create_ast_node(ARRAY_ACCESS_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
          add_ast_child($$, $3);
      }
    | expression ADD expression
      {
          $$ = create_ast_node(BINARY_EXPR_NODE);
          set_op_value($$, ADD);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression MUL expression
      {
          $$ = create_ast_node(BINARY_EXPR_NODE);
          set_op_value($$, MUL);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | LPA expression RPA
      {
          $$ = create_ast_node(PAREN_EXPR_NODE);
          add_ast_child($$, $2);
      }
    | ID LPA arg_list RPA
      {
          $$ = create_ast_node(CALL_EXPR_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
          add_ast_child($$, $3);
      }
    ;

arg_list
    : arg_list arg CMA
      {
          $$ = $1;
          add_ast_child($$, $2);
      }
    | /* empty */
      {
          $$ = create_ast_node(ARG_LIST_NODE);
      }
    ;

arg
    : expression
      {
          $$ = create_ast_node(ARG_NODE);
          add_ast_child($$, $1);
      }
    | ID LBK RBK
      {
          $$ = create_ast_node(ARRAY_ARG_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
      }
    | ID LPA RPA
      {
          $$ = create_ast_node(FUNC_ARG_NODE);
          ASTNode* id_node = create_ast_node(ID_EXPR_NODE);
          set_string_value(id_node, $1);
          add_ast_child($$, id_node);
      }
    ;

condition
    : expression LT expression
      {
          $$ = create_ast_node(RELATIONAL_CONDITION_NODE);
          set_op_value($$, LT);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression LE expression
      {
          $$ = create_ast_node(RELATIONAL_CONDITION_NODE);
          set_op_value($$, LE);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression EQ expression
      {
          $$ = create_ast_node(RELATIONAL_CONDITION_NODE);
          set_op_value($$, EQ);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression NE expression
      {
          $$ = create_ast_node(RELATIONAL_CONDITION_NODE);
          set_op_value($$, NE);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression GT expression
      {
          $$ = create_ast_node(RELATIONAL_CONDITION_NODE);
          set_op_value($$, GT);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression GE expression
      {
          $$ = create_ast_node(RELATIONAL_CONDITION_NODE);
          set_op_value($$, GE);
          add_ast_child($$, $1);
          add_ast_child($$, $3);
      }
    | expression
      {
          $$ = create_ast_node(CONDITION_NODE);
          add_ast_child($$, $1);
      }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d: %s\n", s, yylineno, yytext);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Cannot open file %s\n", argv[1]);
            return 1;
        }
        yyin = file;
    }
    
    int result = yyparse();
    
    if (result == 0 && ast_root != NULL) {
        printf("\n===== 抽象语法树 (AST) =====\n\n");
        print_ast_tree(ast_root, 0);
        printf("\n========================\n");
        
        // 释放AST内存
        free_ast_tree(ast_root);
    } else {
        printf("Parsing failed or no AST generated.\n");
    }
    
    return result;
}