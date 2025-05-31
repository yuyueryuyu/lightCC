#ifndef PARSETREE_HPP
#define PARSETREE_HPP

#include <vector>
#include <string>
#include "token.hpp"

/// @brief 解析树结点
struct ParseTreeNode {
    /// @brief 符号名称
    std::string symbol;
    /// @brief 是否为终结符
    bool is_terminal;      
    /// @brief 子节点
    std::vector<ParseTreeNode*> children;
    /// @brief 如果是终结符，存储token值
    std::string token_value;
    /// @brief 对应的token
    Token start;
    Token end;

    /// @brief 构造函数 
    /// @param sym 符号名称
    /// @param is_term 是否终结符
    /// @param value token值
    ParseTreeNode(const std::string& sym, Token start, Token end) 
        : symbol(sym), is_terminal(false), token_value(""), start(start), end(end) {}

    ParseTreeNode(const std::string& sym, Token terminal) 
        : symbol(sym), is_terminal(true), token_value(terminal.getValue()), start(terminal), end(terminal) {}

    /// @brief 析构函数
    ~ParseTreeNode() {
        for (ParseTreeNode* child : children) {
            delete child;
        }
    }
    
    /// @brief 打印解析树
    /// @param depth 缩进
    void print(int depth = 0) const;
    
    /// @brief 转换为JSON字符串
    /// @param depth 缩进深度
    /// @return 
    std::string toJSON(int depth = 0) const;
};

#endif