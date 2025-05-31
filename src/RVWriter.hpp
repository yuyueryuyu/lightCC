#ifndef RVWRITER_HPP
#define RVWRITER_HPP

#include <string>
#include <vector>
#include "util/ir.hpp"
#include "util/riscv.hpp"
#include "util/registers.hpp"

class RVWriter {
private:
    BasicBlock* curBlock{nullptr};
    IRFunc* curFunc{nullptr};
    RegInt* readIntVal(IRVal* val, RegInt* dfl);
    RegFloat* readFloVal(IRVal* src, RegFloat* dfl, RegInt* tmp);
public:
    void visitProgram(IRProgram* prog);
    void visitGlobl(IRVar* var);
    void visitFunc(IRFunc* func);
    void visitBlock(BasicBlock* block);
    void visitInstr(IRInstr* instr);
    void visitAlloc(IRAlloc* alloc);
    void visitLoad(IRLoad* load);
    void visitStore(IRStore* store);
    void visitGetPtr(IRGetPtr* gptr);
    void visitGetElPtr(IRGetElPtr* gptr);
    void visitBinary(IRBinary* binary);
    void visitBr(IRBr* br);
    void visitJump(IRJump* jmp);
    void visitI2F(IRI2F* i2f);
    void visitF2I(IRF2I* f2i);
    void visitCall(IRCall* call);
    void visitRet(IRRet* ret);
};

#endif