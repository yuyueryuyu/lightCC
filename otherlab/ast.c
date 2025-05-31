#include "ast.h"

/* 创建新的AST节点 */
ASTNode* create_ast_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    node->type = type;
    node->child_count = 0;
    node->children = NULL;
    node->line_number = 0;
    
    // 初始化数据
    memset(&node->data, 0, sizeof(NodeData));
    
    return node;
}

/* 添加子节点 */
void add_ast_child(ASTNode* parent, ASTNode* child) {
    if (parent == NULL || child == NULL) return;
    
    parent->children = (ASTNode**)realloc(parent->children, 
                       (parent->child_count + 1) * sizeof(ASTNode*));
    if (parent->children == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    parent->children[parent->child_count] = child;
    parent->child_count++;
}

/* 设置节点数据 */
void set_int_value(ASTNode* node, int value) {
    if (node) node->data.int_val = value;
}

void set_float_value(ASTNode* node, float value) {
    if (node) node->data.float_val = value;
}

void set_string_value(ASTNode* node, char* value) {
    if (node && value) node->data.str_val = strdup(value);
}

void set_type_value(ASTNode* node, int type) {
    if (node) node->data.type_val = type;
}

void set_op_value(ASTNode* node, int op) {
    if (node) node->data.op_val = op;
}

/* 获取节点类型字符串 */
const char* get_node_type_string(ASTNodeType type) {
    switch(type) {
        case PROGRAM_NODE: return "PROGRAM";
        case DECL_LIST_NODE: return "DECLARATION_LIST";
        case DECL_NODE: return "DECLARATION";
        case VAR_DECL_NODE: return "VARIABLE_DECLARATION";
        case ARRAY_DECL_NODE: return "ARRAY_DECLARATION";
        case FUNC_DECL_NODE: return "FUNCTION_DECLARATION";
        case TYPE_SPEC_NODE: return "TYPE_SPECIFIER";
        case PARAM_LIST_NODE: return "PARAMETER_LIST";
        case PARAM_NODE: return "PARAMETER";
        case ARRAY_PARAM_NODE: return "ARRAY_PARAMETER";
        case FUNC_PARAM_NODE: return "FUNCTION_PARAMETER";
        case STMT_LIST_NODE: return "STATEMENT_LIST";
        case ASSIGN_STMT_NODE: return "ASSIGNMENT_STATEMENT";
        case ARRAY_ASSIGN_STMT_NODE: return "ARRAY_ASSIGNMENT_STATEMENT";
        case IF_STMT_NODE: return "IF_STATEMENT";
        case IF_ELSE_STMT_NODE: return "IF_ELSE_STATEMENT";
        case WHILE_STMT_NODE: return "WHILE_STATEMENT";
        case RETURN_STMT_NODE: return "RETURN_STATEMENT";
        case COMPOUND_STMT_NODE: return "COMPOUND_STATEMENT";
        case CALL_STMT_NODE: return "CALL_STATEMENT";
        case EXPR_NODE: return "EXPRESSION";
        case ID_EXPR_NODE: return "IDENTIFIER";
        case NUM_EXPR_NODE: return "NUMBER";
        case FLOAT_EXPR_NODE: return "FLOAT";
        case ARRAY_ACCESS_NODE: return "ARRAY_ACCESS";
        case BINARY_EXPR_NODE: return "BINARY_EXPRESSION";
        case PAREN_EXPR_NODE: return "PARENTHESIZED_EXPRESSION";
        case CALL_EXPR_NODE: return "FUNCTION_CALL";
        case ARG_LIST_NODE: return "ARGUMENT_LIST";
        case ARG_NODE: return "ARGUMENT";
        case ARRAY_ARG_NODE: return "ARRAY_ARGUMENT";
        case FUNC_ARG_NODE: return "FUNCTION_ARGUMENT";
        case CONDITION_NODE: return "CONDITION";
        case RELATIONAL_CONDITION_NODE: return "RELATIONAL_CONDITION";
        default: return "UNKNOWN";
    }
}

/* 获取类型字符串 */
const char* get_type_string(int type) {
    switch(type) {
        case 1: return "int";     // 假设INT token值为1
        case 2: return "float";   // 假设FLOAT token值为2
        case 3: return "void";    // 假设VOID token值为3
        default: return "unknown_type";
    }
}

/* 获取操作符字符串 */
const char* get_operator_string(int op) {
    switch(op) {
        case 1: return "+";       // ADD
        case 2: return "-";       // SUB
        case 3: return "*";       // MUL
        case 4: return "/";       // DIV
        case 5: return "%";       // MOD
        case 6: return "=";       // ASG
        case 7: return "==";      // EQ
        case 8: return "!=";      // NE
        case 9: return "<";       // LT
        case 10: return "<=";     // LE
        case 11: return ">";      // GT
        case 12: return ">=";     // GE
        case 13: return "&&";     // ANDAND
        case 14: return "||";     // OROR
        default: return "unknown_op";
    }
}

/* 打印AST */
void print_ast_tree(ASTNode* node, int indent) {
    if (node == NULL) return;
    
    /* 打印缩进 */
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    /* 打印节点类型和数据 */
    printf("%s", get_node_type_string(node->type));
    
    switch(node->type) {
        case NUM_EXPR_NODE:
            printf(" (%d)", node->data.int_val);
            break;
        case FLOAT_EXPR_NODE:
            printf(" (%f)", node->data.float_val);
            break;
        case ID_EXPR_NODE:
            if (node->data.str_val)
                printf(" (%s)", node->data.str_val);
            break;
        case TYPE_SPEC_NODE:
            printf(" (%s)", get_type_string(node->data.type_val));
            break;
        case BINARY_EXPR_NODE:
        case RELATIONAL_CONDITION_NODE:
            printf(" (%s)", get_operator_string(node->data.op_val));
            break;
        case ASSIGN_STMT_NODE:
        case ARRAY_ASSIGN_STMT_NODE:
            printf(" (=)");
            break;
        default:
            break;
    }
    
    printf("\n");
    
    /* 递归打印子节点 */
    for (int i = 0; i < node->child_count; i++) {
        print_ast_tree(node->children[i], indent + 1);
    }
}

/* 释放AST内存 */
void free_ast_tree(ASTNode* node) {
    if (node == NULL) return;
    
    /* 释放子节点 */
    for (int i = 0; i < node->child_count; i++) {
        free_ast_tree(node->children[i]);
    }
    
    /* 释放字符串数据 */
    if (node->type == ID_EXPR_NODE && node->data.str_val) {
        free(node->data.str_val);
    }
    
    /* 释放子节点数组和节点本身 */
    free(node->children);
    free(node);
}