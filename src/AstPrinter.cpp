#include "AstPrinter.hpp"
using namespace std;

// 节点访问方法
void PrettyPrinter::visitNode(Node* node, ostream& out) {
    if (!node) return;
    
    // 根据节点类型进行分发
    if (auto program = dynamic_cast<Program*>(node)) {
        visitProgram(program, out);
    } else if (auto funcDecl = dynamic_cast<FuncDecl*>(node)) {
        visitFuncDecl(funcDecl, out);
    } else if (auto varDecl = dynamic_cast<VarDecl*>(node)) {
        visitVarDecl(varDecl, out);
    } else if (auto block = dynamic_cast<Block*>(node)) {
        visitBlock(block, out);
    } else if (auto assign = dynamic_cast<Assign*>(node)) {
        visitAssign(assign, out);
    } else if (auto ifStmt = dynamic_cast<If*>(node)) {
        visitIf(ifStmt, out);
    } else if (auto whileStmt = dynamic_cast<While*>(node)) {
        visitWhile(whileStmt, out);
    } else if (auto returnStmt = dynamic_cast<Return*>(node)) {
        visitReturn(returnStmt, out);
    } else if (auto exprEval = dynamic_cast<ExprEval*>(node)) {
        visitExprEval(exprEval, out);
    } else if (auto cast = dynamic_cast<Cast*>(node)) {
        visitCast(cast, out);
    } else if (auto binary = dynamic_cast<Binary*>(node)) {
        visitBinary(binary, out);
    } else if (auto call = dynamic_cast<Call*>(node)) {
        visitCall(call, out);
    } else if (auto index = dynamic_cast<Index*>(node)) {
        visitIndex(index, out);
    } else if (auto id = dynamic_cast<Id*>(node)) {
        visitId(id, out);
    } else if (auto intLit = dynamic_cast<Int*>(node)) {
        visitInt(intLit, out);
    } else if (auto floatLit = dynamic_cast<Float*>(node)) {
        visitFloat(floatLit, out);
    } else if (auto type = dynamic_cast<Type*>(node)) {
        visitType(type, out);
    }
}

// 程序节点
void PrettyPrinter::visitProgram(Program* program, ostream& out) {
    printIndent();
    out << "Program {" << endl;
    increaseIndent();
    
    // 打印声明
    if (!program->getDecls().empty()) {
        printIndent();
        out << "Declarations:" << endl;
        increaseIndent();
        for (auto decl : program->getDecls()) {
            visitNode(decl, out);
        }
        decreaseIndent();
    }
    
    // 打印语句
    if (!program->getStmts().empty()) {
        printIndent();
        out << "Statements:" << endl;
        increaseIndent();
        for (auto stmt : program->getStmts()) {
            visitNode(stmt, out);
        }
        decreaseIndent();
    }
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// 函数声明
void PrettyPrinter::visitFuncDecl(FuncDecl* funcDecl, ostream& out) {
    printIndent();
    out << "FuncDecl {" << endl;
    increaseIndent();
    
    printIndent();
    out << "ReturnType: ";
    visitNode(funcDecl->getRetType(), out);
    out << endl;
    
    printIndent();
    out << "Name: ";
    visitNode(funcDecl->getId(), out);
    out << endl;
    
    if (!funcDecl->getParams().empty()) {
        printIndent();
        out << "Parameters:" << endl;
        increaseIndent();
        for (auto param : funcDecl->getParams()) {
            visitNode(param, out);
        }
        decreaseIndent();
    }
    
    if (!funcDecl->getDecls().empty()) {
        printIndent();
        out << "LocalDeclarations:" << endl;
        increaseIndent();
        for (auto decl : funcDecl->getDecls()) {
            visitNode(decl, out);
        }
        decreaseIndent();
    }
    
    if (!funcDecl->getStmts().empty()) {
        printIndent();
        out << "Body:" << endl;
        increaseIndent();
        for (auto stmt : funcDecl->getStmts()) {
            visitNode(stmt, out);
        }
        decreaseIndent();
    }
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// 变量声明
void PrettyPrinter::visitVarDecl(VarDecl* varDecl, ostream& out) {
    printIndent();
    out << "VarDecl {" << endl;
    increaseIndent();
    
    printIndent();
    out << "Type: ";
    visitNode(varDecl->getType(), out);
    out << endl;
    
    printIndent();
    out << "Name: ";
    visitNode(varDecl->getId(), out);
    out << endl;
    
    if (varDecl->getLen() > 0) {
        printIndent();
        out << "Dimension: " << varDecl->getLen() << endl;
    }
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// 语句块
void PrettyPrinter::visitBlock(Block* block, ostream& out) {
    printIndent();
    out << "Block {" << endl;
    increaseIndent();
    
    for (auto stmt : block->getBody()) {
        visitNode(stmt, out);
    }
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// 赋值语句
void PrettyPrinter::visitAssign(Assign* assign, ostream& out) {
    printIndent();
    out << "Assign {" << endl;
    increaseIndent();
    
    printIndent();
    out << "Target: ";
    visitNode(assign->getTarget(), out);
    out << endl;
    
    printIndent();
    out << "Value: ";
    visitNode(assign->getValue(), out);
    out << endl;
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// If语句
void PrettyPrinter::visitIf(If* ifStmt, ostream& out) {
    printIndent();
    out << "If {" << endl;
    increaseIndent();
    
    printIndent();
    out << "Condition: ";
    visitNode(ifStmt->getCond(), out);
    out << endl;
    
    printIndent();
    out << "ThenStmt:" << endl;
    increaseIndent();
    visitNode(ifStmt->getThenStmt(), out);
    decreaseIndent();
    
    if (ifStmt->getElseStmt()) {
        printIndent();
        out << "ElseStmt:" << endl;
        increaseIndent();
        visitNode(ifStmt->getElseStmt(), out);
        decreaseIndent();
    }
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// While语句
void PrettyPrinter::visitWhile(While* whileStmt, ostream& out) {
    printIndent();
    out << "While {" << endl;
    increaseIndent();
    
    printIndent();
    out << "Condition: ";
    visitNode(whileStmt->getCond(), out);
    out << endl;
    
    printIndent();
    out << "Body:" << endl;
    increaseIndent();
    visitNode(whileStmt->getBody(), out);
    decreaseIndent();
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// Return语句
void PrettyPrinter::visitReturn(Return* returnStmt, ostream& out) {
    printIndent();
    out << "Return {" << endl;
    increaseIndent();
    
    if (returnStmt->getValue()) {
        printIndent();
        out << "Value: ";
        visitNode(returnStmt->getValue(), out);
        out << endl;
    } else {
        printIndent();
        out << "Value: void" << endl;
    }
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// 表达式语句
void PrettyPrinter::visitExprEval(ExprEval* exprEval, ostream& out) {
    printIndent();
    out << "ExprEval {" << endl;
    increaseIndent();
    
    printIndent();
    out << "Expression: ";
    visitNode(exprEval->getExpr(), out);
    out << endl;
    
    decreaseIndent();
    printIndent();
    out << "}" << endl;
}

// 二元表达式
void PrettyPrinter::visitBinary(Binary* binary, ostream& out) {
    out << "Binary(" << binary->getOp() << ") {";
    visitNode(binary->getLeft(), out);
    out << ", ";
    visitNode(binary->getRight(), out);
    out << "}";
}

// 函数调用
void PrettyPrinter::visitCall(Call* call, ostream& out) {
    out << "Call {";
    visitNode(call->getId(), out);
    out << "(";
    for (size_t i = 0; i < call->getArgs().size(); i++) {
        if (i > 0) out << ", ";
        visitNode(call->getArgs()[i], out);
    }
    out << ")}";
}

// 数组索引
void PrettyPrinter::visitIndex(Index* index, ostream& out) {
    out << "Index {";
    visitNode(index->getId(), out);
    out << "[";
    visitNode(index->getIndex(), out);
    out << "]}";
}

// 标识符
void PrettyPrinter::visitCast(Cast* cast, ostream& out) {
    out << "Cast(from=" << cast->getFrom()->toString() << ", to=" << cast->getTo()->toString() << "(";
    visitNode(cast->getExpr(), out);
    out<<")";
}

// 标识符
void PrettyPrinter::visitId(Id* id, ostream& out) {
    out << "Id(" << id->getName() << ")";
}

// 整数字面量
void PrettyPrinter::visitInt(Int* intLit, ostream& out) {
    out << "Int(" << intLit->getValue() << ")";
}

// 浮点数字面量
void PrettyPrinter::visitFloat(Float* floatLit, ostream& out) {
    out << "Float(" << floatLit->getValue() << ")";
}

// 类型
void PrettyPrinter::visitType(Type* type, ostream& out) {
    out << "Type(" << type->getName() << ")";
}
