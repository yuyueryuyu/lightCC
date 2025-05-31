#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

#include <string> 
#include "ir.hpp"
#include "type.hpp"

class Directive {
public:
    virtual ~Directive() = default;
    virtual std::string toString() const = 0;
};

class Bss : public Directive {
public:
    Bss() {}
    virtual std::string toString() const { return ".bss"; }
};

class Text : public Directive {
public:
    Text() {}
    virtual std::string toString() const { return ".text"; }
};

class Align : public Directive {
private:
    int pow;
public:
    Align(int pow) : pow(pow) {}
    virtual std::string toString() const { return ".align " + std::to_string(pow); }
};

class Size : public Directive {
private:
    IRSym* sym;
public:
    Size(IRSym* sym) : sym(sym) {}
    virtual std::string toString() const { 
        if (auto ftype = dynamic_cast<FType*>(sym->getType())) {
            return ".size "+sym->getName()+", .-"+sym->getName();
        } else {
            return ".size "+sym->getName()+", "+std::to_string(sym->getType()->getSize());
        }
    }
};

class TypeDir : public Directive {
private:
    IRSym* sym;
public:
    TypeDir(IRSym* sym) : sym(sym) {}
    virtual std::string toString() const { 
        if (auto ftype = dynamic_cast<FType*>(sym->getType())) {
            return ".type "+sym->getName() +", @function";
        } else {
            return ".type "+sym->getName() +", @object";
        }
    }
};

class GloblDir : public Directive {
private:
    IRSym* sym;
public:
    GloblDir(IRSym* sym) : sym(sym) {}
    virtual std::string toString() const { return ".globl " + sym->getName(); }
};

#endif