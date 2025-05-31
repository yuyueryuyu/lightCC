#ifndef IR_HPP
#define IR_HPP
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <set>
#include <sstream>
#include "type.hpp"
#include "symboltable.hpp"
#include "symbol.hpp"
#include "storage.hpp"

// 前向声明
class BasicBlock;
class IRInstr;
class Instr;

/**
 * IR值的基类，所有被操作的值都继承自此类
 */
class IRVal {
protected:
    IType* type;

public:
    explicit IRVal(IType* type) : type(type) {}
    virtual ~IRVal() = default;

    IType* getType() const { return type; }
    virtual std::string toString() const = 0;
};

/**
 * 符号值，代表一个命名的变量或者标签
 */
class IRSym : public IRVal {
private:
    std::string name;
    Storage* storage {nullptr};
public:
    IRSym(IType* type, const std::string& name) : IRVal(type), name(name) {}
    
    const std::string& getName() const { return name; }
    
    std::string toString() const override {
        return name + ":" + (type ? type->toString() : "void") + (storage == nullptr ? "" : storage->toString());
    }
    
    bool operator==(const IRSym* other) const {
        return name == other->name;
    }
    
    bool operator!=(const IRSym* other) const {
        return !(*this == other);
    }

    void setStorage(Storage* s) { storage = s; }
    Storage* getStorage() { return storage; }
};

/**
 * 整数常量值
 */
class IRInt : public IRVal {
private:
    int value;

public:
    IRInt(int value) : IRVal(&INT_TYPE), value(value) {}
    
    int getValue() const { return value; }
    
    std::string toString() const override {
        return std::to_string(value);
    }
};

/**
 * 浮点数常量值
 */
class IRFlo : public IRVal {
private:
    float value;

public:
    IRFlo(float value) : IRVal(&FLOAT_TYPE), value(value) {}
    
    float getValue() const { return value; }
    
    std::string toString() const override {
        return std::to_string(value);
    }
};

/**
 * 定义的基类，包括全局变量和函数
 */
class IRDef {
protected:
    IRSym* sym;

public:
    explicit IRDef(IRSym* sym) : sym(sym) {}
    virtual ~IRDef() = default;
    
    IRSym* getSym() const { return sym; }
    virtual std::string toString() const = 0;
};

/**
 * 全局变量定义
 */
class IRVar : public IRDef {
public:
    IRVar(IRSym* sym);

    IRVar(Var* var) : IRDef(new IRSym(new PType(var->getType()), "@"+var->getName())) {}
    
    std::string toString() const override {
        std::string result = "global " + sym->toString();
        return result;
    }

    std::string toRiscV() const {
        std::ostringstream oss;
        oss << "  .globl" << std::endl;
        oss << "  .bss" << std::endl;
        oss << "  .align 2" << std::endl;
        oss << "  .type " << sym->getName() <<", @object" << std::endl;
        oss << "  .size " << sym->getName() << ", " << sym->getType()->getSize() << std::endl;
        oss << sym->getName() << ":" << std::endl;
        oss << "  .zero " << sym->getType()->getSize() << std::endl;
        return oss.str();
    }
};

/**
 * 基本块，包含指令列表和终结指令
 */
class BasicBlock {
private:
    IRSym* label;
    std::vector<IRInstr*> instrs;
    IRInstr* endInstr;
    std::vector<IRSym*> params; // 用于 SSA 形式的参数
    std::vector<Instr*> rv_instrs;
public:
    explicit BasicBlock(IRSym* label) : label(label), endInstr(nullptr) {}
    
    IRSym* getLabel() const { return label; }
    
    // 添加普通指令
    void addInstr(IRInstr* instr);

    void add(Instr* instr) { rv_instrs.push_back(instr); }
    
    // 设置终结指令（跳转、分支或返回）
    void setEndInstr(IRInstr* instr);
    
    // 添加基本块参数（用于 SSA 形式）
    void addParam(IRSym* param) {
        params.push_back(param);
    }
    
    // 获取所有指令（不包括终结指令）
    const std::vector<IRInstr*>& getInstrs() const { return instrs; }
    
    // 获取终结指令
    IRInstr* getEndInstr() const { return endInstr; }
    
    // 获取参数列表
    const std::vector<IRSym*>& getParams() const { return params; }
    
    // 打印基本块
    void print(std::ostream& os) const;

    std::string toRiscV() const;
};

/**
 * 函数定义
 */
class IRFunc : public IRDef {
private:
    std::vector<IRSym*> params;
    std::vector<BasicBlock*> blocks;
    std::set<IRFunc*> calls;
    std::unordered_map<std::string, BasicBlock*> blockMap;
    SymbolTable<IRSym*>* st{nullptr};
    int size {-1};
    int paramSize{-1};
    std::vector<Instr*> entry;
    IRSym* epilogueLabel {nullptr};
    std::vector<Instr*> epilogue;
public:
    IRFunc(IRSym* sym, const std::vector<IRSym*>& params)
        : IRDef(sym), params(params) {}

    IRFunc(const Func* func) : IRDef(new IRSym(func->getType(), "@"+func->getName())) {
        auto ps = func->getParams();
        for (int i = 0; i < ps.size(); i++) {
            auto p = ps[i];
            auto type = p->getType();
            if (auto t = dynamic_cast<FType*>(type)) {
                IRSym* sym = new IRSym(new PType(type), "@"+p->getName());
                addParam(sym);
            } else if (auto t = dynamic_cast<AType*>(type)) {
                IRSym* sym = new IRSym(new PType(type), "@"+p->getName());
                addParam(sym);
            } else {
                IRSym* sym = new IRSym(type, "@"+p->getName());
                addParam(sym);
            }
        }
    }

    void addEntryInstr(Instr* instr) { entry.push_back(instr); }
    void addEpilogueInstr(Instr* instr) { epilogue.push_back(instr); }

    void addCall(IRFunc* call) {
        if (call)
            calls.insert(call);
    }

    std::set<IRFunc*> getCalls() {
        return calls;
    }

    void setSize(int s) { size = s; }
    int getSize() { return size; }
    void setParamSize(int s) { paramSize = s; }
    int getParamSize() { return paramSize; }

    void setEpilogueLabel(IRSym* sym) {
        epilogueLabel = sym;
    }

    IRSym* getEpilogueLabel() { return epilogueLabel; }
    
    // 添加参数
    void addParam(IRSym* param) {
        params.push_back(param);
    }
    
    // 添加基本块
    void addBlock(BasicBlock* block);
    
    // 获取基本块
    BasicBlock* getBlock(const std::string& name);
    
    // 获取入口基本块
    BasicBlock* getEntryBlock() {
        return blocks.empty() ? nullptr : blocks[0];
    }

    std::vector<BasicBlock*> getBlocks() { return blocks; }
    
    // 获取参数列表
    const std::vector<IRSym*>& getParams() const { return params; }
    
    // 打印函数
    void print(std::ostream& os) const;
    
    std::string toString() const override {
        std::string result = "fun ";
        result += sym->getName() + "(";
        
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) result += ", ";
            result += params[i]->toString();
        }
        result += ") : ";
        if (sym->getType()) {
            result += sym->getType()->toString() + " ";
        } else {
            result += "void ";
        }
        return result;
    }

    void setST(SymbolTable<IRSym*>* table) { st = table; }
    SymbolTable<IRSym*>* getST() { return st; }

    std::string toRiscV() const;
};

/**
 * 完整的IR程序
 */
class IRProgram {
private:
    SymbolTable<IRSym*>* st{nullptr};
    std::vector<IRVar*> globls;
    std::vector<IRFunc*> funcs;

public:
    explicit IRProgram() {}
    
    // 添加全局变量
    void addGlobal(IRVar* global) {
        globls.push_back(global);
    }

    std::vector<IRVar*> getGlobal() { return globls; }
    
    // 添加函数
    void addFunc(IRFunc* func) {
        funcs.push_back(func);
    }

    std::vector<IRFunc*> getFunc() { return funcs; }

    void addDecl(IRDef* def) {
        if (auto func = dynamic_cast<IRFunc*>(def)) {
            addFunc(func);
        } else if (auto var = dynamic_cast<IRVar*>(def)) {
            addGlobal(var);
        }
    }
    
    // 获取函数
    IRFunc* getFunction(const std::string& name) {
        for (auto& func : funcs) {
            if (func->getSym()->getName() == name) {
                return func;
            }
        }
        return nullptr;
    }
    
    // 打印整个程序
    void print(std::ostream& os) const;
    void setST(SymbolTable<IRSym*>* table) { st = table; }
    SymbolTable<IRSym*>* getST() { return st; }

    void printRV(std::ostream& os) const {
        for (auto var : globls) {
            os << var->toRiscV() << std::endl;
        }
        for (auto func : funcs) {
            os << func->toRiscV() << std::endl;
        }
    }
};

/**
 * IR指令基类
 */
class IRInstr {
protected:
    std::vector<IRSym*> def;
    std::vector<IRSym*> use;

public:
    virtual ~IRInstr() = default;
    
    // 获取定义的符号
    const std::vector<IRSym*>& getDef() const { return def; }
    
    // 获取使用的符号
    const std::vector<IRSym*>& getUse() const { return use; }
    
    // 指令的字符串表示
    virtual std::string toString() const = 0;
    
    // 判断是否是终结指令（跳转、分支或返回）
    virtual bool isTerminator() const { return false; }
};

/**
 * 分配内存指令
 */
class IRAlloc : public IRInstr {
private:
    IRSym* dst;
    IType* type;
    /// @brief 相对于FP的位置
    int position {-1};
public:
    IRAlloc(IRSym* dst, IType* type) : dst(dst), type(type) {
        def.push_back(dst);
    }
    
    IRSym* getDst() const { return dst; }
    IType* getAllocType() const { return type; }
    
    std::string toString() const override {
        return dst->toString() + " = alloc " + type->toString() + (position == -1 ? "" : "[allocated in fp" + std::to_string(position) + "]") ;
    }
    void setPosition(int pos) { position = pos; }
    int getPosition() { return position; }
    
};

/**
 * 加载指令
 */
class IRLoad : public IRInstr {
private:
    IRSym* dst;
    IRSym* sym;

public:
    IRLoad(IRSym* dst, IRSym* sym) : dst(dst), sym(sym) {
        def.push_back(dst);
        use.push_back(sym);
    }
    
    IRSym* getDst() const { return dst; }
    IRSym* getSym() const { return sym; }
    
    std::string toString() const override {
        return dst->toString() + " = load " + sym->toString();
    }
};

/**
 * 存储指令
 */
class IRStore : public IRInstr {
private:
    IRVal* src;
    IRSym* sym;

public:
    IRStore(IRVal* src, IRSym* sym)
        : src(src), sym(sym) {
        
        use.push_back(sym);
        // 如果源是一个符号，也添加到使用列表
        if (auto srcSym = dynamic_cast<IRSym*>(this->src)) {
            use.push_back(srcSym);
        }
    }
    
    IRVal* getSrc() const { return src; }
    IRSym* getSym() const { return sym; }
    
    std::string toString() const override {
        return "store " + src->toString() + ", " + sym->toString();
    }
};

/**
 * 指针运算 *T -> *T
 */
class IRGetPtr : public IRInstr {
private:
    IRSym* dst;
    IRSym* sym;
    IRVal* offset;

public:
    IRGetPtr(IRSym* dst, IRSym* sym, IRVal* offset)
        : dst(dst), sym(sym), offset(offset) {
        
        def.push_back(dst);
        use.push_back(sym);
        
        // 如果偏移量是一个符号，也添加到使用列表
        if (auto offsetSym = dynamic_cast<IRSym*>(this->offset)) {
            use.push_back(offsetSym);
        }
    }
    
    IRSym* getDst() const { return dst; }
    IRSym* getSym() const { return sym; }
    IRVal* getOffset() const { return offset; }
    
    std::string toString() const override {
        return dst->toString() + " = getptr " + sym->toString() + ", " + offset->toString();
    }
};

/**
 * 数组元素指针指令 *[T, n] -> *T
 */
class IRGetElPtr : public IRInstr {
private:
    IRSym* dst;
    IRSym* sym;
    IRVal* offset;

public:
    IRGetElPtr(IRSym* dst, IRSym* sym, IRVal* offset)
        : dst(dst), sym(sym), offset(offset) {
        
        def.push_back(dst);
        use.push_back(sym);
        
        // 如果偏移量是一个符号，也添加到使用列表
        if (auto offsetSym = dynamic_cast<IRSym*>(this->offset)) {
            use.push_back(offsetSym);
        }
    }
    
    IRSym* getDst() const { return dst; }
    IRSym* getSym() const { return sym; }
    IRVal* getOffset() const { return offset; }
    
    std::string toString() const override {
        return dst->toString() + " = getelptr " + sym->toString() + ", " + offset->toString();
    }
};

/**
 * 二元运算指令
 */
class IRBinary : public IRInstr {
private:
    IRSym* dst;
    IRVal* src1;
    IRVal* src2;
    char op;

public:
    IRBinary(IRSym* dst, IRVal* src1, IRVal* src2, char op)
        : dst(dst), src1(src1), src2(src2), op(op) {
        
        def.push_back(dst);
        
        // 如果操作数是符号，添加到使用列表
        if (auto src1Sym = dynamic_cast<IRSym*>(this->src1)) {
            use.push_back(src1Sym);
        }
        
        if (auto src2Sym = dynamic_cast<IRSym*>(this->src2)) {
            use.push_back(src2Sym);
        }
    }
    
    IRSym* getDst() const { return dst; }
    IRVal* getSrc1() const { return src1; }
    IRVal* getSrc2() const { return src2; }
    char getOp() const { return op; }
    
    std::string toString() const override {
        std::string opStr;
        switch (op) {
            case '+': opStr = "add"; break;
            case 'l': opStr = "sle"; break;
            case '*': opStr = "mul"; break;
            case '=': opStr = "seq"; break;
            case '<': opStr = "slt"; break;
            case '!': opStr = "sne"; break;
            default: opStr = std::string(1, op);
        }
        
        return dst->toString() + " = " + opStr + " " + src1->toString() + ", " + src2->toString();
    }
};

/**
 * 条件分支指令
 */
class IRBr : public IRInstr {
private:
    IRVal* val;
    IRSym* thenLabel;
    std::vector<IRVal*> thenArgs;
    IRSym* elseLabel;
    std::vector<IRVal*> elseArgs;

public:
    IRBr(IRVal* val, 
         IRSym* thenLabel, IRSym* elseLabel, 
         std::vector<IRVal*> thenArgs = {}, std::vector<IRVal*> elseArgs = {})
        : val(val), thenLabel(thenLabel), thenArgs(thenArgs), elseLabel(elseLabel), elseArgs(elseArgs) {
        
        // 如果条件是符号，添加到使用列表
        if (auto valSym = dynamic_cast<IRSym*>(this->val)) {
            use.push_back(valSym);
        }
        
        // 添加参数中的符号到使用列表
        for (const auto& arg : this->thenArgs) {
            if (auto argSym = dynamic_cast<IRSym*>(arg)) {
                use.push_back(argSym);
            }
        }
        
        for (const auto& arg : this->elseArgs) {
            if (auto argSym = dynamic_cast<IRSym*>(arg)) {
                use.push_back(argSym);
            }
        }
    }
    
    IRVal* getVal() const { return val; }
    IRSym* getThenLabel() const { return thenLabel; }
    IRSym* getElseLabel() const { return elseLabel; }
    
    const std::vector<IRVal*>& getThenArgs() const { return thenArgs; }
    const std::vector<IRVal*>& getElseArgs() const { return elseArgs; }
    
    std::string toString() const override {
        std::string result = "br " + val->toString() + ", " + thenLabel->toString();
        
        if (!thenArgs.empty()) {
            result += "(";
            for (size_t i = 0; i < thenArgs.size(); ++i) {
                if (i > 0) result += ", ";
                result += thenArgs[i]->toString();
            }
            result += ")";
        }
        
        result += ", " + elseLabel->toString();
        
        if (!elseArgs.empty()) {
            result += "(";
            for (size_t i = 0; i < elseArgs.size(); ++i) {
                if (i > 0) result += ", ";
                result += elseArgs[i]->toString();
            }
            result += ")";
        }
        
        return result;
    }
    
    bool isTerminator() const override { return true; }
};

/**
 * 无条件跳转指令
 */
class IRJump : public IRInstr {
private:
    IRSym* label;
    std::vector<IRVal*> args;

public:
    IRJump(IRSym* label, std::vector<IRVal*> args = {})
        : label(label), args(args) {
        
        // 添加参数中的符号到使用列表
        for (const auto& arg : this->args) {
            if (auto argSym = dynamic_cast<IRSym*>(arg)) {
                use.push_back(argSym);
            }
        }
    }
    
    IRSym* getLabel() const { return label; }
    const std::vector<IRVal*>& getArgs() const { return args; }
    
    std::string toString() const override {
        std::string result = "jump " + label->toString();
        
        if (!args.empty()) {
            result += "(";
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) result += ", ";
                result += args[i]->toString();
            }
            result += ")";
        }
        
        return result;
    }
    
    bool isTerminator() const override { return true; }
};

/**
 * 整数转浮点数指令
 */
class IRI2F : public IRInstr {
private:
    IRSym* dst;
    IRVal* src;

public:
    IRI2F(IRSym* dst, IRVal* src) : dst(dst), src(src) {
        def.push_back(dst);
        if (auto srcSym = dynamic_cast<IRSym*>(src)) {
            use.push_back(srcSym);
        }
    }
    
    IRSym* getDst() const { return dst; }
    IRVal* getSrc() const { return src; }
    
    std::string toString() const override {
        return dst->toString() + " = i2f " + src->toString();
    }
};

/**
 * 浮点数转整数指令
 */
class IRF2I : public IRInstr {
private:
    IRSym* dst;
    IRVal* src;

public:
    IRF2I(IRSym* dst, IRVal* src) : dst(dst), src(src) {
        def.push_back(dst);
        if (auto srcSym = dynamic_cast<IRSym*>(src)) {
            use.push_back(srcSym);
        }
    }
    
    IRSym* getDst() const { return dst; }
    IRVal* getSrc() const { return src; }
    
    std::string toString() const override {
        return dst->toString() + " = f2i " + src->toString();
    }
};

/**
 * 函数调用指令
 */
class IRCall : public IRInstr {
private:
    IRSym* func;
    std::vector<IRVal*> args;
    IRSym* result; // 可选的返回值
    IRFunc* resolution;

public:
    // 无返回值的构造函数
    IRCall(IRSym* func, std::vector<IRVal*> args)
        : func(func), args(args), result(nullptr), resolution(nullptr) {
        
        use.push_back(func);
        
        // 添加参数中的符号到使用列表
        for (const auto& arg : this->args) {
            if (auto argSym = dynamic_cast<IRSym*>(arg)) {
                use.push_back(argSym);
            }
        }
    }
    
    // 有返回值的构造函数
    IRCall(IRSym* result, IRSym* func, std::vector<IRVal*> args)
        : func(func), args(args), result(result), resolution(nullptr) {
        
        if (result) {
            def.push_back(result);
        }
        
        use.push_back(func);
        
        // 添加参数中的符号到使用列表
        for (const auto& arg : this->args) {
            if (auto argSym = dynamic_cast<IRSym*>(arg)) {
                use.push_back(argSym);
            }
        }
    }
    
    IRSym* getFunc() const { return func; }
    const std::vector<IRVal*>& getArgs() const { return args; }
    IRSym* getResult() const { return result; }
    IRFunc* getResolution() { return resolution; }
    void resolve(IRFunc* func) { resolution = func; }
    
    std::string toString() const override {
        std::string resultStr = result ? result->toString() + " = " : "";
        std::string argsStr = "";
        
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) argsStr += ", ";
            argsStr += args[i]->toString();
        }
        
        return resultStr + "call " + func->toString() + "(" + argsStr + ")";
    }
};

/**
 * 返回指令
 */
class IRRet : public IRInstr {
private:
    IRVal* val; // 可选的返回值

public:
    // 无返回值的构造函数
    IRRet() : val(nullptr) {}
    
    // 有返回值的构造函数
    explicit IRRet(IRVal* val) : val(val) {
        // 如果返回值是符号，添加到使用列表
        if (auto valSym = dynamic_cast<IRSym*>(this->val)) {
            use.push_back(valSym);
        }
    }
    
    IRVal* getVal() const { return val; }
    
    std::string toString() const override {
        if (val) {
            return "ret " + val->toString();
        }
        return "ret";
    }
    
    bool isTerminator() const override { return true; }
};


#endif