#include "parsetree.hpp"
#include <iostream>

/// @brief 打印解析树
/// @param depth 缩进
void ParseTreeNode::print(int depth) const {
    std::string indent(depth * 2, ' ');
    if (is_terminal) {
        std::cout << indent << symbol;
        if (!token_value.empty() && token_value != symbol) {
            std::cout << " (" << token_value << ")";
        }
        std::cout << std::endl;
    } else {
        std::cout << indent << symbol << " ->" << std::endl;
        for (ParseTreeNode* child : children) {
            child->print(depth + 1);
        }
    }
}

/// @brief 转换为JSON字符串
/// @param depth 缩进深度
/// @return 
std::string ParseTreeNode::toJSON(int depth) const {
    std::string indent(depth * 2, ' ');
    std::string result = indent + "{\n";
    result += indent + "  \"symbol\": \"" + symbol + "\",\n";
    result += indent + "  \"is_terminal\": " + (is_terminal ? "true" : "false") + ",\n";
    
    if (is_terminal && !token_value.empty()) {
        result += indent + "  \"value\": \"" + token_value + "\",\n";
    }
    
    if (!children.empty()) {
        result += indent + "  \"children\": [\n";
        for (size_t i = 0; i < children.size(); i++) {
            result += children[i]->toJSON(depth + 2);
            if (i < children.size() - 1) {
                result += ",";
            }
            result += "\n";
        }
        result += indent + "  ]\n";
    } else {
        result += indent + "  \"children\": []\n";
    }
    
    result += indent + "}";
    return result;
}
