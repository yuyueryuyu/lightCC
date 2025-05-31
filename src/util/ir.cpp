#include "ir.hpp"
#include <assert.h>
#include "riscv.hpp"

// 实现先前声明的成员函数

void BasicBlock::addInstr(IRInstr* instr) {
    if (instr->isTerminator()) {
        setEndInstr(instr);
    } else {
        instrs.push_back(instr);
    }
}

void BasicBlock::setEndInstr(IRInstr* instr) {
    assert(instr->isTerminator() && "End instruction must be a terminator");
    endInstr = std::move(instr);
}

void BasicBlock::print(std::ostream& os) const {
    // 打印标签和参数
    os << label->toString() << "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) os << ", ";
        os << params[i]->toString();
    }
    os << "):\n";
    
    // 打印指令
    for (const auto& instr : instrs) {
        os << "  " << instr->toString() << "\n";
    }
    
    // 打印终结指令
    if (endInstr) {
        os << "  " << endInstr->toString() << "\n";
    }
}

std::string BasicBlock::toRiscV() const {
    std::ostringstream oss;
    oss << label->getName() << ":" << std::endl;
    for (auto instr : rv_instrs) {
        oss << "  " << instr->toString() << std::endl;
    }
    return oss.str();
}

void IRFunc::addBlock(BasicBlock* block) {
    blockMap[block->getLabel()->getName()] = block;
    blocks.push_back(std::move(block));
}

BasicBlock* IRFunc::getBlock(const std::string& name) {
    auto it = blockMap.find(name);
    if (it != blockMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::string IRFunc::toRiscV() const {
    std::ostringstream oss;
    oss << "  .align 1" << std::endl;
    oss << "  .globl" << std::endl;
    oss << "  .text" << std::endl;
    oss << "  .type " << sym->getName() <<", @function" << std::endl;
    oss << sym->getName() << ":" << std::endl;
    for (auto instr : entry) {
        oss << "  " << instr->toString() << std::endl;
    }
    for (auto block : blocks) {
        oss << block->toRiscV();
    }
    oss << epilogueLabel->getName() << ":" << std::endl;
    for (auto instr : epilogue) {
        oss << "  " << instr->toString() << std::endl;
    }
    oss << "  .size " << sym->getName() << ", .-" << sym->getName() << std::endl;
    return oss.str();
}

void IRFunc::print(std::ostream& os) const {
    // 打印函数签名
    os << toString() << " {\n";
    
    // 打印基本块
    for (const auto& block : blocks) {
        block->print(os);
        os << "\n";
    }
    
    os << "}\n";
}

void IRProgram::print(std::ostream& os) const {
    
    // 打印全局变量
    for (const auto& global : globls) {
        os << global->toString() << ";\n";
    }
    
    if (!globls.empty()) {
        os << "\n";
    }
    
    // 打印函数
    for (const auto& func : funcs) {
        func->print(os);
        os << "\n";
    }
}