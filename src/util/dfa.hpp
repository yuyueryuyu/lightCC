#ifndef DFA_HPP
#define DFA_HPP

#include <map>
#include <set>
#include <string>

/// @brief DFA
class DFA {
private:
    /// @brief 状态集
    std::set<std::string> states;    
    /// @brief 开始状态
    std::string startState; 
    /// @brief 接受状态集及其对应的token
    std::map<std::string, std::string> acceptStates;    
    /// @brief 状态转换表
    std::map<std::pair<std::string, std::string>, std::string> transitions;
public:
    /// @brief 读入文件构造DFA
    /// @param filename 文件名
    DFA(const std::string& filename);

    /// @brief 检查DFA是否合法
    /// @return 
    bool validate();

    /// @brief 使用DFA识别字符串
    /// @param input 字符串
    /// @return 一个pair，表示接受的Token及其接受字符串的长度
    std::pair<std::string, size_t> recognizeString(const std::string& input);
};

/// @brief 识别文件输入
/// @param filename 文件名
/// @param dfa 
void recognizeFile(const std::string& filename, DFA dfa);

/// @brief 识别命令行输入
/// @param dfa 
void recognizeCin(DFA dfa);

#endif