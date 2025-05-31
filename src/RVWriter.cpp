#include "RVWriter.hpp"
#include <cstring>

RegInt* RVWriter::readIntVal(IRVal* src, RegInt* dfl) {
    if (auto src_int = dynamic_cast<IRInt*>(src)) {
        curBlock->add(new Li(*dfl, src_int->getValue()));
        return dfl;
    } else if (auto src_flo = dynamic_cast<IRFlo*>(src)) {
        int ival;
        float fval = src_flo->getValue();
        std::memcpy(&ival, &fval, sizeof(fval));
        curBlock->add(new Li(*dfl, ival));
        return dfl;
    } else if (auto src_sym = dynamic_cast<IRSym*>(src)) {
        auto src_st = src_sym->getStorage();
        if (auto src_it = dynamic_cast<RegStorage*>(src_st)) {
            auto src_reg = src_it->getReg();
            return dynamic_cast<RegInt*>(src_reg);
        } else if (auto src_it = dynamic_cast<StackStorage*>(src_st)) {
            int srcaddr = src_it->getOffset();
            curBlock->add(new Load(Load::Op::LW, *dfl, FP, srcaddr));
            return dfl;
        }
    }
}

RegFloat* RVWriter::readFloVal(IRVal* src, RegFloat* dfl, RegInt* tmp) {
    if (auto src_flo = dynamic_cast<IRFlo*>(src)) {
        int ival;
        float fval = src_flo->getValue();
        std::memcpy(&ival, &fval, sizeof(fval));
        curBlock->add(new Li(*tmp, ival));
        curBlock->add(new RegIntFloatMv(*dfl, *tmp));
        return dfl;
    } else if (auto src_sym = dynamic_cast<IRSym*>(src)) {
        auto src_st = src_sym->getStorage();
        if (auto src_it = dynamic_cast<RegStorage*>(src_st)) {
            auto src_reg = src_it->getReg();
            return dynamic_cast<RegFloat*>(src_reg);
        } else if (auto src_it = dynamic_cast<StackStorage*>(src_st)) {
            int srcaddr = src_it->getOffset();
            curBlock->add(new Flw(*dfl, FP, srcaddr));
            return dfl;
        }
    }
}

void RVWriter::visitInstr(IRInstr* instr) {
    if (!instr) return;
    
    if (auto it = dynamic_cast<IRAlloc*>(instr)) {
        visitAlloc(it);
    } else if (auto it = dynamic_cast<IRLoad*>(instr)) {
        visitLoad(it);
    } else if (auto it = dynamic_cast<IRStore*>(instr)) {
        visitStore(it);
    } else if (auto it = dynamic_cast<IRGetPtr*>(instr)) {
        visitGetPtr(it);
    } else if (auto it = dynamic_cast<IRGetElPtr*>(instr)) {
        visitGetElPtr(it);
    } else if (auto it = dynamic_cast<IRBinary*>(instr)) {
        visitBinary(it);
    } else if (auto it = dynamic_cast<IRBr*>(instr)) {
        visitBr(it);
    } else if (auto it = dynamic_cast<IRJump*>(instr)) {
        visitJump(it);
    } else if (auto it = dynamic_cast<IRI2F*>(instr)) {
        visitI2F(it);
    } else if (auto it = dynamic_cast<IRF2I*>(instr)) {
        visitF2I(it);
    } else if (auto it = dynamic_cast<IRCall*>(instr)) {
        visitCall(it);
    } else if (auto it = dynamic_cast<IRRet*>(instr)) {
        visitRet(it);
    }
}

void RVWriter::visitProgram(IRProgram* prog) {
    for (auto func : prog->getFunc()) {
        visitFunc(func);
    }
}

void RVWriter::visitGlobl(IRVar* var) {
    // pass
}

void RVWriter::visitFunc(IRFunc* func) {
    curFunc = func;
    func->addEntryInstr(new Imm(Imm::Op::ADDI, SP, SP, func->getSize()));
    func->addEntryInstr(new Store(Store::Op::SW, RA, SP, -func->getSize() - 4));
    func->addEntryInstr(new Store(Store::Op::SW, FP, SP, -func->getSize() - 8));
    func->addEntryInstr(new Imm(Imm::Op::ADDI, FP, SP, -func->getSize()));
    for (auto block : func->getBlocks()) {
        visitBlock(block);
    }
    func->addEpilogueInstr(new Load(Load::Op::LW, FP, SP, -func->getSize() - 8));
    func->addEpilogueInstr(new Load(Load::Op::LW, RA, SP, -func->getSize() - 4));
    func->addEpilogueInstr(new Imm(Imm::Op::ADDI, SP, SP, -func->getSize()));
}

void RVWriter::visitBlock(BasicBlock* block) {
    curBlock = block;
    for (auto instr : block->getInstrs()) {
        visitInstr(instr);
    }
    curBlock = nullptr;
}

void RVWriter::visitAlloc(IRAlloc* alloc) {
    auto dst = alloc->getDst();
    auto storage = dst->getStorage();
    if (auto regs = dynamic_cast<RegStorage*>(storage)) {
        auto reg = regs->getReg();
        auto ireg = dynamic_cast<RegInt*>(reg);
        curBlock->add(new Imm(Imm::Op::ADDI, *ireg, FP, alloc->getPosition()));
    } else if (auto mems = dynamic_cast<StackStorage*>(storage)) {
        int addr = mems->getOffset();
        curBlock->add(new Imm(Imm::Op::ADDI, T6, FP, alloc->getPosition()));
        curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
    }
}

void RVWriter::visitLoad(IRLoad* load) {
    auto sym = load->getSym();
    auto sym_storage = sym->getStorage();
    auto dst = load->getDst();
    auto dst_storage = dst->getStorage();
    // 源寄存器在寄存器里
    if (auto sym_regs = dynamic_cast<RegStorage*>(sym_storage)) {
        auto sym_reg = sym_regs->getReg();
        // load 的sym的类型一定为指针，所以一定存在整数寄存器
        auto sym_ireg = dynamic_cast<RegInt*>(sym_reg);
        // 目标寄存器在寄存器里
        if (auto dst_regs = dynamic_cast<RegStorage*>(dst_storage)) {
            auto dst_reg = dst_regs->getReg();
            // 目标寄存器是整数寄存器
            if (auto dst_ireg = dynamic_cast<RegInt*>(dst_reg)) {
                curBlock->add(new Load(Load::Op::LW, *dst_ireg, *sym_ireg, 0));
            }
            // 目标寄存器是浮点寄存器 
            else if (auto dst_freg = dynamic_cast<RegFloat*>(dst_reg)) {
                curBlock->add(new Flw(*dst_freg, *sym_ireg, 0));
            } 

        } 
        // 目标寄存器被spill到栈上
        else if (auto dst_mems = dynamic_cast<StackStorage*>(dst_storage)) {
            int addr = dst_mems->getOffset();
            curBlock->add(new Load(Load::Op::LW, T6, *sym_ireg, 0));
            curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
        }

    } 
    // 源寄存器在栈上
    else if (auto sym_mems = dynamic_cast<StackStorage*>(sym_storage)) {
        int sym_addr = sym_mems->getOffset();
        // 目标寄存器在寄存器里
        if (auto dst_regs = dynamic_cast<RegStorage*>(dst_storage)) {
            auto dst_reg = dst_regs->getReg();
            // 目标寄存器是整数寄存器
            if (auto dst_ireg = dynamic_cast<RegInt*>(dst_reg)) {
                curBlock->add(new Load(Load::Op::LW, *dst_ireg, FP, sym_addr));
            }
            // 目标寄存器是浮点寄存器 
            else if (auto dst_freg = dynamic_cast<RegFloat*>(dst_reg)) {
                curBlock->add(new Flw(*dst_freg, FP, sym_addr));
            } 

        } 
        // 目标寄存器被spill到栈上
        else if (auto dst_mems = dynamic_cast<StackStorage*>(dst_storage)) {
            int addr = dst_mems->getOffset();
            curBlock->add(new Load(Load::Op::LW, T6, FP, sym_addr));
            curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
        }
    }
    // sym是一个全局变量
    else if (dynamic_cast<StaticStorage*>(sym_storage)) {
        curBlock->add(new La(A0, Label(sym->getName())));
        // 目标寄存器在寄存器里
        if (auto dst_regs = dynamic_cast<RegStorage*>(dst_storage)) {
            auto dst_reg = dst_regs->getReg();
            // 目标寄存器是整数寄存器
            if (auto dst_ireg = dynamic_cast<RegInt*>(dst_reg)) {
                curBlock->add(new Load(Load::Op::LW, *dst_ireg, A0, 0));
            }
            // 目标寄存器是浮点寄存器 
            else if (auto dst_freg = dynamic_cast<RegFloat*>(dst_reg)) {
                curBlock->add(new Flw(*dst_freg, A0, 0));
            } 

        } 
        // 目标寄存器被spill到栈上
        else if (auto dst_mems = dynamic_cast<StackStorage*>(dst_storage)) {
            int addr = dst_mems->getOffset();
            curBlock->add(new Load(Load::Op::LW, T6, A0, 0));
            curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
        }
    }
}

void RVWriter::visitStore(IRStore* store) {
    auto sym = store->getSym();
    auto sym_storage = sym->getStorage();
    auto src = store->getSrc();
    if (auto src_int = dynamic_cast<IRInt*>(src)) {
        curBlock->add(new Li(T6, src_int->getValue()));
        if (auto it = dynamic_cast<RegStorage*>(sym_storage)) {
            auto reg = it->getReg();
            auto ireg = dynamic_cast<RegInt*>(reg);
            curBlock->add(new Store(Store::Op::SW, T6, *ireg, 0));
        } else if (auto it = dynamic_cast<StackStorage*>(sym_storage)) {
            int addr = it->getOffset();
            curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
        } else if (dynamic_cast<StaticStorage*>(sym_storage)) {
            curBlock->add(new SwGlobal(T6, Label(sym->getName()), A0));
        }
    } else if (auto src_flo = dynamic_cast<IRFlo*>(src)) {
        int ival;
        float fval = src_flo->getValue();
        std::memcpy(&ival, &fval, sizeof(fval));
        curBlock->add(new Li(T6, ival));
        if (auto it = dynamic_cast<RegStorage*>(sym_storage)) {
            auto reg = it->getReg();
            auto ireg = dynamic_cast<RegInt*>(reg);
            curBlock->add(new Store(Store::Op::SW, T6, *ireg, 0));
        } else if (auto it = dynamic_cast<StackStorage*>(sym_storage)) {
            int addr = it->getOffset();
            curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
        } else if (dynamic_cast<StaticStorage*>(sym_storage)) {
            curBlock->add(new SwGlobal(T6, Label(sym->getName()), A0));
        }
    } else if (auto src_sym = dynamic_cast<IRSym*>(src)) {
        auto src_st = src_sym->getStorage();
        if (auto src_it = dynamic_cast<RegStorage*>(src_st)) {
            auto src_reg = src_it->getReg();
            if (auto sireg = dynamic_cast<RegInt*>(src_reg)) {
                if (auto it = dynamic_cast<RegStorage*>(sym_storage)) {
                    auto reg = it->getReg();
                    auto ireg = dynamic_cast<RegInt*>(reg);
                    curBlock->add(new Store(Store::Op::SW, *sireg, *ireg, 0));
                } else if (auto it = dynamic_cast<StackStorage*>(sym_storage)) {
                    int addr = it->getOffset();
                    curBlock->add(new Store(Store::Op::SW, *sireg, FP, addr));
                } else if (dynamic_cast<StaticStorage*>(sym_storage)) {
                    curBlock->add(new SwGlobal(*sireg, Label(sym->getName()), A0));
                }
            } else if (auto sfreg = dynamic_cast<RegFloat*>(src_reg)) {
                if (auto it = dynamic_cast<RegStorage*>(sym_storage)) {
                    auto reg = it->getReg();
                    auto ireg = dynamic_cast<RegInt*>(reg);
                    curBlock->add(new Fsw(*sfreg, *ireg, 0));
                } else if (auto it = dynamic_cast<StackStorage*>(sym_storage)) {
                    int addr = it->getOffset();
                    curBlock->add(new Fsw(*sfreg, FP, addr));
                } else if (dynamic_cast<StaticStorage*>(sym_storage)) {
                    curBlock->add(new FswGlobal(*sfreg, Label(sym->getName()), A0));
                }
            }
        } else if (auto src_it = dynamic_cast<StackStorage*>(src_st)) {
            int srcaddr = src_it->getOffset();
            curBlock->add(new Load(Load::Op::LW, T6, FP, srcaddr));
            if (auto it = dynamic_cast<RegStorage*>(sym_storage)) {
                auto reg = it->getReg();
                auto ireg = dynamic_cast<RegInt*>(reg);
                curBlock->add(new Store(Store::Op::SW, T6, *ireg, 0));
            } else if (auto it = dynamic_cast<StackStorage*>(sym_storage)) {
                int addr = it->getOffset();
                curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
            } else if (dynamic_cast<StaticStorage*>(sym_storage)) {
                curBlock->add(new SwGlobal(T6, Label(sym->getName()), A0));
            }
        }
    }
}

void RVWriter::visitGetPtr(IRGetPtr* gptr) {
    // pass, IR里不会生成这种指令
}

void RVWriter::visitGetElPtr(IRGetElPtr* gptr) {
    auto dst = gptr->getDst();
    auto sym = gptr->getSym();
    auto offset = gptr->getOffset();
    auto off_reg = readIntVal(offset, &T6);
    curBlock->add(new Imm(Imm::Op::SLLI, *off_reg, *off_reg, 2));
    auto sym_reg = readIntVal(sym, &A0);
    auto dst_str = dst->getStorage();
    if (auto it = dynamic_cast<RegStorage*>(dst_str)) {
        auto reg = dynamic_cast<RegInt*>(it->getReg());
        curBlock->add(new Reg(Reg::Op::ADD, *reg, *sym_reg, *off_reg));
    } else if (auto it = dynamic_cast<StackStorage*>(dst_str)) {
        int addr = it->getOffset();
        curBlock->add(new Reg(Reg::Op::ADD, A0, *sym_reg, *off_reg));
        curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
    }
}

void RVWriter::visitBinary(IRBinary* binary) {
    auto lhs = binary->getSrc1();
    auto rhs = binary->getSrc2();
    auto dst = binary->getDst();
    auto dst_st = dst->getStorage();
    if (lhs->getType()->equals(&FLOAT_TYPE)) {
        auto lhs_reg = readFloVal(lhs, &FA0, &T6);
        auto  rhs_reg = readFloVal(rhs, &FA1, &T6);
        switch (binary->getOp()) {
            case '+':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegFloat*>(it->getReg());
                    curBlock->add(new FBinary(FBinary::Op::FADD, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new FBinary(FBinary::Op::FADD, FA0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Fsw(FA0, FP, addr));
                }
                break;
            case '*':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegFloat*>(it->getReg());
                    curBlock->add(new FBinary(FBinary::Op::FMUL, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new FBinary(FBinary::Op::FMUL, FA0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Fsw(FA0, FP, addr));
                }
                break;
            case '<':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new FComp(FComp::Op::FLT, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new FComp(FComp::Op::FLT, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
                }
                break;
            case 'l':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new FComp(FComp::Op::FLE, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new FComp(FComp::Op::FLE, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
                }
                break;
            case '=':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new FComp(FComp::Op::FEQ, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new FComp(FComp::Op::FEQ, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
                }
                break;
            default: break;
        }
    } else if (lhs->getType()->equals(&INT_TYPE)) {
        auto lhs_reg = readIntVal(lhs, &A0);
        auto  rhs_reg = readIntVal(rhs, &A1);
        switch (binary->getOp()) {
            case '+':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new Reg(Reg::Op::ADD, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new Reg(Reg::Op::ADD, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
                }
                break;
            case '*':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new Reg(Reg::Op::MUL, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new Reg(Reg::Op::MUL, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
                }
                break;
            case '<':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new Reg(Reg::Op::SLT, *freg, *lhs_reg, *rhs_reg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new Reg(Reg::Op::SLT, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
                }
                break;
            case 'l':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new Reg(Reg::Op::SLT, *freg, *lhs_reg, *rhs_reg));
                    curBlock->add(new Reg(Reg::Op::XOR, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new RegZ(RegZ::Op::SEQZ, A0, A0));
                    curBlock->add(new Reg(Reg::Op::OR, *freg, *freg, A0));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new Reg(Reg::Op::SLT, T6, *lhs_reg, *rhs_reg));
                    curBlock->add(new Reg(Reg::Op::XOR, A0, *lhs_reg, *rhs_reg));
                    curBlock->add(new RegZ(RegZ::Op::SEQZ, A0, A0));
                    curBlock->add(new Reg(Reg::Op::OR, T6, T6, A0));
                    curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
                }
                break;
            case '=':
                if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
                    auto freg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new Reg(Reg::Op::XOR, *freg, *lhs_reg, *rhs_reg));
                    curBlock->add(new RegZ(RegZ::Op::SEQZ, *freg, *freg));
                } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
                    int addr = it->getOffset();
                    curBlock->add(new Reg(Reg::Op::XOR, T6, *lhs_reg, *rhs_reg));
                    curBlock->add(new RegZ(RegZ::Op::SEQZ, T6, T6));
                    curBlock->add(new Store(Store::Op::SW, T6, FP, addr));
                }
                break;
            default: break;
        }
    }
}

void RVWriter::visitBr(IRBr* br) {
    auto val = br->getVal();
    auto val_reg = readIntVal(val, &T6);
    curBlock->add(new BranchZ(BranchZ::Op::BNEZ, *val_reg, Label(br->getThenLabel()->getName())));
    curBlock->add(new BranchZ(BranchZ::Op::BEQZ, *val_reg, Label(br->getElseLabel()->getName())));
}

void RVWriter::visitJump(IRJump* jmp) {
    curBlock->add(new J(Label(jmp->getLabel()->getName())));
}

void RVWriter::visitI2F(IRI2F* i2f) {
    auto src = i2f->getSrc();
    auto dst = i2f->getDst();
    auto dst_st = dst->getStorage();
    auto src_reg = readIntVal(src, &T6);
    if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
        auto dst_reg = dynamic_cast<RegFloat*>(it->getReg());
        curBlock->add(new RegIntCvt(RegIntCvt::Op::FCVT_S_W, *dst_reg, *src_reg));
    } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
        int addr = it->getOffset();
        curBlock->add(new RegIntCvt(RegIntCvt::Op::FCVT_S_W, FA0, *src_reg));
        curBlock->add(new Fsw(FA0, FP, addr));
    }
}

void RVWriter::visitF2I(IRF2I* f2i) {
    auto src = f2i->getSrc();
    auto dst = f2i->getDst();
    auto dst_st = dst->getStorage();
    auto src_reg = readFloVal(src, &FA0, &T6);
    if (auto it = dynamic_cast<RegStorage*>(dst_st)) {
        auto dst_reg = dynamic_cast<RegInt*>(it->getReg());
        curBlock->add(new FloatCvt(FloatCvt::Op::FCVT_W_S, *dst_reg, *src_reg));
    } else if (auto it = dynamic_cast<StackStorage*>(dst_st)) {
        int addr = it->getOffset();
        curBlock->add(new FloatCvt(FloatCvt::Op::FCVT_W_S, A0, *src_reg));
        curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
    }
}
void RVWriter::visitCall(IRCall* call) {
    auto func = call->getResolution();
    if (func == nullptr) {
        if (call->getArgs().size() == 1) {
            auto arg = call->getArgs()[0];
            if (arg->getType()->equals(&FLOAT_TYPE)) {
                auto ireg = readFloVal(arg, &FA0, &T6);
                curBlock->add(new FUnary(FUnary::Op::FMV, FA0, *ireg));
            } else {
                auto ireg = readIntVal(arg, &A0);
                curBlock->add(new RegZ(RegZ::Op::MV, A0, *ireg));
            }
        }
        auto sym = call->getFunc();
        auto st = sym->getStorage();
        if (auto it = dynamic_cast<RegStorage*>(st)) {
            auto reg = dynamic_cast<RegInt*>(it);
            curBlock->add(new Jalr(RA, *reg, 0));
        } else if (auto it = dynamic_cast<StackStorage*>(st)) {
            int addr = it->getOffset();
            curBlock->add(new Load(Load::Op::LW, T6, FP, addr));
            curBlock->add(new Jalr(RA, T6, 0));
        }
    } else {
        for (size_t i = 0; i < call->getArgs().size(); i++) {
            auto param = func->getParams()[i];
            auto pstore = param->getStorage();
            auto arg = call->getArgs()[i];
            if (arg->getType()->equals(&FLOAT_TYPE)) {
                auto val = readFloVal(arg, &FT11, &T6);
                if (auto it = dynamic_cast<RegStorage*>(pstore)) {
                    auto reg = dynamic_cast<RegFloat*>(it->getReg());
                    curBlock->add(new FUnary(FUnary::Op::FMV, *reg, *val));
                } else if (auto it = dynamic_cast<StackStorage*>(pstore)) {
                    int addr = it->getOffset();
                    curBlock->add(new Fsw(*val, SP, addr));
                }
            } else {
                auto val = readIntVal(arg, &T6);
                if (auto it = dynamic_cast<RegStorage*>(pstore)) {
                    auto reg = dynamic_cast<RegInt*>(it->getReg());
                    curBlock->add(new RegZ(RegZ::Op::MV, *reg, *val));
                } else if (auto it = dynamic_cast<StackStorage*>(pstore)) {
                    int addr = it->getOffset();
                    curBlock->add(new Store(Store::Op::SW, *val, SP, addr));
                }
            }
        }
        curBlock->add(new Jal(RA, Label(call->getFunc()->getName())));
    }
    auto res = call->getResult();
    if (res != nullptr) {
        auto resst = res->getStorage();
        if (auto it = dynamic_cast<RegStorage*>(resst)) {
            if (res->getType()->equals(&FLOAT_TYPE)) {
                auto freg = dynamic_cast<RegFloat*>(it->getReg());
                curBlock->add(new FUnary(FUnary::Op::FMV, *freg, FA0));
            } else {
                auto freg = dynamic_cast<RegInt*>(it->getReg());
                curBlock->add(new RegZ(RegZ::Op::MV, *freg, A0));
            }
        } else if (auto it = dynamic_cast<StackStorage*>(resst)) {
            int addr = it->getOffset();
            if (res->getType()->equals(&FLOAT_TYPE)) {
                curBlock->add(new Fsw(FA0, FP, addr));
            } else {
                curBlock->add(new Store(Store::Op::SW, A0, FP, addr));
            }
        }
    }
}

void RVWriter::visitRet(IRRet* ret) {
    if (ret->getVal() == nullptr) {
        curBlock->add(new RegZ(RegZ::Op::MV, A0, ZERO));
    } else {
        auto val = ret->getVal();
        if (val->getType()->equals(&FLOAT_TYPE)) {
            auto val_reg = readFloVal(val, &FA0, &T6);
            curBlock->add(new FUnary(FUnary::Op::FMV, FA0, *val_reg));
        } else if (val->getType()->equals(&INT_TYPE)) {
            auto val_reg = readIntVal(val, &A0);
            curBlock->add(new RegZ(RegZ::Op::MV, A0, *val_reg));
        }
    }
    curBlock->add(new J(Label(curFunc->getEpilogueLabel()->getName())));
}