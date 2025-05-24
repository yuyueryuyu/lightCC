#include "parserule.hpp"
#include <iostream>

bool Production::operator==(const Production& other) const {
    return left == other.left && right == other.right;
}

bool Production::operator<(const Production& other) const {
    if (left != other.left) return left < other.left;
    return right < other.right;
}

void Production::print() {
    std::cout << left << " -> ";
    for (std::string str : right) {
        std::cout << str;
    }
    std::cout << std::endl;
}