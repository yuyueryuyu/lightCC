#include <iostream>
#include <string>
#include <vector>
#include "util/astnodes.hpp"
#include "util/symbol.hpp"
#include "util/symboltable.hpp"
#include "util/error.hpp"
#include "util/type.hpp"
#include "util/ir.hpp"


class IRBuilder {
private:
    IRProgram* irprog{nullptr};
    SymbolTable<IRSym*>* currentST;
    SymbolTable<IRSym*>* globalST;
    BasicBlock* currentBlock{ nullptr };
    IRFunc* currentFunc{nullptr};
    std::vector<Error> errors;

    void err(Node* node, std::string errMsg) {
        Error error("Semantic", node->getPos(), errMsg);
        errors.push_back(error);
    }

    int tempid{0};
    int labelid{0};

    IRSym* genTempSym(IType* type) {
        std::string name = "%"+std::to_string(++tempid);
        auto sym = new IRSym(type, name);
        currentST->put(name, sym);
        return sym;
    }

    IRSym* genLocalSym(IType* type, std::string name) {
        std::string n = "%"+name;
        auto sym = new IRSym(type, n);
        currentST->put(n, sym);
        return sym;
    }
    
    IRSym* genLocalLabel() {
        std::string name = ".L"+std::to_string(++labelid);
        auto sym = new IRSym(&LABEL_TYPE, name);
        currentST->put(name, sym);
        return sym;
    }
public:
    IRBuilder() : globalST(new SymbolTable<IRSym*>()) {
        currentST = globalST;
    }
    
    // 节点访问方法
    void visitNode(Node* node);
    
    IRDef *visitSymbol(Node *node);

    IRVal* visitExpr(Node* node);
    
    // 程序节点
    IRProgram* visitProgram(Program* program);
    
    // 函数声明
    IRDef* visitFunc(FuncDecl* funcDecl);
    
    // 变量声明
    IRDef* visitVar(VarDecl* varDecl);
    
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
    IRVal* visitBinary(Binary* binary);
    
    // 函数调用
    IRVal* visitCall(Call* call);
    
    // 数组索引
    IRVal* visitIndex(Index* index);
    
    // 标识符
    IRVal* visitId(Id* id);
    
    // 整数字面量
    IRVal* visitInt(Int* intLit);
    
    // 浮点数字面量
    IRVal*  visitFloat(Float* floatLit);

    // 浮点数字面量
    IRVal*  visitCast(Cast* cast);

    void outputErrors(std::string file);
    bool hasErr() { return !errors.empty(); }
    void clear() { errors.clear(); }
};
