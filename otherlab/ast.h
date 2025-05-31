#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* AST节点类型 */
typedef enum {
    PROGRAM_NODE,
    DECL_LIST_NODE,
    DECL_NODE,
    VAR_DECL_NODE,
    ARRAY_DECL_NODE,
    FUNC_DECL_NODE,
    TYPE_SPEC_NODE,
    PARAM_LIST_NODE,
    PARAM_NODE,
    ARRAY_PARAM_NODE,
    FUNC_PARAM_NODE,
    STMT_LIST_NODE,
    ASSIGN_STMT_NODE,
    ARRAY_ASSIGN_STMT_NODE,
    IF_STMT_NODE,
    IF_ELSE_STMT_NODE,
    WHILE_STMT_NODE,
    RETURN_STMT_NODE,
    COMPOUND_STMT_NODE,
    CALL_STMT_NODE,
    EXPR_NODE,
    ID_EXPR_NODE,
    NUM_EXPR_NODE,
    FLOAT_EXPR_NODE,
    ARRAY_ACCESS_NODE,
    BINARY_EXPR_NODE,
    PAREN_EXPR_NODE,
    CALL_EXPR_NODE,
    ARG_LIST_NODE,
    ARG_NODE,
    ARRAY_ARG_NODE,
    FUNC_ARG_NODE,
    CONDITION_NODE,
    RELATIONAL_CONDITION_NODE
} ASTNodeType;

/* 节点数据的联合体 */
typedef union {
    int int_val;
    float float_val;
    char *str_val;
    int type_val;  /* INT, FLOAT, VOID */
    int op_val;    /* 操作符 */
} NodeData;

/* AST节点结构 */
typedef struct ast_node {
    ASTNodeType type;
    NodeData data;
    int child_count;
    struct ast_node **children;
    int line_number;
} ASTNode;

/* 创建新的AST节点 */
ASTNode* create_ast_node(ASTNodeType type);

/* 添加子节点 */
void add_ast_child(ASTNode* parent, ASTNode* child);

/* 设置节点数据 */
void set_int_value(ASTNode* node, int value);
void set_float_value(ASTNode* node, float value);
void set_string_value(ASTNode* node, char* value);
void set_type_value(ASTNode* node, int type);
void set_op_value(ASTNode* node, int op);

/* 打印AST */
void print_ast_tree(ASTNode* root, int indent);

/* 释放AST内存 */
void free_ast_tree(ASTNode* root);

/* 获取节点类型字符串 */
const char* get_node_type_string(ASTNodeType type);

/* 获取类型字符串 */
const char* get_type_string(int type);

/* 获取操作符字符串 */
const char* get_operator_string(int op);

#endif /* AST_H */