#include "IRBuilder.hpp"

void IRBuilder::visitNode(Node* node) {
    if (!node) return;
    
    // 根据节点类型进行分发
    if (auto block = dynamic_cast<Block*>(node)) {
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

IRDef* IRBuilder::visitSymbol(Node *node) {
    if (!node) return nullptr;
    
    if (auto funcDecl = dynamic_cast<FuncDecl*>(node)) {
        return visitFunc(funcDecl);
    } else if (auto varDecl = dynamic_cast<VarDecl*>(node)) {
        return visitVar(varDecl);
    }

    return nullptr;
}

IRVal* IRBuilder::visitExpr(Node* node) {
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
    } else if (auto cast = dynamic_cast<Cast*>(node)) {
        return visitCast(cast);
    }

    return nullptr;
}
// 程序节点
IRProgram* IRBuilder::visitProgram(Program* program) {
    IRProgram* irProg = new IRProgram();
    for (auto decl : program->getDecls()) {
        auto sym = visitSymbol(decl);
        irProg->addDecl(sym);
    }
    std::vector<IType*> v;
    std::vector<IRSym*> v2;
    auto main_type = new FType(&VOID_TYPE, v);    
    auto main_func = new IRFunc(new IRSym(main_type, "__main__"), v2);
    currentBlock = new BasicBlock(genLocalLabel());
    currentFunc = main_func;
    currentFunc->addBlock(currentBlock);
    for (auto stmt : program->getStmts()) {
        visitNode(stmt);
    }
    currentBlock->addInstr(new IRRet());
    irProg->addDecl(main_func);
    return irProg;
}

// 函数声明
IRDef* IRBuilder::visitFunc(FuncDecl* funcDecl) {
    Func* func = funcDecl->getResolution();
    IRFunc* irFunc = new IRFunc(func);
    currentST->put("@"+func->getName(), irFunc->getSym());
    
    if (func->isParameter()) return irFunc;
    
    currentST = new SymbolTable<IRSym*>(currentST);
    currentBlock = new BasicBlock(genLocalLabel());
    currentFunc = irFunc;
    
    currentFunc->addBlock(currentBlock);
    for (int i = 0; i < func->getParams().size(); i++) {
        auto param = func->getParams()[i];
        auto paramSym = irFunc->getParams()[i];
        currentST->put(paramSym->getName(), paramSym);

        auto paramType = param->getType();
        auto temp = genLocalSym(paramType, param->getName());
        auto alloc = new IRAlloc(temp, paramType);
        currentBlock->addInstr(alloc);
        
        auto store = new IRStore(paramSym, temp);
        currentBlock->addInstr(store);
    }
    
    for (auto decl : funcDecl->getDecls()) {
        auto varDecl = dynamic_cast<VarDecl*>(decl);
        IRDef* var = visitSymbol(decl);
        auto temp = genLocalSym(new PType(var->getSym()->getType()), varDecl->getId()->getName());
        auto alloc = new IRAlloc(temp, var->getSym()->getType());
        currentBlock->addInstr(alloc);
    }
    
    for (auto stmt : funcDecl->getStmts()) {
        visitNode(stmt);
    }
    currentBlock->addInstr(new IRRet());
    irFunc->setST(currentST);
    currentST = currentST->getParent();
    currentBlock = nullptr;
    currentFunc = nullptr;
    return irFunc;
}

// 变量声明
IRDef* IRBuilder::visitVar(VarDecl* varDecl) {
    Var* var = varDecl->getResolution();
    IRVar* irVar = new IRVar(var);
    currentST->put("@"+var->getName(), new IRSym(new PType(var->getType()), "@"+var->getName()));
    return irVar;
}

// 语句块
void IRBuilder::visitBlock(Block* block) {
    for (auto stmt : block->getBody()) {
        visitNode(stmt);
    }
}

// 赋值语句
void IRBuilder::visitAssign(Assign* assign) {
    auto val = visitExpr(assign->getValue());
    if (auto id = dynamic_cast<Id*>(assign->getTarget())) {
        if (currentST->declares("%"+id->getName())) {
            auto sym = *currentST->get("%"+id->getName());
            auto store = new IRStore(val, sym);
            currentBlock->addInstr(store);
        } else if (currentST->declaresRecursive("@"+id->getName())) {
            auto sym = *currentST->getRecursive("@"+id->getName());
            auto store = new IRStore(val, sym);
            currentBlock->addInstr(store);
        }
    } else if(auto index = dynamic_cast<Index*>(assign->getTarget())) {
        auto idx = visitExpr(index->getIndex());
        auto id = index->getId();
        if (currentST->declares("%"+id->getName())) {
            auto sym = *currentST->get("%"+id->getName());
            auto elAddr = genTempSym(new PType(index->getType()));
            currentBlock->addInstr(new IRGetElPtr(elAddr, sym, idx));
            auto store = new IRStore(val, elAddr);
            currentBlock->addInstr(store);
        } else if (currentST->declaresRecursive("@"+id->getName())) {
            auto sym = *currentST->getRecursive("@"+id->getName());
            auto elAddr = genTempSym(new PType(index->getType()));
            currentBlock->addInstr(new IRGetElPtr(elAddr, sym, idx));
            auto store = new IRStore(val, sym);
            currentBlock->addInstr(store);
        }
    }
}

// If语句
void IRBuilder::visitIf(If* ifStmt) {
    auto val = visitExpr(ifStmt->getCond());
    IRSym* thenLabel = genLocalLabel();
    IRSym* elseLabel = genLocalLabel();
    IRSym* endLabel = genLocalLabel();
    auto br = new IRBr(val, thenLabel, elseLabel);
    currentBlock->addInstr(br);
    
    currentBlock = new BasicBlock(thenLabel);
    currentFunc->addBlock(currentBlock);
    visitNode(ifStmt->getThenStmt());
    currentBlock->addInstr(new IRJump(endLabel));

    currentBlock = new BasicBlock(elseLabel);
    currentFunc->addBlock(currentBlock);
    visitNode(ifStmt->getElseStmt());
    currentBlock->addInstr(new IRJump(endLabel));
    
    currentBlock = new BasicBlock(endLabel);
    currentFunc->addBlock(currentBlock);
}

// While语句
void IRBuilder::visitWhile(While* whileStmt) {
    auto val = visitExpr(whileStmt->getCond());
    IRSym* loopLabel = genLocalLabel();
    IRSym* endLabel = genLocalLabel();
    auto br = new IRBr(val, loopLabel, endLabel);
    currentBlock->addInstr(br);
    
    currentBlock = new BasicBlock(loopLabel);
    currentFunc->addBlock(currentBlock);
    visitNode(whileStmt->getBody());
    currentBlock->addInstr(new IRJump(loopLabel));

    currentBlock = new BasicBlock(endLabel);
    currentFunc->addBlock(currentBlock);
}

// Return语句
void IRBuilder::visitReturn(Return* returnStmt) {
    auto val = visitExpr(returnStmt->getValue());
    currentBlock->addInstr(new IRRet(val));
    currentBlock = new BasicBlock(genLocalLabel());
    currentFunc->addBlock(currentBlock);
}

// 表达式语句
void IRBuilder::visitExprEval(ExprEval* exprEval) {
    if (auto call = dynamic_cast<Call*>(exprEval->getExpr())) {
        std::vector<IRVal*> args;
        for (auto arg : call->getArgs()) {
            args.push_back(visitExpr(arg));
        }
        if (currentST->declares("%"+call->getId()->getName())) {
            auto funcPointer = *currentST->get("%"+call->getId()->getName());
            auto funcAddr = genTempSym(funcPointer->getType());
            currentBlock->addInstr(new IRLoad(funcAddr, funcPointer));
            auto call = new IRCall(funcAddr, args);
            currentBlock->addInstr(call);
        } else if (currentST->declaresRecursive("@"+call->getId()->getName())) {
            auto funcLabel = *currentST->getRecursive("@"+call->getId()->getName());
            auto call = new IRCall(funcLabel, args);
            currentBlock->addInstr(call);
        }
    }
}

// 二元表达式
IRVal* IRBuilder::visitBinary(Binary* binary) {
    auto lhs = visitExpr(binary->getLeft());
    auto rhs = visitExpr(binary->getRight());
    auto sym = genTempSym(binary->getType());
    currentBlock->addInstr(new IRBinary(sym, lhs, rhs, binary->getOp()));
    return sym;
}

// 函数调用
IRVal* IRBuilder::visitCall(Call* call) {
    auto sym = genTempSym(call->getType());
    std::vector<IRVal*> args;
    for (auto arg : call->getArgs()) {
        args.push_back(visitExpr(arg));
    }
    if (currentST->declares("%"+call->getId()->getName())) {
        auto funcPointer = *currentST->get("%"+call->getId()->getName());
        auto funcAddr = genTempSym(funcPointer->getType());
        currentBlock->addInstr(new IRLoad(funcAddr, funcPointer));
        auto call = new IRCall(sym, funcAddr, args);
        currentBlock->addInstr(call);
    } else if (currentST->declaresRecursive("@"+call->getId()->getName())) {
        auto funcLabel = *currentST->getRecursive("@"+call->getId()->getName());
        auto call = new IRCall(sym, funcLabel, args);
        currentBlock->addInstr(call);
    }
    return sym;
}

// 数组索引
IRVal* IRBuilder::visitIndex(Index* index) {
    auto sym = genTempSym(index->getType());
    auto idx = visitExpr(index->getIndex());
    if (currentST->declares("%"+index->getId()->getName())) {
        auto arr = *currentST->get("%"+index->getId()->getName());
        auto elAddr = genTempSym(new PType(index->getType()));
        currentBlock->addInstr(new IRGetElPtr(elAddr, arr, idx));
        currentBlock->addInstr(new IRLoad(sym, elAddr));
    } else if (currentST->declaresRecursive("@"+index->getId()->getName())) {
        auto arr = *currentST->getRecursive("@"+index->getId()->getName());
        auto elAddr = genTempSym(new PType(index->getType()));
        currentBlock->addInstr(new IRGetElPtr(elAddr, arr, idx));
        currentBlock->addInstr(new IRLoad(sym, elAddr));
    }
    return sym;
}

// 标识符
IRVal* IRBuilder::visitId(Id* id) {
    auto sym = genTempSym(id->getType());
    if (currentST->declares("%"+id->getName())) {
        auto sym_addr = *currentST->get("%"+id->getName());
        currentBlock->addInstr(new IRLoad(sym, sym_addr));
    } else if (currentST->declaresRecursive("@"+id->getName())) {
        auto sym_addr = *currentST->getRecursive("@"+id->getName());
        currentBlock->addInstr(new IRLoad(sym, sym_addr));
    }
    return sym;
}

// 整数字面量
IRVal* IRBuilder::visitInt(Int* intLit) {
    return new IRInt(intLit->getValue());
}

// 浮点数字面量
IRVal* IRBuilder::visitFloat(Float* floatLit) {
    return new IRFlo(floatLit->getValue());
}

IRVal* IRBuilder::visitCast(Cast* cast) {
    auto toType = cast->getTo();
    auto fromType = cast->getFrom();
    auto sym = genTempSym(cast->getTo());
    auto expr = visitExpr(cast->getExpr());
    if (toType->equals(&BOOL_TYPE)) {
        if (fromType->equals(&INT_TYPE))
            currentBlock->addInstr(new IRBinary(sym, expr, new IRInt(0), '!'));
        if (fromType->equals(&FLOAT_TYPE))
            currentBlock->addInstr(new IRBinary(sym, expr, new IRFlo(0.0),'!'));
    }
    if (toType->equals(&INT_TYPE) && fromType->equals(&FLOAT_TYPE)) {
        currentBlock->addInstr(new IRF2I(sym, expr));
    }
    if (toType->equals(&FLOAT_TYPE) && fromType->equals(&INT_TYPE)) {
        currentBlock->addInstr(new IRI2F(sym, expr));
    }
    return sym;
}