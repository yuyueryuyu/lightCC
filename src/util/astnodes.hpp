#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <vector>
#include <string>
#include "token.hpp" 
#include "type.hpp"  
#include "symboltable.hpp"
#include "symbol.hpp"

class Node;
class Decl;
class Expr;
class Stmt;
class Type;
class Literal;
class Int;
class Float;
class Id;
class Index;
class Binary;
class Call;
class Assign;
class If;
class While;
class Return;
class ExprEval;
class Block;
class VarDecl;
class FuncDecl;
class Program;

/// @brief AST结点类基类
class Node {
private:
    /// @brief 结点在源程序的位置
    size_t pos[4];
public:
    /// @brief 根据token构建结点类
    /// @param token 
    Node(Token start, Token end) {
        pos[0] = start.getPos()[0];
        pos[1] = start.getPos()[1];
        pos[2] = end.getPos()[2];
        pos[3] = end.getPos()[3];
    }

    /// @brief 直接根据位置构建节点类
    /// @param p 位置
    Node(size_t p[]) {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
        pos[3] = p[3];
    }

    /// @brief 析构函数
    virtual ~Node() = default;

    /// @brief 获取结点位置
    /// @return int[2]表示结点的开始、结束位置（字节数）
    size_t* getPos() { return pos; }
};

/// @brief AST结点声明类
class Decl : public Node {
public:
    /// @brief 根据token构建声明类
    /// @param token 
    Decl(Token start, Token end) : Node(start, end) {}

    /// @brief 析构函数
    virtual ~Decl() = default;
};


/// @brief AST结点表达式类
class Expr : public Node {
private:
    IType* type{nullptr};
public:
    /// @brief 根据token构建表达式类
    /// @param token 
    Expr(Token start, Token end) : Node(start, end) {}

    /// @brief 根据位置构建表达式类
    /// @param pos 
    Expr(size_t pos[]) : Node(pos) {}

    /// @brief 析构函数
    virtual ~Expr() = default;
    
    void setType(IType* t) { type = t; }
    IType* getType() { return type; }
};

/// @brief 类型转换表达式
class Cast : public Expr {
private:
    /// @brief 源类型
    IType* from;

    /// @brief 目标类型
    IType* to;

    /// @brief 源表达式
    Expr* expr;
public:
    /// @brief 构造函数
    /// @param expr 源表达式
    /// @param from 源类型
    /// @param to 目标类型
    Cast(Expr* expr, IType* from, IType* to) : Expr(expr->getPos()), from(from), to(to), expr(expr) {
        setType(to);
    }

    /// @brief 析构函数
    ~Cast() { delete expr; }

    /// @brief 获取源类型
    /// @return 源类型
    IType* getFrom() { return from; }

    /// @brief 获取目标类型
    /// @return 目标类型
    IType* getTo() { return to; }

    /// @brief 获取源表达式
    /// @return 源表达式
    Expr* getExpr() { return expr; }
};

/// @brief 字面量
class Literal : public Expr {
public:
    /// @brief 构造函数
    /// @param token 
    Literal(Token start, Token end) : Expr(start, end) {}

    /// @brief 析构函数
    virtual ~Literal() = default;
};

/// @brief 整数字面量
class Int : public Literal {
private:
    /// @brief AST结点的整数值
    int value;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param val 整数值
    Int(Token start, Token end, int val) : Literal(start, end), value(val) {}

    /// @brief 获取值
    /// @return AST结点的整数值
    int getValue() const { return value; }
};

/// @brief 浮点字面量
class Float : public Literal {
private:
    /// @brief AST结点的浮点数值
    float value;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param val 数值
    Float(Token start, Token end, float val) : Literal(start, end), value(val) {}

    /// @brief 获取浮点数值
    /// @return AST结点的浮点数值
    float getValue() const { return value; }
};

/// @brief 标识符
class Id : public Expr {
private:
    /// @brief 名称
    std::string name;
    Symbol* resolution{nullptr};
public:
    /// @brief 构造函数
    /// @param token 
    /// @param name 名称
    Id(Token start, Token end, std::string name) : Expr(start, end), name(name) {}

    /// @brief 获取名称 
    /// @return 标识符的名称
    std::string getName() const { return name; }

    void resolve(Symbol* sym) { resolution = sym; }
    Symbol* getResolution() { return resolution; }
};

/// @brief 数组访问表达式
class Index : public Expr {
private:
    /// @brief 数组标识符
    Id* id;

    /// @brief 数组下标
    Expr* index;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param id 数组标识符
    /// @param index 数组下标
    Index(Token start, Token end, Id* id, Expr* index) : Expr(start, end), id(id), index(index) {}

    /// @brief 析构函数
    ~Index() { delete id; delete index; }

    /// @brief 获取数组标识符
    /// @return 数组标识符
    Id* getId() const { return id; }

    /// @brief 获取数组下标
    /// @return 数组下标
    Expr* getIndex() const { return index; }
};

/// @brief 二元表达式
class Binary : public Expr {
private:
    /// @brief 运算符 根据语言定义，支持如下运算符：
    /// +：加法；*：乘法；=：相等；<：小于； l :小于等于
    char op;
    /// @brief 左操作元
    Expr* left;
    /// @brief 右操作元
    Expr* right;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param op 运算符
    /// @param left 左操作元
    /// @param right 右操作元
    Binary(Token start, Token end, char op, Expr* left, Expr* right) :
        Expr(start, end), op(op), left(left), right(right) {
    }

    /// @brief 析构函数
    ~Binary() { delete left; delete right; }

    /// @brief 获取运算符
    /// @return 运算符
    char getOp() const { return op; }

    /// @brief 获取左操作元
    /// @return 左操作元
    Expr* getLeft() const { return left; }

    /// @brief 获取右操作元
    /// @return 右操作元
    Expr* getRight() const { return right; }

    /// @brief 将左操作元进行类型转换
    /// @param from 源类型
    /// @param to 目标类型
    void castLeft(IType* from, IType* to) {
        left = new Cast(left, from, to);
    }

    /// @brief 将右操作元进行类型转换
    /// @param from 源类型
    /// @param to 目标类型
    void castRight(IType* from, IType* to) {
        right = new Cast(right, from, to);
    }
};

/// @brief 函数调用表达式
class Call : public Expr {
private:
    /// @brief 函数标识符
    Id* id;

    /// @brief 实参列表
    std::vector<Expr*> args;

    Func* resolution;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param id 函数标识符 
    /// @param args 实参列表
    Call(Token start, Token end, Id* id, std::vector<Expr*> args) :
        Expr(start, end), id(id), args(args) {
    }

    /// @brief 析构函数
    ~Call() {
        delete id;
        for (auto arg : args) delete arg;
    }

    /// @brief 获取函数标识符
    /// @return 函数标识符
    Id* getId() const { return id; }


    /// @brief 获取实参列表
    /// @return 实参列表
    std::vector<Expr*> getArgs() const { return args; }

    /// @brief 对实参列表的第i号元素进行类型转换
    /// @param from 源类型
    /// @param to 目标类型
    /// @param i 下标
    void castArg(IType* from, IType* to, int i) {
        args[i] = new Cast(args[i], from, to);
    }

    Func* getResolution() { return resolution; }
    void resolve(Func* func) { resolution = func; }
};

/// @brief 类型注解
class Type : public Node {
private:
    /// @brief 名称
    std::string name;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param typeName 类型名称 
    Type(Token start, Token end, std::string typeName) : Node(start, end), name(typeName) {}

    /// @brief 获取类型名
    /// @return 类型名
    std::string getName() const { return name; }
};

/// @brief 语句
class Stmt : public Node {
public:
    /// @brief 构造函数
    /// @param token 
    Stmt(Token start, Token end) : Node(start, end) {}
    
    /// @brief 析构函数 
    virtual ~Stmt() = default;
};

/// @brief 赋值语句
class Assign : public Stmt {
private:
    /// @brief 左值
    Expr* target;

    /// @brief 右值
    Expr* value;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param target 左值
    /// @param val 右值
    Assign(Token start, Token end, Expr* target, Expr* val) : Stmt(start, end), target(target), value(val) {}
    
    /// @brief 析构函数
    ~Assign() { delete target; delete value; }
    
    /// @brief 获取左值
    /// @return 左值
    Expr* getTarget() const { return target; }
    
    /// @brief 获取右值
    /// @return 右值
    Expr* getValue() const { return value; }

    /// @brief 对右值进行类型转换
    /// @param from 源类型
    /// @param to 目标类型
    void castVal(IType* from, IType* to) {
        value = new Cast(value, from, to);
    }
};

/// @brief 条件语句
class If : public Stmt {
private:
    /// @brief 条件
    Expr* cond;
    /// @brief 条件为真跳转的语句
    Stmt* thenStmt;
    /// @brief 条件为假跳转的语句
    Stmt* elseStmt;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param cond 条件 
    /// @param thenStmt 条件为真跳转的语句 
    /// @param elseStmt 条件为假跳转的语句 (默认为null)
    If(Token start, Token end, Expr* cond, Stmt* thenStmt, Stmt* elseStmt = nullptr) :
        Stmt(start, end), cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {
    }

    /// @brief 析构函数
    ~If() { delete cond; delete thenStmt; delete elseStmt; }
    
    /// @brief 获取条件表达式
    /// @return 条件表达式
    Expr* getCond() const { return cond; }

    /// @brief 获取条件为真的语句
    /// @return 条件为真的语句
    Stmt* getThenStmt() const { return thenStmt; }

    /// @brief 获取条件为假的语句
    /// @return 条件为假的语句
    Stmt* getElseStmt() const { return elseStmt; }

    /// @brief 将条件类型转换为内建BOOL类型
    /// @param from 源类型
    void castCond(IType* from) {
        cond = new Cast(cond, from, new BType(BOOL_TYPE));
    }
};

/// @brief 循环语句
class While : public Stmt {
private:
    /// @brief 条件
    Expr* cond;
    /// @brief 循环体
    Stmt* body;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param cond 条件
    /// @param body 循环体
    While(Token start, Token end, Expr* cond, Stmt* body) : Stmt(start, end), cond(cond), body(body) {}

    /// @brief 析构函数
    ~While() { delete cond; delete body; }
    
    /// @brief 获取条件表达式
    /// @return 条件表达式
    Expr* getCond() const { return cond; }
    
    /// @brief 获取循环体
    /// @return 循环体
    Stmt* getBody() const { return body; }

    /// @brief 将条件转换为内建BOOL类型
    /// @param from 源类型
    void castCond(IType* from) {
        cond = new Cast(cond, from, new BType(BOOL_TYPE));
    }
};

/// @brief 返回语句
class Return : public Stmt {
private:
    /// @brief 返回值
    Expr* value;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param val 返回值（默认为空，虽然文法是不准为空的，不准为空为什么要有void函数，都到最后再返回吗）
    Return(Token start, Token end, Expr* val = nullptr) : Stmt(start, end), value(val) {}
    
    /// @brief 析构函数
    ~Return() { delete value; }
    
    /// @brief 获取返回值
    /// @return 返回值
    Expr* getValue() const { return value; }

    /// @brief 对返回值进行类型转换
    /// @param from 源类型
    /// @param to 目标类型
    void castVal(IType* from, IType* to) {
        value = new Cast(value, from, to);
    }
};

/// @brief 表达式语句（文法里只有函数表达式允许单独作为语句捏）
class ExprEval : public Stmt {
private:
    /// @brief 表达式
    Expr* expr;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param e 表达式
    ExprEval(Token start, Token end, Expr* e) : Stmt(start, end), expr(e) {}
    
    /// @brief 析构函数
    ~ExprEval() { delete expr; }
    
    /// @brief 获取表达式
    /// @return 表达式
    Expr* getExpr() const { return expr; }
};

/// @brief 语句块（块里只允许有语句，which means不用维护语句块的作用域了hhh
class Block : public Stmt {
private:
    /// @brief 语句列表
    std::vector<Stmt*> body;
public:
    /// @brief 构造函数
    /// @param token 
    /// @param statements 语句列表 
    Block(Token start, Token end, std::vector<Stmt*> statements) : Stmt(start, end), body(statements) {}
    
    /// @brief 析构函数
    ~Block() { for (auto stmt : body) delete stmt; }
    
    /// @brief 获取语句列表
    /// @return 语句列表
    std::vector<Stmt*> getBody() const { return body; }
};

/// @brief 变量声明
class VarDecl : public Decl {
private:
    /// @brief 类型注解
    Type* type;

    /// @brief 变量名标识符
    Id* id;

    /// @brief 长度，函数数组类形参为-1，标量为0，数组为正值 （根据文法只有一维数组！）
    int len;

    Var* resolution{nullptr};
public:
    /// @brief 构造函数
    /// @param token 
    /// @param type 类型
    /// @param id 变量名标识符
    /// @param len 长度
    VarDecl(Token start, Token end, Type* type, Id* id, int len = 0) :
        Decl(start, end), type(type), id(id), len(len) {
    }

    /// @brief 析构函数
    ~VarDecl() { delete type; delete id; }
    
    /// @brief 获取类型注解
    /// @return 类型注解
    Type* getType() const { return type; }
    
    /// @brief 获取标识符
    /// @return 标识符
    Id* getId() const { return id; }
    
    /// @brief 获取数组长度
    /// @return 长度
    int getLen() const { return len; }

    void resolve(Var* sym) { resolution = sym; }
    Var* getResolution() { return resolution; }
};

/// @brief 函数声明
class FuncDecl : public Decl {
private:
    /// @brief 返回类型
    Type* retType;
    /// @brief 函数标识符
    Id* id;
    /// @brief 形参列表
    std::vector<Decl*> params;
    /// @brief 局部变量声明列表
    std::vector<Decl*> decls;
    /// @brief 语句列表
    std::vector<Stmt*> stmts;

    /// @brief 函数局部符号表
    SymbolTable<Symbol*>* funcST{nullptr};

    Func* resolution{nullptr};
public:
    /// @brief 构造函数
    /// @param token 
    /// @param retType 返回类型
    /// @param funcId 函数标识符
    /// @param params 形参列表
    /// @param decls 局部变量声明列表 
    /// @param stmts 语句列表
    FuncDecl(Token start, Token end, Type* retType, Id* funcId, std::vector<Decl*> params,
        std::vector<Decl*> decls, std::vector<Stmt*> stmts) :
        Decl(start, end), retType(retType), id(funcId), params(params), decls(decls), stmts(stmts) {
    }

    /// @brief 析构函数
    ~FuncDecl() {
        delete retType;
        delete id;
        for (auto param : params) delete param;
        for (auto decl : decls) delete decl;
        for (auto stmt : stmts) delete stmt;
    }

    /// @brief 获取返回类型
    /// @return 返回类型
    Type* getRetType() const { return retType; }
    
    /// @brief 获取函数标识符
    /// @return 函数标识符
    Id* getId() const { return id; }

    /// @brief 获取形参列表
    /// @return 形参列表
    std::vector<Decl*> getParams() const { return params; }
    
    /// @brief 获取局部声明列表
    /// @return 局部声明列表
    std::vector<Decl*> getDecls() const { return decls; }
    
    /// @brief 获取语句列表
    /// @return 语句列表
    std::vector<Stmt*> getStmts() const { return stmts; }

    void setST(SymbolTable<Symbol*>* st) { funcST = st; }
    SymbolTable<Symbol*>* getST() { return funcST; }

    void resolve(Func* sym) { resolution = sym; }
    Func* getResolution() { return resolution; }
};

/// @brief 程序
class Program : public Node {
private:
    /// @brief 声明列表
    std::vector<Decl*> decls;

    /// @brief 语句列表
    std::vector<Stmt*> stmts;

    /// @brief 全局符号表
    SymbolTable<Symbol*>* globalST{nullptr};
public:
    /// @brief 构造函数
    /// @param token 
    /// @param d 声明列表
    /// @param s 语句列表
    Program(Token start, Token end, std::vector<Decl*> d, std::vector<Stmt*> s) :
        Node(start, end), decls(d), stmts(s) {
    }

    /// @brief 析构函数
    ~Program() {
        for (auto decl : decls) delete decl;
        for (auto stmt : stmts) delete stmt;
    }

    /// @brief 获取声明列表
    /// @return 声明列表
    std::vector<Decl*> getDecls() const { return decls; }
    
    /// @brief 获取语句列表
    /// @return 语句列表
    std::vector<Stmt*> getStmts() const { return stmts; }

    void setST(SymbolTable<Symbol*>* st) { globalST = st; }
    SymbolTable<Symbol*>* getST() { return globalST; }
};

#endif