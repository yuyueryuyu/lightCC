#ifndef TYPE_HPP
#define TYPE_HPP

#include <vector>
#include <string>

class IType;
class BType;
class AType;
class FType;
class Type;

/// @brief 类型基类
class IType {
public:
    /// @brief 析构函数
    virtual ~IType() = default;

    /// @brief 转换为字符串 
    /// @return 字符串
    virtual std::string toString() const = 0;

    /// @brief 检查类型是否相等
    /// @param other 另一个类型
    /// @return bool值
    virtual bool equals(const IType* other) const = 0;

    /// @brief 拷贝类型
    /// @return 拷贝指针
    virtual IType* clone() const = 0;

    virtual int getSize() const = 0;
};

/// @brief 基本类型
class BType : public IType {
private:
    /// @brief 名称
    std::string name;
public:
    /// @brief 构造函数
    /// @param name 名称
    BType(std::string name) : name(name) {}

    /// @brief 用类型注解构造类型
    /// @param t 类型注解
    BType(Type* t);

    /// @brief 拷贝构造函数 
    /// @param other 
    BType(const BType& other) : name(other.name) {}

    /// @brief 获取类型名称
    /// @return 名称
    std::string getName() const { return name; }

    /// @brief 转换为字符串
    /// @return 字符串
    virtual std::string toString() const override {
        return name;
    }

    /// @brief 检查类型是否相同
    /// @param other 类型
    /// @return bool值
    virtual bool equals(const IType* other) const override {
        const BType* otherBType = dynamic_cast<const BType*>(other);
        return otherBType && name == otherBType->name;
    }

    /// @brief 拷贝
    /// @return 指针
    virtual BType* clone() const override {
        return new BType(name);
    }

    virtual int getSize() const { return 4; }
};

/// @brief 数组类型
class AType : public IType {
private:
    /// @brief 基类型
    BType* base;
    /// @brief 数组长度 
    int len;
public:
    /// @brief 构造函数
    /// @param base 基类型
    /// @param len 数组长度
    AType(BType* base, int len) : base(new BType(*base)), len(len) {}
    
    /// @brief 拷贝构造函数 
    /// @param other 
    AType(const AType& other) : base(new BType(*other.base)), len(other.len) {}

    /// @brief 析构函数
    ~AType() { delete base; }

    /// @brief 获取基类型
    /// @return 基类型
    BType* getBase() const { return base; }
    
    /// @brief 获取数组长度
    /// @return 数组长度
    int getLen() const { return len; }

    /// @brief 转换为字符串
    /// @return 字符串
    virtual std::string toString() const override {
        return base->toString() + "[" + std::to_string(len) + "]";
    }

    /// @brief 检测类型是否相同
    /// @param other 
    /// @return bool值
    virtual bool equals(const IType* other) const override {
        const AType* otherAType = dynamic_cast<const AType*>(other);
        return otherAType &&
            base->equals(otherAType->base) &&
            len == otherAType->len;
    }

    /// @brief 拷贝
    /// @return 
    virtual AType* clone() const override {
        return new AType(base, len);
    }

    virtual int getSize() const { return 4 * len; }
};

/// @brief 函数类型
class FType : public IType {
private:
    /// @brief 返回类型
    BType* ret;
    /// @brief 形参类型列表
    std::vector<IType*> params;
public:
    /// @brief 构造函数
    /// @param r 返回类型
    /// @param p 形参类型列表
    FType(BType* r) : ret(new BType(*r)) {}

    /// @brief 构造函数
    /// @param r 返回类型
    /// @param p 形参类型列表
    FType(BType* r, std::vector<IType*> params) : ret(new BType(*r)), params(params) {}

    /// @brief 拷贝构造函数
    /// @param other 
    FType(const FType& other) : ret(new BType(*other.ret)) {
        // 深拷贝参数列表
        for (IType* param : other.params) {
            params.push_back(param->clone());
        }
    }

    /// @brief 析构函数
    ~FType() {
        delete ret;
        // 清理参数列表中的动态分配内存
        for (IType* param : params) {
            delete param;
        }
    }

    /// @brief 获取返回类型
    /// @return 返回类型
    BType* getRetType() const { return ret; }
    
    /// @brief 获取形参类型列表
    /// @return 形参类型列表
    std::vector<IType*> getParamsType() const { return params; }

    /// @brief 添加形参类型
    /// @param param 
    void addParam(IType* param) {
        if (param) {
            params.push_back(param->clone());
        }
    }

    /// @brief 转换为字符串
    /// @return 字符串
    virtual std::string toString() const override {
        std::string result = ret->toString() + "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) result += ", ";
            result += params[i]->toString();
        }
        result += ")";
        return result;
    }

    /// @brief 检测类型是否相同
    /// @param other 
    /// @return 
    virtual bool equals(const IType* other) const override {
        const FType* otherFType = dynamic_cast<const FType*>(other);
        if (!otherFType || !ret->equals(otherFType->ret) || params.size() != otherFType->params.size()) {
            return false;
        }

        // 比较每个参数类型
        for (size_t i = 0; i < params.size(); ++i) {
            if (!params[i]->equals(otherFType->params[i])) {
                return false;
            }
        }
        return true;
    }

    /// @brief 拷贝类型
    /// @return 
    virtual FType* clone() const override {
        std::vector<IType*> clonedParams;
        for (IType* param : params) {
            clonedParams.push_back(param->clone());
        }
        return new FType(ret, clonedParams);
    }

    virtual int getSize() const { return 0; }
};

/// @brief 指针类型
class PType : public IType {
private:
    /// @brief 基类型
    IType* base;
public:
    /// @brief 构造函数
    /// @param base 基类型
    PType(IType* base) : base(base->clone()) {}
    
    /// @brief 拷贝构造函数 
    /// @param other 
    PType(const PType& other) : base(other.base->clone()) {}

    /// @brief 析构函数
    ~PType() { delete base; }

    /// @brief 获取基类型
    /// @return 基类型
    IType* getBase() const { return base; }

    /// @brief 转换为字符串
    /// @return 字符串
    virtual std::string toString() const override {
        return base->toString() + "*";
    }

    /// @brief 检测类型是否相同
    /// @param other 
    /// @return bool值
    virtual bool equals(const IType* other) const override {
        const PType* otherPType = dynamic_cast<const PType*>(other);
        return otherPType &&
            base->equals(otherPType->base);
    }

    /// @brief 拷贝
    /// @return 
    virtual PType* clone() const override {
        return new PType(base);
    }

    virtual int getSize() const { return 4; }
};

/// @brief int类型
inline BType INT_TYPE("int");

/// @brief float类型
inline BType FLOAT_TYPE("float");

/// @brief 内建的bool类型，仅用于条件表达式
inline BType BOOL_TYPE("bool");

/// @brief void类型，仅用于函数返回类型
inline BType VOID_TYPE("void");

/// @brief label类型，IR中的标签标识符的类型。
inline BType LABEL_TYPE("label");

#endif