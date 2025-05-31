#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <sstream>

/// @brief 错误类
class Error {
    private:
        /// @brief 错误类型：词法错误、语法错误、语义错误
        std::string type;
        /// @brief 错误位置
        size_t position[4];
        /// @brief 错误信息
        std::string errMsg;
    public:
        /// @brief 构造函数
        /// @param type 错误类型
        /// @param position 错误位置
        /// @param errMsg 错误信息
        Error(std::string type, size_t position[], std::string errMsg) : type(type), errMsg(errMsg) {
            this->position[0] = position[0];
            this->position[1] = position[1];
            this->position[2] = position[2];
            this->position[3] = position[3];
        }

        /// @brief 将错误转换为string
        /// @return string
        std::string toString() {
            std::ostringstream oss;
            oss << "error:" << position[0] << ":" << position[1] << ":" << position[2] << ":" << position[3] << ":" << type << " error " << errMsg << ".";
            return oss.str();
        }
};
#endif