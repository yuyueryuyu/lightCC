#include <iostream>
#include <string>
#include <vector>
#include "util/astnodes.hpp"
#include "util/symbol.hpp"
#include "util/symboltable.hpp"
#include "util/error.hpp"
#include "util/type.hpp"

class TypeChecker {
private:
    SymbolTable<Symbol *>* currentST;
    SymbolTable<Symbol *>* globalST;
    Func* currentFunc{ nullptr };
    std::vector<Error> errors;
    void err(Node* node, std::string errMsg) {
        Error error("Semantic", node->getPos(), errMsg);
        errors.push_back(error);
    }
public:
    TypeChecker() : globalST(new SymbolTable<Symbol*>()) {
        currentST = globalST;
    }
    
    // 节点访问方法
    void visitNode(Node* node);
    
    Symbol *visitSymbol(Node *node);

    IType* visitExpr(Node* node);
    
    // 程序节点
    void visitProgram(Program* program);
    
    // 函数声明
    Symbol *visitFunc(FuncDecl* funcDecl);
    
    // 变量声明
    Symbol* visitVar(VarDecl* varDecl);
    
    // 语句块
    void visitBlock(Block* block);
    
    // 赋值语句
    void visitAssign(Assign* assign);
    
    // If语句
    void visitIf(If* ifStmt);
    
    // While语句
    void visitWhile(While* whileStmt);
    
    // Return语句
    void visitReturn(Return* returnStmt);
    
    // 表达式语句
    void visitExprEval(ExprEval* exprEval);
    
    // 二元表达式
    IType* visitBinary(Binary* binary);
    
    // 函数调用
    IType* visitCall(Call* call);
    
    // 数组索引
    IType* visitIndex(Index* index);
    
    // 标识符
    IType* visitId(Id* id);
    
    // 整数字面量
    IType* visitInt(Int* intLit);
    
    // 浮点数字面量
    IType*  visitFloat(Float* floatLit);

    void outputErrors(std::string file);
    bool hasErr() { return !errors.empty(); }
    void clear() { errors.clear(); }
};
