#ifndef STORE_HPP
#define STORE_HPP
#include <string>
#include <vector>
#include "registers.hpp"

class Storage {
public:
    virtual ~Storage() = default; 
    virtual std::string toString() = 0;
};

class StackStorage : public Storage {
private:
    /// @brief 相对于FP的offset，+8表示存储了FP和RA。
    int offset;
public:
    StackStorage(int offset) : offset(offset) {}
    int getOffset() { return offset; }
    std::string toString() {
        if (offset < 0) 
            return "[in fp" + std::to_string(offset) +"]";
        else 
            return "[in fp+" + std::to_string(offset) +"]";
    }
};

class RegStorage : public Storage {
private:
    Register* reg;
public:
    RegStorage(Register* reg) : reg(reg) {}
    Register* getReg() { return reg; }
    std::string toString() {
        return "[in " + reg->toString() + "]";
    }
};

class StaticStorage : public Storage {
public:
    StaticStorage() {}
    std::string toString() {
        return "[in static area]";
    }
};



#endif