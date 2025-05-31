#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

/// @brief 词法单元
class Token {
private:
    /// @brief 源程序的位置
    size_t position[4];
    /// @brief 词法单元记号
    std::string id;
    /// @brief 对应的字符串
    std::string value;

public:
    /// @brief 构造函数
    Token() {
        this->position[0] = 0;
        this->position[1] = 0;
        this->position[2] = 0;
        this->position[3] = 0;
        this->id = "";
        this->value = "";
    }

    /// @brief 构造函数 
    /// @param position 位置
    /// @param id 记号
    /// @param value 字符串
    Token(size_t position[], std::string id, std::string value) {
        this->position[0] = position[0];
        this->position[1] = position[1];
        this->position[2] = position[2];
        this->position[3] = position[3];
        this->id = id;
        this->value = value;
    }

    /// @brief 转换为字符串
    /// @return 
    std::string toString() {
        return "(" + id + ", " + value + ")";
    }

    /// @brief 获取记号
    /// @return 
    std::string getId() {
        return id;
    }

    /// @brief 获取实际值
    /// @return 
    std::string getValue() {
        return value;
    }

    /// @brief 获取位置
    /// @return 
    size_t* getPos() {
        return position;
    }
};

#endif