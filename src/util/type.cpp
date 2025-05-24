#include "type.hpp"
#include "astnodes.hpp"

BType::BType(Type *t) {
    if (t == nullptr) name ="";
    else name = t->getName();
}