#ifndef RISCV_HPP
#define RISCV_HPP

#include <string>
#include <vector>
#include <sstream>
#include "registers.hpp"

class Label {
private:
    std::string name;
public:
    Label(const std::string& n) : name(n) {}
    std::string toString() const { return name; }
};


// Base instruction interface
class Instr {
public:
    virtual ~Instr() = default;
    virtual std::string toString() const = 0;
};

// Register-Register operations
class Reg : public Instr {
public:
    enum class Op {
        ADD, SUB, AND, OR, XOR,
        SLL, SRL, SRA, SLT, SLTU,
        MUL, MULH, MULHU, MULHSU,
        DIV, DIVU, REM, REMU
    };
    
    Op op;
    RegInt rd, rs1, rs2;
    
    Reg(Op o, RegInt d, RegInt s1, RegInt s2) 
        : op(o), rd(d), rs1(s1), rs2(s2) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString() << ", " << rs2.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::ADD: return "add";
            case Op::SUB: return "sub";
            case Op::AND: return "and";
            case Op::OR: return "or";
            case Op::XOR: return "xor";
            case Op::SLL: return "sll";
            case Op::SRL: return "srl";
            case Op::SRA: return "sra";
            case Op::SLT: return "slt";
            case Op::SLTU: return "sltu";
            case Op::MUL: return "mul";
            case Op::MULH: return "mulh";
            case Op::MULHU: return "mulhu";
            case Op::MULHSU: return "mulhsu";
            case Op::DIV: return "div";
            case Op::DIVU: return "divu";
            case Op::REM: return "rem";
            case Op::REMU: return "remu";
        }
        return "";
    }
};

// Register-Zero operations (pseudo instructions)
class RegZ : public Instr {
public:
    enum class Op {
        MV, SEQZ, SNEZ, SLTZ, SGTZ, NEG
    };
    
    Op op;
    RegInt rd, rs1;
    
    RegZ(Op o, RegInt d, RegInt s1) : op(o), rd(d), rs1(s1) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::MV: return "mv";
            case Op::SEQZ: return "seqz";
            case Op::SNEZ: return "snez";
            case Op::SLTZ: return "sltz";
            case Op::SGTZ: return "sgtz";
            case Op::NEG: return "neg";
        }
        return "";
    }
};

// Floating-point unary operations
class FUnary : public Instr {
public:
    enum class Op {
        FMV, FNEG, FABS, FSQRT
    };
    
    Op op;
    RegFloat rd, rs1;
    
    FUnary(Op o, RegFloat d, RegFloat s1) : op(o), rd(d), rs1(s1) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::FMV: return "fmv.s";
            case Op::FNEG: return "fneg.s";
            case Op::FABS: return "fabs.s";
            case Op::FSQRT: return "fsqrt.s";
        }
        return "";
    }
};

// Floating-point classification
class Fclass : public Instr {
public:
    RegInt rd;
    RegFloat rs;
    
    Fclass(RegInt d, RegFloat s) : rd(d), rs(s) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "fclass.s " << rd.toString() << ", " << rs.toString();
        return oss.str();
    }
};

// Floating-point binary operations
class FBinary : public Instr {
public:
    enum class Op {
        FADD, FSUB, FMUL, FDIV, FMIN, FMAX,
        FSGNJ, FSGNJN, FSGNJX
    };
    
    Op op;
    RegFloat rd, rs1, rs2;
    
    FBinary(Op o, RegFloat d, RegFloat s1, RegFloat s2) 
        : op(o), rd(d), rs1(s1), rs2(s2) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString() << ", " << rs2.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::FADD: return "fadd.s";
            case Op::FSUB: return "fsub.s";
            case Op::FMUL: return "fmul.s";
            case Op::FDIV: return "fdiv.s";
            case Op::FMIN: return "fmin.s";
            case Op::FMAX: return "fmax.s";
            case Op::FSGNJ: return "fsgnj.s";
            case Op::FSGNJN: return "fsgnjn.s";
            case Op::FSGNJX: return "fsgnjx.s";
        }
        return "";
    }
};

// Floating-point comparison
class FComp : public Instr {
public:
    enum class Op {
        FEQ, FLT, FLE
    };
    
    Op op;
    RegInt rd;
    RegFloat rs1, rs2;
    
    FComp(Op o, RegInt d, RegFloat s1, RegFloat s2) 
        : op(o), rd(d), rs1(s1), rs2(s2) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString() << ", " << rs2.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::FEQ: return "feq.s";
            case Op::FLT: return "flt.s";
            case Op::FLE: return "fle.s";
        }
        return "";
    }
};

// Floating-point multiply-add
class FMulAdd : public Instr {
public:
    enum class Op {
        FMADD, FMSUB, FNMSUB, FNMADD
    };
    
    Op op;
    RegFloat rd, rs1, rs2, rs3;
    
    FMulAdd(Op o, RegFloat d, RegFloat s1, RegFloat s2, RegFloat s3) 
        : op(o), rd(d), rs1(s1), rs2(s2), rs3(s3) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString() << ", " << rs2.toString() << ", " << rs3.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::FMADD: return "fmadd.s";
            case Op::FMSUB: return "fmsub.s";
            case Op::FNMSUB: return "fnmsub.s";
            case Op::FNMADD: return "fnmadd.s";
        }
        return "";
    }
};

// RegFloat to integer conversion
class FloatCvt : public Instr {
public:
    enum class Op {
        FCVT_W_S, FCVT_WU_S
    };
    
    Op op;
    RegInt rd;
    RegFloat rs;
    
    FloatCvt(Op o, RegInt d, RegFloat s) : op(o), rd(d), rs(s) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs.toString() << ", rtz";
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::FCVT_W_S: return "fcvt.w.s";
            case Op::FCVT_WU_S: return "fcvt.wu.s";
        }
        return "";
    }
};

// RegFloat to integer move
class FloatRegIntMv : public Instr {
public:
    RegInt rd;
    RegFloat rs;
    
    FloatRegIntMv(RegInt d, RegFloat s) : rd(d), rs(s) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "fmv.x.w " << rd.toString() << ", " << rs.toString();
        return oss.str();
    }
};

// RegInteger to float conversion
class RegIntCvt : public Instr {
public:
    enum class Op {
        FCVT_S_W, FCVT_S_WU
    };
    
    Op op;
    RegFloat rd;
    RegInt rs;
    
    RegIntCvt(Op o, RegFloat d, RegInt s) : op(o), rd(d), rs(s) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::FCVT_S_W: return "fcvt.s.w";
            case Op::FCVT_S_WU: return "fcvt.s.wu";
        }
        return "";
    }
};

// RegInteger to float move
class RegIntFloatMv : public Instr {
public:
    RegFloat rd;
    RegInt rs;
    
    RegIntFloatMv(RegFloat d, RegInt s) : rd(d), rs(s) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "fmv.w.x " << rd.toString() << ", " << rs.toString();
        return oss.str();
    }
};

// Immediate operations
class Imm : public Instr {
public:
    enum class Op {
        ADDI, ANDI, ORI, XORI,
        SLLI, SRLI, SRAI, SLTI, SLTIU
    };
    
    Op op;
    RegInt rd, rs1;
    int imm;
    
    Imm(Op o, RegInt d, RegInt s1, int i) : op(o), rd(d), rs1(s1), imm(i) {
        switch (op) {
            case Op::SLLI:
            case Op::SRLI:
            case Op::SRAI:
                assert(0 <= imm && imm < 32);
                break;
            default:
                assert(-2048 <= imm && imm < 2048);
                break;
        }
    }
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << rs1.toString() << ", " << imm;
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::ADDI: return "addi";
            case Op::ANDI: return "andi";
            case Op::ORI: return "ori";
            case Op::XORI: return "xori";
            case Op::SLLI: return "slli";
            case Op::SRLI: return "srli";
            case Op::SRAI: return "srai";
            case Op::SLTI: return "slti";
            case Op::SLTIU: return "sltiu";
        }
        return "";
    }
};

// Load operations
class Load : public Instr {
public:
    enum class Op {
        LB, LBU, LH, LHU, LW
    };
    
    Op op;
    RegInt rd, rs1;
    int imm;
    
    Load(Op o, RegInt d, RegInt s1, int i) : op(o), rd(d), rs1(s1), imm(i) {
        assert(-2048 <= imm && imm < 2048);
    }
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rd.toString() << ", " << imm << "(" << rs1.toString() << ")";
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::LB: return "lb";
            case Op::LBU: return "lbu";
            case Op::LH: return "lh";
            case Op::LHU: return "lhu";
            case Op::LW: return "lw";
        }
        return "";
    }
};

// Floating-point load
class Flw : public Instr {
public:
    RegFloat rd;
    RegInt rs1;
    int imm;
    
    Flw(RegFloat d, RegInt s1, int i) : rd(d), rs1(s1), imm(i) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "flw " << rd.toString() << ", " << imm << "(" << rs1.toString() << ")";
        return oss.str();
    }
};

// Store operations
class Store : public Instr {
public:
    enum class Op {
        SB, SH, SW
    };
    
    Op op;
    RegInt rs2, rs1;
    int imm;
    
    Store(Op o, RegInt s2, RegInt s1, int i) : op(o), rs2(s2), rs1(s1), imm(i) {
        assert(-2048 <= imm && imm < 2048);
    }
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rs2.toString() << ", " << imm << "(" << rs1.toString() << ")";
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::SB: return "sb";
            case Op::SH: return "sh";
            case Op::SW: return "sw";
        }
        return "";
    }
};

// Floating-point store
class Fsw : public Instr {
public:
    RegFloat rd;
    RegInt rs1;
    int imm;
    
    Fsw(RegFloat d, RegInt s1, int i) : rd(d), rs1(s1), imm(i) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "fsw " << rd.toString() << ", " << imm << "(" << rs1.toString() << ")";
        return oss.str();
    }
};

// Global store operations
class SwGlobal : public Instr {
public:
    RegInt rd;
    Label label;
    RegInt tmp;
    
    SwGlobal(RegInt d, Label l, RegInt t) : rd(d), label(l), tmp(t) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "sw " << rd.toString() << ", " << label.toString() << ", " << tmp.toString();
        return oss.str();
    }
};

class FswGlobal : public Instr {
public:
    RegFloat rd;
    Label label;
    RegInt tmp;
    
    FswGlobal(RegFloat d, Label l, RegInt t) : rd(d), label(l), tmp(t) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "fsw " << rd.toString() << ", " << label.toString() << ", " << tmp.toString();
        return oss.str();
    }
};

// Branch operations
class Branch : public Instr {
public:
    enum class Op {
        BEQ, BGE, BGEU, BLT, BLTU, BNE,
        BGTU, BLE, BLEU, BGT
    };
    
    Op op;
    RegInt rs1, rs2;
    Label label;
    
    Branch(Op o, RegInt s1, RegInt s2, Label l) : op(o), rs1(s1), rs2(s2), label(l) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rs1.toString() << ", " << rs2.toString() << ", " << label.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::BEQ: return "beq";
            case Op::BGE: return "bge";
            case Op::BGEU: return "bgeu";
            case Op::BLT: return "blt";
            case Op::BLTU: return "bltu";
            case Op::BNE: return "bne";
            case Op::BGTU: return "bgtu";
            case Op::BLE: return "ble";
            case Op::BLEU: return "bleu";
            case Op::BGT: return "bgt";
        }
        return "";
    }
};

// Branch with zero
class BranchZ : public Instr {
public:
    enum class Op {
        BEQZ, BGEZ, BGTZ, BLTZ, BLEZ, BNEZ
    };
    
    Op op;
    RegInt rs1;
    Label label;
    
    BranchZ(Op o, RegInt s1, Label l) : op(o), rs1(s1), label(l) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << opToString() << " " << rs1.toString() << ", " << label.toString();
        return oss.str();
    }
    
private:
    std::string opToString() const {
        switch (op) {
            case Op::BEQZ: return "beqz";
            case Op::BGEZ: return "bgez";
            case Op::BGTZ: return "bgtz";
            case Op::BLTZ: return "bltz";
            case Op::BLEZ: return "blez";
            case Op::BNEZ: return "bnez";
        }
        return "";
    }
};

// Jump and link
class Jal : public Instr {
public:
    RegInt rd;
    Label label;
    
    Jal(RegInt d, Label l) : rd(d), label(l) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "jal " << rd.toString() << " " << label.toString();
        return oss.str();
    }
};

// Jump and link register
class Jalr : public Instr {
public:
    RegInt rd, rs1;
    int imm;
    
    Jalr(RegInt d, RegInt s1, int i) : rd(d), rs1(s1), imm(i) {
        assert(-2048 <= imm && imm < 2048);
    }
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "jalr " << rd.toString() << " " << imm << "(" << rs1.toString() << ")";
        return oss.str();
    }
};

// Local label
class LocalLabel : public Instr {
public:
    Label label;
    
    LocalLabel(Label l) : label(l) {}
    
    std::string toString() const override {
        return label.toString();
    }
};

// Upper immediate instructions
class Auipc : public Instr {
public:
    RegInt rd;
    int immu;
    
    Auipc(RegInt d, int i) : rd(d), immu(i) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "auipc " << rd.toString() << ", " << immu;
        return oss.str();
    }
};

class Lui : public Instr {
public:
    RegInt rd;
    int immu;
    
    Lui(RegInt d, int i) : rd(d), immu(i) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "lui " << rd.toString() << ", " << immu;
        return oss.str();
    }
};

// System call
class Ecall : public Instr {
public:
    std::string toString() const override {
        return "ecall";
    }
};

// Load immediate
class Li : public Instr {
public:
    RegInt rd;
    int imm;
    
    Li(RegInt d, int i) : rd(d), imm(i) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "li " << rd.toString() << ", " << imm;
        return oss.str();
    }
};

// Load immediate global
class LiGlobl : public Instr {
public:
    RegInt rd;
    Label imm;
    
    LiGlobl(RegInt d, Label i) : rd(d), imm(i) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "li " << rd.toString() << ", $" << imm.toString();
        return oss.str();
    }
};

// Load address
class La : public Instr {
public:
    RegInt rd;
    Label label;
    
    La(RegInt d, Label l) : rd(d), label(l) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "la " << rd.toString() << ", " << label.toString();
        return oss.str();
    }
};

// Jump
class J : public Instr {
public:
    Label label;
    
    J(Label l) : label(l) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "j " << label.toString();
        return oss.str();
    }
};

// Jump register
class Jr : public Instr {
public:
    RegInt rs;
    
    Jr(RegInt s) : rs(s) {}
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "jr " << rs.toString();
        return oss.str();
    }
};

// Return
class Ret : public Instr {
public:
    std::string toString() const override {
        return "ret";
    }
};

#endif