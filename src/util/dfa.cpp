#include <iostream>
#include <fstream>
#include <regex>
#include "dfa.hpp"

// 读取DFA五元组定义
DFA::DFA(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        exit(1);
    }
    
    // 读取状态集
    int statesSize;
    file >> statesSize;
    for (int i = 0; i < statesSize; i++) {
        std::string state;
        file >> state;
        states.insert(state);
    }
    
    // 读取开始状态
    file >> startState;
    
    // 读取接受状态集
    int acceptSize;
    file >> acceptSize;
    for (int i = 0; i < acceptSize; i++) {
        std::string state, token;
        file >> state >> token;
        acceptStates[state] = token;
    }
    
    // 读取状态转换表
    int transitionSize;
    file >> transitionSize;
    for (int i = 0; i < transitionSize; i++) {
        std::string fromState, toState;
        std::string reg_ex;
        file >> fromState >> reg_ex >> toState;
        transitions[{fromState, reg_ex}] = toState;
    }
    
    file.close();
}

bool DFA::validate() {
    // 检查开始状态是否唯一且包含在状态集中
    if (states.find(startState) == states.end()) {
        std::cerr << "错误: 开始状态不在状态集中" << std::endl;
        return false;
    }
    
    // 检查接受状态集是否为空
    if (acceptStates.empty()) {
        std::cerr << "错误: 接受状态集为空" << std::endl;
        return false;
    }
    
    // 检查所有接受状态是否包含在状态集中
    for (const auto& state : acceptStates) {
        if (states.find(state.first) == states.end()) {
            std::cerr << "错误: 接受状态 '" << state.first << "' 不在状态集中" << std::endl;
            return false;
        }
    }

    
    return true;
}

// 使用DFA验证字符串
std::pair<std::string, size_t> DFA::recognizeString(const std::string& input) {
    std::string currentState = startState;
    size_t i = 0;
    for (i = 0; i < input.length(); i++) {
        char c = input[i];
        std::string s(1, c);
        bool found = false;
        // 获取下一个状态
        for (const auto& p : transitions) {
            std::smatch result;
            std::regex rgx(p.first.second);
	        bool ret = regex_match(s, result, rgx);
            if (p.first.first == currentState && ret) {
                currentState = p.second;
                found = true;
                break;
            }
        }
        if (!found) break;
    }
    
    // 检查最终状态是否为接受状态
    bool isAccepted = acceptStates.find(currentState) != acceptStates.end();
    std::string token = isAccepted ? acceptStates.find(currentState)->second : "";
    return make_pair(token, i);
}

void recognizeFile(const std::string& filename, DFA dfa) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        exit(1);
    }
    std::string input = "";
    char ch;
    while (!file.eof()) {
        file.get(ch);
        input += ch;
    }

    for (size_t i = 0; i < input.size();) {
        auto p = dfa.recognizeString(input.substr(i));
        std::string token = p.first;
        size_t ori = i;
        i += p.second;
        if (token == "SKIP") {
            continue;
        }
        if (token == "") {
            std::cout << i << ";" << i << ": Lexer Error near \" " << input[i] << " \"."<< std::endl;
            i++;
            //cout << "(" << token << ", " << str << ")" << endl;
        } else {
            std::cout << "(" << token << ", " << input.substr(ori, p.second) << ")" << std::endl;
        }
    }
    
    file.close();
}

void recognizeCin(DFA dfa) {
    std::cout << "Please input: " << std::endl;
    std::string input = "";
    char ch;
    while (!std::cin.eof()) {
        std::cin.get(ch);
        input += ch;
    }
    for (size_t i = 0; i < input.size();) {
        auto p = dfa.recognizeString(input.substr(i));
        std::string token = p.first;
        size_t ori = i;
        i += p.second;
        if (token == "SKIP") {
            continue;
        }
        if (token == "") {
            std::cout << "Lexer Error."<< std::endl;
            i++;
            //cout << "(" << token << ", " << str << ")" << endl;
        } else {
            std::cout << "(" << token << ", " << input.substr(ori, p.second) << ")" << std::endl;
        }
    }
}