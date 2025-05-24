#ifndef ASTPRINTER_HPP
#define ASTPRINTER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "util/astnodes.hpp"
#include "util/symbol.hpp"

class PrettyPrinter {
private:
    int indent_level;
    std::string indent_str;
    
    // 增加缩进
    void increaseIndent() {
        indent_level++;
        indent_str = std::string(indent_level * 2, ' ');
    }
    
    // 减少缩进
    void decreaseIndent() {
        indent_level--;
        indent_str = std::string(indent_level * 2, ' ');
    }
    
    // 打印缩进
    void printIndent() {
        std::cout << indent_str;
    }
    
public:
    PrettyPrinter() : indent_level(0), indent_str("") {}
    
    // 节点访问方法
    void visitNode(Node* node, std::ostream& out);
    
    // 程序节点
    void visitProgram(Program* program, std::ostream& out);
    
    // 函数声明
    void visitFuncDecl(FuncDecl* funcDecl, std::ostream& out);

    // 变量声明
    void visitVarDecl(VarDecl* varDecl, std::ostream& out);
    
    // 语句块
    void visitBlock(Block* block, std::ostream& out);
    
    // 赋值语句
    void visitAssign(Assign* assign, std::ostream& out);
    
    // If语句
    void visitIf(If* ifStmt, std::ostream& out);

    // While语句
    void visitWhile(While* whileStmt, std::ostream& out);
    
    // Return语句
    void visitReturn(Return* returnStmt, std::ostream& out);
    
    // 表达式语句
    void visitExprEval(ExprEval* exprEval, std::ostream& out);
    
    // 二元表达式
    void visitBinary(Binary* binary, std::ostream& out);
    
    // 函数调用
    void visitCall(Call* call, std::ostream& out);
    
    // 数组索引
    void visitIndex(Index* index, std::ostream& out);

    // 标识符
    void visitCast(Cast* cast, std::ostream& out);

    // 标识符
    void visitId(Id* id, std::ostream& out);
    
    // 整数字面量
    void visitInt(Int* intLit, std::ostream& out);
    
    // 浮点数字面量
    void visitFloat(Float* floatLit, std::ostream& out);
    
    // 类型
    void visitType(Type* type, std::ostream& out);
    
    // 公共方法：打印AST
    void print(Node* root) {
        visitNode(root, std::cout);
    }

    void output(Node* root, std::ofstream& file) {
        visitNode(root, file);
    }
};

#endif