#include "TypeChecker.hpp"
#include <fstream>
using namespace std;


    
// 节点访问方法
void TypeChecker::visitNode(Node* node) {
    if (!node) return;
    
    // 根据节点类型进行分发
    if (auto program = dynamic_cast<Program*>(node)) {
        visitProgram(program);
    } else if (auto block = dynamic_cast<Block*>(node)) {
        visitBlock(block);
    } else if (auto assign = dynamic_cast<Assign*>(node)) {
        visitAssign(assign);
    } else if (auto ifStmt = dynamic_cast<If*>(node)) {
        visitIf(ifStmt);
    } else if (auto whileStmt = dynamic_cast<While*>(node)) {
        visitWhile(whileStmt);
    } else if (auto returnStmt = dynamic_cast<Return*>(node)) {
        visitReturn(returnStmt);
    } else if (auto exprEval = dynamic_cast<ExprEval*>(node)) {
        visitExprEval(exprEval);
    }
}

Symbol* TypeChecker::visitSymbol(Node *node) {
    if (!node) return nullptr;
    
    if (auto funcDecl = dynamic_cast<FuncDecl*>(node)) {
        return visitFunc(funcDecl);
    } else if (auto varDecl = dynamic_cast<VarDecl*>(node)) {
        return visitVar(varDecl);
    }

    return nullptr;
}

IType* TypeChecker::visitExpr(Node* node) {
    if (!node) return nullptr;
    
    if (auto binary = dynamic_cast<Binary*>(node)) {
        return visitBinary(binary);
    } else if (auto call = dynamic_cast<Call*>(node)) {
        return visitCall(call);
    } else if (auto index = dynamic_cast<Index*>(node)) {
        return visitIndex(index);
    } else if (auto id = dynamic_cast<Id*>(node)) {
        return visitId(id);
    } else if (auto intLit = dynamic_cast<Int*>(node)) {
        return visitInt(intLit);
    } else if (auto floatLit = dynamic_cast<Float*>(node)) {
        return visitFloat(floatLit);
    }

    return nullptr;
}

// 程序节点
void TypeChecker::visitProgram(Program* program) {
    for (auto decl : program->getDecls()) {
        visitSymbol(decl);
    }
    
    for (auto stmt : program->getStmts()) {
        visitNode(stmt);
    }

    program->setST(globalST);
}



// 函数声明
Symbol* TypeChecker::visitFunc(FuncDecl* funcDecl) {
    BType *retType = new BType(funcDecl->getRetType());
    string name = funcDecl->getId()->getName();

    Func *func = new Func(name, retType);
    funcDecl->resolve(func);
    currentFunc = func;

    if (currentST->declares(func->getName())) {
        err(funcDecl, "redefining function: " + func->getName());
    } else {
        currentST->put(func->getName(), func);
    }

    currentST = new SymbolTable<Symbol*>(currentST);
    for (auto param : funcDecl->getParams()) {
        Symbol *sym = visitSymbol(param);
        if (sym != nullptr) {
            func->addParam(sym);
            if (auto f = dynamic_cast<Func*>(sym)) {
                f->setParam();
            }
        }
    }
    
    for (auto decl : funcDecl->getDecls()) {
        if (auto funcDecl = dynamic_cast<FuncDecl*>(decl)) {
            err(funcDecl, "defining function within function body: " + func->getName());
        } else {
            Symbol *sym = visitSymbol(decl);
            if (sym != nullptr) {
                func->addLocal(sym);
            }
        }
    }
    
    for (auto stmt : funcDecl->getStmts()) {
        visitNode(stmt);
    }
    funcDecl->setST(currentST);
    currentST = currentST->getParent();
    return func;
}

// 变量声明
Symbol* TypeChecker::visitVar(VarDecl* varDecl) {
    
    BType *type = new BType(varDecl->getType());

    if (varDecl->getId() == nullptr) {
        if (type->equals(&VOID_TYPE)) {
            return nullptr;
        }
        return new Var("", type);
    }
    string name = varDecl->getId()->getName();

    if (type->equals(&VOID_TYPE)) {
        err(varDecl, "defining void type variable: " + name);
        return nullptr;
    }

    int len = varDecl->getLen();
    if (len == 0) {
        Var *sym = new Var(name, type);
        varDecl->resolve(sym);
        if (currentST->declares(sym->getName())) {
            err(varDecl, "redefining variable: " + name);
        } else {
            currentST->put(sym->getName(), sym);
        }
        return sym;

    } else {
        Var *sym = new Var(name, new AType(type, len));
        varDecl->resolve(sym);
        if (currentST->declares(sym->getName())) {
            err(varDecl, "redefining variable: " + name);
        } else {
            currentST->put(sym->getName(), sym);
        }
        return sym;
    }

}

// 语句块
void TypeChecker::visitBlock(Block* block) {
    for (auto stmt : block->getBody()) {
        visitNode(stmt);
    }
}

// 赋值语句
void TypeChecker::visitAssign(Assign* assign) {
    IType* valType = visitExpr(assign->getValue());

    if (dynamic_cast<Id*>(assign->getTarget()) == nullptr && 
        dynamic_cast<Index*>(assign->getTarget()) == nullptr) {
        err(assign, "assign target is not a valid lvalue");
        return;
    }

    IType* varType = visitExpr(assign->getTarget());

    if (auto it = dynamic_cast<AType*>(varType)) {
        err(assign, "cannot assign to an array");
        return;
    }

    if (auto it = dynamic_cast<AType*>(valType)) {
        err(assign, "array cannot be assigned");
        return;
    }

    if (auto it = dynamic_cast<FType*>(varType)) {
        err(assign, "cannot assign to a function");
        return;
    }

    if (auto it = dynamic_cast<FType*>(valType)) {
        err(assign, "function cannot be assigned");
        return;
    }

    if (auto var = dynamic_cast<BType*>(varType)) {
        if (auto val = dynamic_cast<BType*>(valType)) {
            if (var->equals(&VOID_TYPE) || val->equals(&VOID_TYPE)) {
                err(assign, "cannot assign void type");
                return;
            }
            if (!var->equals(val))
                assign->castVal(val, var);
        }
    }
}

// If语句
void TypeChecker::visitIf(If* ifStmt) {
    IType* type = visitExpr(ifStmt->getCond());
    if (!type->equals(&BOOL_TYPE)) ifStmt->castCond(type);

    visitNode(ifStmt->getThenStmt());
    
    if (ifStmt->getElseStmt()) {
        visitNode(ifStmt->getElseStmt());
    }
}

// While语句
void TypeChecker::visitWhile(While* whileStmt) {

    IType* type = visitExpr(whileStmt->getCond());
    if (!type->equals(&BOOL_TYPE)) whileStmt->castCond(type);

    visitNode(whileStmt->getBody());
}

// Return语句
void TypeChecker::visitReturn(Return* returnStmt) {
    
    if (returnStmt->getValue()) {
        IType * retType = currentFunc->getRetType();

        if (auto ret = dynamic_cast<BType*>(retType)) {
            IType * type = visitExpr(returnStmt->getValue());
            if (auto it = dynamic_cast<AType*>(type)) {
                err(returnStmt, "return type not compatible");
                return;
            }

            if (auto it = dynamic_cast<FType*>(type)) {
                err(returnStmt, "return type not compatible");
                return;
            }

            if (auto val = dynamic_cast<BType*>(type)) {
                if (val->equals(&VOID_TYPE) ) {
                    err(returnStmt, "return type not compatible");
                    return;
                }
                if (!ret->equals(val))
                    returnStmt->castVal(val, ret);
            }
        } else {
            err(returnStmt, "return type not compatible");
            return;
        }
    } 
}

// 表达式语句
void TypeChecker::visitExprEval(ExprEval* exprEval) {
    visitExpr(exprEval->getExpr());
}

// 二元表达式
IType* TypeChecker::visitBinary(Binary* binary) {
    char op = binary->getOp();
    IType *lhs = visitExpr(binary->getLeft());
    IType *rhs = visitExpr(binary->getRight());

    if (auto it = dynamic_cast<AType*>(lhs)) {
        err(binary, "left operand type not compatible in binary expression");
    }

    if (auto it = dynamic_cast<AType*>(rhs)) {
        err(binary, "right operand type not compatible in binary expression");
    }

    if (auto it = dynamic_cast<FType*>(lhs)) {
        err(binary, "left operand type not compatible in binary expression");
    }

    if (auto it = dynamic_cast<FType*>(rhs)) {
        err(binary, "right operand type not compatible in binary expression");
    }

    if (auto ltype = dynamic_cast<BType*>(lhs)) {
        if (auto rtype = dynamic_cast<BType*>(rhs)) {
            if (ltype->equals(&VOID_TYPE) || rtype->equals(&VOID_TYPE)) {
                err(binary, "void type not compatible in binary expression");
            } else if (!ltype->equals(rtype)) {
                if (ltype->equals(&INT_TYPE)) {
                    binary->castLeft(ltype, rtype);
                    lhs = rtype;
                }
                if (rtype->equals(&INT_TYPE)) {
                    binary->castRight(rtype, ltype);
                    rhs = ltype;
                }
            }
        }
    }

    switch (op) {
        case '+': 
        case '*':
            binary->setType(lhs); 
            return lhs;
        case '=':
        case '<':
        case 'l':
            binary->setType(&BOOL_TYPE); 
            return &BOOL_TYPE;
        default: 
            binary->setType(lhs); 
            return lhs;
    }
}

// 函数调用
IType* TypeChecker::visitCall(Call* call) {
    if (!currentST->declaresRecursive(call->getId()->getName())) {
        err(call, "undeclared function: "+ call->getId()->getName());
        call->setType(new BType(INT_TYPE));
        return call->getType();
    }
    Symbol *sym = *currentST->getRecursive(call->getId()->getName());
    
    if (auto func = dynamic_cast<Func*>(sym)) {
        call->resolve(func);
        if (func->getParams().size() != call->getArgs().size()) {
            err(call, "function arguments doesn't match: expecting " + to_string(func->getParams().size()) + ", actual " +  to_string(call->getArgs().size()));
            return func->getRetType();
        }
        for (size_t i = 0; i < call->getArgs().size(); i++) {
            IType *argType = visitExpr(call->getArgs()[i]);
            IType *paramType = func->getParams()[i]->getType(); 
            if (auto arg = dynamic_cast<BType*>(argType)) {
                if (auto param = dynamic_cast<BType*>(paramType)) {
                    if (arg->equals(&VOID_TYPE) || param->equals(&VOID_TYPE)) {
                        err(call, "void type cannot be used as argument");
                    }
                    if (!param->equals(arg)) {
                        call->castArg(arg, param, i);
                    }
                } else if (!paramType->equals(argType)) {
                    err(call, "argument type doesn't match");
                }
            } else if (!paramType->equals(argType)) {
                err(call, "argument type doesn't match");
            }
        }
        call->setType(func->getRetType());
        return func->getRetType();
    } else {

        err(call, "not a function: " + call->getId()->getName());
        call->setType(new BType(INT_TYPE));
        return call->getType();
    }
}

// 数组索引
IType* TypeChecker::visitIndex(Index* index) {
    IType *type = visitExpr(index->getId());
    if (auto it = dynamic_cast<AType*>(type)) {
        IType *dim = visitExpr(index->getIndex());
        if (!dim->equals(&INT_TYPE)) {
            err(index, "dimension is not integer");
        }
        index->setType(it->getBase());
        return it->getBase();
    } else {
        err(index, "not an array: " + index->getId()->getName());
        index->setType(new BType(INT_TYPE));
        return index->getType();
    }
}

// 标识符
IType* TypeChecker::visitId(Id* id) {
    string name = id->getName();
    if (!currentST->declaresRecursive(name)) {
        err(id, "undeclared variable: " + name);
        id->setType(new BType(INT_TYPE));
        return id->getType();
    }
    Symbol *sym = *currentST->getRecursive(name);
    id->resolve(sym);
    id->setType(sym->getType());
    return sym->getType();
}

// 整数字面量
IType* TypeChecker::visitInt(Int* intLit) {
    intLit->setType(&INT_TYPE);
    return &INT_TYPE;
}

// 浮点数字面量
IType* TypeChecker::visitFloat(Float* floatLit) {
    floatLit->setType(&FLOAT_TYPE);
    return &FLOAT_TYPE;
}

void TypeChecker::outputErrors(string file) {
    ofstream f(file);
    if (!f.is_open()) {
        cerr << "无法打开文件: " << file << endl;
        return;
    }
    for (auto e : errors) {
        f << e.toString() << endl;
    }
    f.close();
}