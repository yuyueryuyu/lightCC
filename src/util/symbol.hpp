#ifndef SYMBOL_HPP
#define SYMBOL_HPP
#include <string>
#include "type.hpp"

// 前向声明
class Symbol;
class Var;
class Func;

/// @brief 符号基类
class Symbol {
protected:
    /// @brief 符号名
    std::string name;
    /// @brief 符号类型
    IType* type;
public:
    /// @brief 构造函数
    /// @param n 符号名
    /// @param t 符号类型
    Symbol(std::string n, IType* t) : name(n), type(t ? t->clone() : nullptr) {}
    
    /// @brief 拷贝构造函数
    /// @param other 
    Symbol(const Symbol& other) : name(other.name), type(other.type ? other.type->clone() : nullptr) {}
    
    /// @brief 析构函数
    virtual ~Symbol() { delete type; }
    
    /// @brief 获取符号名
    /// @return 
    std::string getName() const { return name; }
    
    /// @brief 获取符号类型 
    /// @return 
    IType* getType() const { return type; }
    
    /// @brief 转换为字符串
    /// @return 
    virtual std::string toString() const {
        return name + " : " + (type ? type->toString() : "unknown");
    }
    
    /// @brief 拷贝
    /// @return 
    virtual Symbol* clone() const = 0;
};

/// @brief 变量符号
class Var : public Symbol {
public:
    /// @brief 构造函数
    /// @param n 变量名
    /// @param t 变量类型
    Var(std::string n, IType* t) : Symbol(n, t){}
    
    /// @brief 拷贝构造函数
    /// @param other 
    Var(const Var& other) : Symbol(other) {}
    
    /// @brief 转换为字符串
    /// @return 
    virtual std::string toString() const override {
        std::string result = Symbol::toString();
        return result;
    }
    
    /// @brief 拷贝
    /// @return 
    virtual Var* clone() const override {
        return new Var(*this);
    }
};

/// @brief 函数符号
class Func : public Symbol {
private:
    /// @brief 返回类型
    BType* retType;
    /// @brief 形参列表
    std::vector<Symbol*> params;
    /// @brief 实参列表
    std::vector<Symbol*> locals;

    bool isParam{false};
    
public:
    /// @brief 构造函数
    /// @param n 函数名
    /// @param ret 返回类型
    Func(std::string n, BType* ret) : Symbol(n, new FType(ret)), retType(ret->clone()) {}
    
    /// @brief 拷贝构造函数
    /// @param other 
    Func(const Func& other) : Symbol(other), retType(other.retType->clone()), params(other.params), locals(other.locals) {}

    /// @brief 析构函数
    ~Func() {
        delete retType;
        for (Symbol* param : params) delete param;
        for (Symbol* local : locals) delete local;
    }
    
    /// @brief 获取返回类型
    /// @return 
    IType* getRetType() const { return retType; }
    
    /// @brief 添加形参
    /// @param param 形参
    void addParam(Symbol* param) {
        if (param) {
            params.push_back(param->clone());
            if (auto ftype = dynamic_cast<FType*>(type)) {
                ftype->addParam(param->getType());
            }
        }
    }
    
    /// @brief 添加局部变量
    /// @param local 局部变量
    void addLocal(Symbol* local) {
        if (local) {
            locals.push_back(local->clone());
        }
    }
    
    /// @brief 获取形参
    /// @return 
    std::vector<Symbol*> getParams() const { return params; }
    
    /// @brief 获取局部变量
    /// @return 
    std::vector<Symbol*> getLocals() const { return locals; }
    
    /// @brief 获取形参数量
    /// @return 
    int getParamCount() const { return params.size(); }
    
    /// @brief 获取局部变量数量
    /// @return 
    int getLocalCount() const { return locals.size(); }
    
    /// @brief 转换为字符串
    /// @return 
    virtual std::string toString() const override {
        std::string result = name + "(";
        for (size_t i = 0; i < params.size(); i++) {
            if (i > 0) result += ", ";
            result += params[i]->getType()->toString() + " " + params[i]->getName();
        }
        result += ") : " + (retType ? retType->toString() : "void");
        return result;
    }
    
    /// @brief 拷贝
    /// @return 
    virtual Func* clone() const override {
        return new Func(*this);
    }

    bool isParameter() { return isParam; }
    void setParam() { isParam = true; }
};

#endif