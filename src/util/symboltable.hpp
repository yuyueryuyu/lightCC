#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <map>
#include <string>
#include <iostream>

/// @brief 符号表模板类 
/// @tparam 符号类
template<class T>
class SymbolTable {
private:
    SymbolTable<T>* parent;
    std::map<std::string, T> tab;
    
public:
    SymbolTable() : parent(nullptr) {}
    
    SymbolTable(SymbolTable<T>* p) : parent(p) {}
    
    ~SymbolTable() {}
    
    // 检查当前作用域是否声明了指定符号
    bool declares(const std::string& key) const {
        return tab.find(key) != tab.end();
    }
    
    // 递归检查是否声明了指定符号（包括父作用域）
    bool declaresRecursive(const std::string& key) const {
        if (declares(key)) return true;
        return parent && parent->declaresRecursive(key);
    }
    
    // 在当前作用域添加符号
    void put(const std::string& key, const T& elm) {
        tab[key] = elm;
    }
    
    // 从当前作用域获取符号
    T* get(const std::string& key) {
        auto it = tab.find(key);
        return (it != tab.end()) ? &(it->second) : nullptr;
    }
    
    // 递归获取符号（包括父作用域）
    T* getRecursive(const std::string& key) {
        if (declares(key)) {
            return get(key);
        }
        return parent ? parent->getRecursive(key) : nullptr;
    }
    
    // 获取当前作用域的所有符号
    std::map<std::string, T> getAll() const {
        return tab;
    }
    
    // 获取父作用域
    SymbolTable<T>* getParent() const {
        return parent;
    }
    
    // 设置父作用域
    void setParent(SymbolTable<T>* p) {
        parent = p;
    }
    
    // 清空当前作用域
    void clear() {
        tab.clear();
    }
    
    // 获取当前作用域符号数量
    size_t size() const {
        return tab.size();
    }
    
    // 检查当前作用域是否为空
    bool empty() const {
        return tab.empty();
    }
    
    // 打印符号表内容（调试用）
    void print(int level = 0) const {
        std::string indent(level * 2, ' ');
        std::cout << indent << "SymbolTable (level " << level << "):" << std::endl;
        for (const auto& pair : tab) {
            std::cout << indent << "  " << pair.first << " -> " << pair.second.toString() << std::endl;
        }
        if (parent) {
            parent->print(level + 1);
        }
    }
};
#endif