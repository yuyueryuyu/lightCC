#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <string>
#include <assert.h>

class Register {
public:
    virtual ~Register() = default;
    virtual std::string toString() const = 0;
};

class RegInt : public Register {
private:
    std::string name;
public:
    RegInt(const std::string& n) : name(n) {}
    std::string toString() const override { return name; }
};

class RegFloat : public Register {
private:
    std::string name;
public:
    RegFloat(const std::string& n) : name(n) {}
    std::string toString() const override { return name; }
};

inline RegInt ZERO("zero"), RA("ra"), SP("sp"), GP("gp"), TP("tp"),
                     T0("t0"), T1("t1"), T2("t2"), FP("fp"), S1("s1"),
                     A0("a0"), A1("a1"), A2("a2"), A3("a3"), A4("a4"), 
                     A5("a5"), A6("a6"), A7("a7"), S2("s2"), S3("s3"),
                     S4("s4"), S5("s5"), S6("s6"), S7("s7"), S8("s8"),
                     S9("s9"), S10("s10"), S11("s11"), 
                     T3("t3"), T4("t4"), T5("t5"), T6("t6");

inline RegFloat FT0("ft0"), FT1("ft1"), FT2("ft2"), FT3("ft3"),
                       FT4("ft4"), FT5("ft5"), FT6("ft6"), FT7("ft7"),
                       FS0("fs0"), FS1("fs1"), FA0("fa0"), FA1("fa1"),
                       FA2("fa2"), FA3("fa3"), FA4("fa4"), FA5("fa5"),
                       FA6("fa6"), FA7("fa7"), FS2("fs2"), FS3("fs3"),
                       FS4("fs4"), FS5("fs5"), FS6("fs6"), FS7("fs7"),
                       FS8("fs8"), FS9("fs9"), FS10("fs10"), FS11("fs11"),
                       FT8("ft8"), FT9("ft9"), FT10("ft10"), FT11("ft11");

inline RegInt* getA(int i) {
    switch(i) {
        case 0: return &A0;
        case 1: return &A1;
        case 2: return &A2;
        case 3: return &A3;
        case 4: return &A4;
        case 5: return &A5;
        case 6: return &A6;
        case 7: return &A7;
        default: return nullptr;
    }
}

inline RegInt* getT(int i) {
    switch(i) {
        case 0: return &T0;
        case 1: return &T1;
        case 2: return &T2;
        case 3: return &T3;
        case 4: return &T4;
        case 5: return &T5;
        case 6: return &T6;
        default: return nullptr;
    }
}

inline RegFloat* getFT(int i) {
    switch(i) {
        case 0: return &FT0;
        case 1: return &FT1;
        case 2: return &FT2;
        case 3: return &FT3;
        case 4: return &FT4;
        case 5: return &FT5;
        case 6: return &FT6;
        case 7: return &FT7;
        case 8: return &FT8;
        case 9: return &FT9;
        case 10: return &FT10;
        case 11: return &FT11;
        default: return nullptr;
    }
}

inline RegFloat* getFA(int i) {
    switch(i) {
        case 0: return &FA0;
        case 1: return &FA1;
        case 2: return &FA2;
        case 3: return &FA3;
        case 4: return &FA4;
        case 5: return &FA5;
        case 6: return &FA6;
        case 7: return &FA7;
        default: return nullptr;
    }
}

#endif