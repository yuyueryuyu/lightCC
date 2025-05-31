#include "Parser.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <fstream>
#include <iomanip>

using namespace std;

void LRParser::parseGrammar(const vector<string>& input) {
    for (const string& line : input) {
        size_t arrow_pos = line.find("->");
        if (arrow_pos == string::npos) continue;
        
        string left = line.substr(0, arrow_pos);
        left = trim(left);
        non_terminals.insert(left);
        
        if (start_symbol.empty()) {
            start_symbol = left;
        }
    }

    int prod_id = 0;  // 为每个产生式分配唯一编号
    for (const string& line : input) {    
        size_t arrow_pos = line.find("->");
        if (arrow_pos == string::npos) continue;

        string left = line.substr(0, arrow_pos);
        left = trim(left);

        string right_side = line.substr(arrow_pos + 2);
        vector<string> alternatives = split(right_side, "|");
        
        for (const string& alt : alternatives) {
            Production prod;
            prod.left = left;
            prod.id = prod_id++;
            
            vector<string> symbols = tokenize(trim(alt));
            for (const string& symbol : symbols) {
                prod.right.push_back(symbol);
                if (non_terminals.find(symbol) == non_terminals.end() && symbol != "ε") {
                    terminals.insert(symbol);
                }
            }
            
            // 如果右部为ε，则将其转换为空向量
            if (prod.right.size() == 1 && prod.right[0] == "ε") {
                prod.right.clear();
            }
            
            productions.push_back(prod);
        }
    }
    
    // 添加终结符 EOF 表示输入结束
    terminals.insert("EOF");
}

// 计算闭包
set<Item> LRParser::closure(const set<Item>& items) {
    set<Item> result = items;
    bool changed = true;
    
    while (changed) {
        changed = false;
        set<Item> new_items = result;
        
        for (const Item& item : result) {
            // 如果点在产生式右部末尾，则跳过
            if (item.dot_position >= item.production.right.size()) {
                continue;
            }
            
            // 获取点后面的符号
            string next_symbol = item.production.right[item.dot_position];
            
            // 如果点后面的符号是非终结符
            if (non_terminals.find(next_symbol) != non_terminals.end()) {
                
                for (const Production& prod : productions) {
                    if (prod.left == next_symbol) {
                        
                        // 创建新项
                        Item new_item;
                        new_item.production = prod;
                        new_item.dot_position = 0;

                        // 添加到结果集中
                        if (new_items.insert(new_item).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
        
        result = new_items;
    }
    
    return result;
}

// 计算GOTO函数
set<Item> LRParser::computeGoto(const set<Item>& items, const string& symbol) {
    set<Item> result;
    
    for (const Item& item : items) {
        // 如果点在产生式右部末尾，则跳过
        if (item.dot_position >= item.production.right.size()) {
            continue;
        }
        
        // 如果点后面的符号与给定符号相同
        if (item.production.right[item.dot_position] == symbol) {
            // 创建新项，将点右移一位
            Item new_item;
            new_item.production = item.production;
            new_item.dot_position = item.dot_position+1;
            result.insert(new_item);
        }
    }

    // 计算闭包
    return closure(result);
}

// 构建规范LR(0)项集族
void LRParser::buildCanonicalCollection() {
    // 创建初始项集I0
    set<Item> initial_items;
    Item initial_item;
    initial_item.production = productions[0];
    initial_item.dot_position = 0;
    initial_items.insert(initial_item);
    
    // 计算I0的闭包
    set<Item> initial_closure = closure(initial_items);
    
    // 创建初始项集
    ItemSet initial_set;
    initial_set.items = initial_closure;
    canonical_collection.push_back(initial_set);
    
    // 使用BFS算法构建项集族
    queue<int> queue;
    queue.push(0);  // 从I0开始
    while (!queue.empty()) {
        int current_index = queue.front();
        queue.pop();
        
        // 收集所有可能的转移符号
        set<string> symbols;
        {
            const ItemSet& current_set_for_symbols = canonical_collection[current_index];
            for (const Item& item : current_set_for_symbols.items) {
                if (item.dot_position < item.production.right.size()) {
                    symbols.insert(item.production.right[item.dot_position]);
                }
            }
        }
        
        // 对每个符号计算GOTO
        for (const string& symbol : symbols) {
            
            set<Item> goto_result = computeGoto(canonical_collection[current_index].items, symbol);
            // 如果结果非空
            if (!goto_result.empty()) {
                // 检查是否已经存在相同的项集
                int existing_index = -1;
                for (size_t i = 0; i < canonical_collection.size(); i++) {
                    if (canonical_collection[i].items == goto_result) {
                        existing_index = i;
                        break;
                    }
                }
                
                if (existing_index == -1) {
                    // 如果不存在，则添加新项集
                    ItemSet new_set;
                    new_set.items = goto_result;
                    canonical_collection.push_back(new_set);
                    existing_index = canonical_collection.size() - 1;
                    queue.push(existing_index);
                }
                // 添加转移
                canonical_collection[current_index].goto_transitions[symbol] = existing_index;
            }
        }
    }
}

// 计算FIRST集合
void LRParser::computeFirstSets() {
    bool changed = true;
    
    // 为每个终结符初始化FIRST集合
    for (const string& terminal : terminals) {
        first_sets[terminal].insert(terminal);
    }
    
    // 为每个非终结符初始化空的FIRST集合
    for (const string& non_terminal : non_terminals) {
        first_sets[non_terminal] = set<string>();
    }
    
    // 重复计算直到不再发生变化
    while (changed) {
        changed = false;
        
        for (const Production& production : productions) {
            const string& left = production.left;
            const vector<string>& right = production.right;
            
            // 如果是空产生式 A -> ε
            if (right.empty()) {
                if (first_sets[left].insert("ε").second) {
                    changed = true;
                }
                continue;
            }
            
            // 计算产生式右部的FIRST集合
            set<string> right_first;
            bool contains_epsilon = true;
            
            for (const string& symbol : right) {
                // 如果当前符号的FIRST集合不包含ε，则后续符号不再考虑
                if (!contains_epsilon) break;
                
                // 如果当前符号是终结符
                if (terminals.find(symbol) != terminals.end()) {
                    right_first.insert(symbol);
                    contains_epsilon = false;
                } else {
                    // 如果当前符号是非终结符
                    const set<string>& symbol_first = first_sets[symbol];
                    // 添加除ε以外的所有符号
                    for (const string& terminal : symbol_first) {
                        if (terminal != "ε") {
                            right_first.insert(terminal);
                        }
                    }
                    // 检查是否包含ε
                    contains_epsilon = symbol_first.find("ε") != symbol_first.end();
                }
            }
            
            // 如果所有右部符号都可以推导出ε，则将ε添加到FIRST(left)
            if (contains_epsilon) {
                right_first.insert("ε");
            }
            
            // 更新FIRST(left)
            for (const string& terminal : right_first) {
                if (first_sets[left].insert(terminal).second) {
                    changed = true;
                }
            }
        }
    }
}

// 计算FOLLOW集合
void LRParser::computeFollowSets() {
    // 为每个非终结符初始化空的FOLLOW集合
    for (const string& non_terminal : non_terminals) {
        follow_sets[non_terminal] = set<string>();
    }
    
    // 将EOF添加到开始符号的FOLLOW集合中
    follow_sets[start_symbol].insert("EOF");
    
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const Production& production : productions) {
            const string& left = production.left;
            const vector<string>& right = production.right;
            
            // 遍历右部的每个符号
            for (size_t i = 0; i < right.size(); i++) {
                const string& current = right[i];
                
                // 只处理非终结符
                if (non_terminals.find(current) == non_terminals.end()) {
                    continue;
                }
                
                // 如果current是右部最后一个符号，或者current后面的所有符号都可以推导出ε
                bool is_last_or_nullable = true;
                if (i == right.size() - 1) {
                    // 如果current是右部最后一个符号，则将FOLLOW(left)添加到FOLLOW(current)
                    for (const string& symbol : follow_sets[left]) {
                        if (follow_sets[current].insert(symbol).second) {
                            changed = true;
                        }
                    }
                } else {
                    // 考虑current后面的符号
                    set<string> trailer_first;
                    bool all_nullable = true;
                    
                    for (size_t j = i + 1; j < right.size(); j++) {
                        const string& next = right[j];
                        
                        // 如果next是终结符
                        if (terminals.find(next) != terminals.end()) {
                            trailer_first.insert(next);
                            all_nullable = false;
                            break;
                        } else {
                            // 如果next是非终结符
                            const set<string>& next_first = first_sets[next];
                            
                            // 添加除ε以外的所有符号
                            for (const string& terminal : next_first) {
                                if (terminal != "ε") {
                                    trailer_first.insert(terminal);
                                }
                            }
                            
                            // 如果next不可以推导出ε，则停止
                            if (next_first.find("ε") == next_first.end()) {
                                all_nullable = false;
                                break;
                            }
                        }
                    }
                    
                    // 将计算得到的FIRST集合添加到FOLLOW(current)
                    for (const string& symbol : trailer_first) {
                        if (follow_sets[current].insert(symbol).second) {
                            changed = true;
                        }
                    }
                    
                    // 如果后面的所有符号都可以推导出ε，则将FOLLOW(left)添加到FOLLOW(current)
                    if (all_nullable) {
                        for (const string& symbol : follow_sets[left]) {
                            if (follow_sets[current].insert(symbol).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

// 构建SLR(1)分析表
void LRParser::buildSLRTable() {
    has_conflicts = false;
    int state_count = canonical_collection.size();
    
    // 初始化ACTION表和GOTO表
    action_table.resize(state_count, vector<ActionEntry>(terminals.size(), ActionEntry()));
    goto_table.resize(state_count);
    
    // 创建终结符到索引的映射
    map<string, int> terminal_indices;
    int idx = 0;
    for (const string& terminal : terminals) {
        terminal_indices[terminal] = idx++;
    }
    
    // 遍历每个状态
    for (int state = 0; state < state_count; state++) {
        const ItemSet& item_set = canonical_collection[state];
        
        // 处理移进和接受操作
        for (const auto& transition : item_set.goto_transitions) {
            const string& symbol = transition.first;
            int next_state = transition.second;
            
            // 如果是终结符，则添加移进操作
            if (terminals.find(symbol) != terminals.end()) {
                int col = terminal_indices[symbol];
                
                // 检查是否已经有操作
                if (action_table[state][col].type == REDUCE) {
                    //cout << "冲突：状态 " << state << " 在输入 " << symbol << " 上有S/R冲突，自动移进解决" << endl;
                    action_table[state][col] = ActionEntry(SHIFT, next_state);
                    continue;
                }
                if (action_table[state][col].type != ERR) {
                    has_conflicts = true;
                    //cout << "冲突：状态 " << state << " 在输入 " << symbol << " 上有多个操作" << endl;
                }
                
                action_table[state][col] = ActionEntry(SHIFT, next_state);
            }
            // 如果是非终结符，则添加GOTO操作
            else if (non_terminals.find(symbol) != non_terminals.end()) {
                goto_table[state][symbol] = next_state;
            }
        }
        
        // 处理归约和接受操作
        for (const Item& item : item_set.items) {
            // 如果点在产生式右部末尾，则添加归约操作
            if (item.dot_position == item.production.right.size()) {
                // 特殊情况：如果是增广文法的起始产生式，则添加接受操作
                if (item.production.left == start_symbol && item.production.id == 0) {
                    int col = terminal_indices["EOF"];
                    
                    // 检查是否已经有操作
                    if (action_table[state][col].type != ERR) {
                        has_conflicts = true;
                        //cout << "冲突：状态 " << state << " 在输入 EOF 上有多个操作" << endl;
                    }
                    
                    action_table[state][col] = ActionEntry(ACCEPT, item.production.id);
                } else {
                    // 对于每个在FOLLOW(item.production.left)中的终结符，添加归约操作
                    for (const string& terminal : follow_sets[item.production.left]) {
                        if (terminals.find(terminal) != terminals.end()) {
                            int col = terminal_indices[terminal];
                            if (action_table[state][col].type == SHIFT) {
                                //cout << "冲突：状态 " << state << " 在输入 " << terminal << " 上S/R冲突, 默认进行移进操作解决" << endl;
                                continue;
                            }
                            // 检查是否已经有操作
                            if (action_table[state][col].type != ERR) {
                                has_conflicts = true;
                                //cout << "冲突：状态 " << state << " 在输入 " << terminal << " 上有多个操作" << endl;
                            }
                            
                            action_table[state][col] = ActionEntry(REDUCE, item.production.id);
                        }
                    }
                }
            }
        }
    }
}

// 辅助函数：修剪字符串两端的空白
string LRParser::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// 辅助函数：按分隔符分割字符串
vector<string> LRParser::split(const string& str, const string& delimiter) {
    vector<string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != string::npos) {
        result.push_back(trim(str.substr(start, end - start)));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    result.push_back(trim(str.substr(start)));
    return result;
}

// 辅助函数：将字符串分割成词法单元
vector<string> LRParser::tokenize(const string& str) {
    vector<string> tokens;
    istringstream iss(str);
    string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// 打印项
string LRParser::itemToString(const Item& item) const {
    string result = item.production.left + " → ";
    
    for (size_t i = 0; i < item.production.right.size(); i++) {
        if (i == item.dot_position) {
            result += "· ";
        }
        result += item.production.right[i] + " ";
    }
    
    if (item.dot_position == item.production.right.size()) {
        result += "·";
    }
    
    return result;
}
// 解析输入并构建SLR(1)分析表
void LRParser::buildParser(const vector<string>& input) {
    parseGrammar(input);
    buildCanonicalCollection();
    computeFirstSets();
    computeFollowSets();
    buildSLRTable();
}

// 打印项集族
void LRParser::printCanonicalCollection() const {
    for (size_t i = 0; i < canonical_collection.size(); i++) {
        cout << "I" << i << ":" << endl;
        cout << "  Kernel:" << endl;
        
        // 打印核心项
        for (const Item& item : canonical_collection[i].items) {
            // 核心项是初始项或者点不在最左边的项
            cout << "    " << itemToString(item) << endl;
        }
        
        cout << "  Goto:" << endl;
        if (canonical_collection[i].goto_transitions.empty()) {
            cout << "    (无)" << endl;
        } else {
            for (const auto& transition : canonical_collection[i].goto_transitions) {
                cout << "    " << transition.first << " → I" << transition.second << endl;
            }
        }
        
        cout << "----------------" << endl;
    }
}

// 打印FIRST集合
void LRParser::printFirstSets() const {
    cout << "FIRST集合:" << endl;
    for (const string& non_terminal : non_terminals) {
        cout << "FIRST(" << non_terminal << ") = { ";
        const set<string>& first = first_sets.at(non_terminal);
        bool first_item = true;
        for (const string& symbol : first) {
            if (!first_item) cout << ", ";
            cout << symbol;
            first_item = false;
        }
        cout << " }" << endl;
    }
    cout << "----------------" << endl;
}

// 打印FOLLOW集合
void LRParser::printFollowSets() const {
    cout << "FOLLOW集合:" << endl;
    for (const string& non_terminal : non_terminals) {
        cout << "FOLLOW(" << non_terminal << ") = { ";
        const set<string>& follow = follow_sets.at(non_terminal);
        bool first_item = true;
        for (const string& symbol : follow) {
            if (!first_item) cout << ", ";
            cout << symbol;
            first_item = false;
        }
        cout << " }" << endl;
    }
    cout << "----------------" << endl;
}

// 打印SLR(1)分析表
void LRParser::printSLRTable() const {
    cout << "SLR(1)分析表:" << endl;
    
    // 创建终结符列表和非终结符列表，用于表头
    vector<string> terminal_list;
    vector<string> non_terminal_list;
    
    for (const string& terminal : terminals) {
        terminal_list.push_back(terminal);
    }
    
    for (const string& non_terminal : non_terminals) {
        non_terminal_list.push_back(non_terminal);
    }
    
    // 打印表头
    cout << setw(8) << "状态";
    
    // 打印ACTION表部分的表头
    for (const string& terminal : terminal_list) {
        cout << " | " << setw(8) << terminal;
    }
    
    // 打印GOTO表部分的表头
    for (const string& non_terminal : non_terminal_list) {
        cout << " | " << setw(8) << non_terminal;
    }
    
    cout << endl;
    
    // 打印分隔线
    cout << setfill('-') << setw(8) << "";
    for (size_t i = 0; i < terminal_list.size() + non_terminal_list.size(); i++) {
        cout << "-|" << setw(8) << "";
    }
    cout << setfill(' ') << endl;
    
    // 打印表内容
    for (size_t state = 0; state < action_table.size(); state++) {
        cout << setw(8) << state;
        
        // 打印ACTION表部分
        for (size_t i = 0; i < terminal_list.size(); i++) {
            string action_str;
            const ActionEntry& entry = action_table[state][i];
            
            if (entry.type == SHIFT) {
                action_str = "s" + to_string(entry.value);
            } else if (entry.type == REDUCE) {
                action_str = "r" + to_string(entry.value);
            } else if (entry.type == ACCEPT) {
                action_str = "acc" + to_string(entry.value);
            } else {
                action_str = "";
            }
            
            cout << " | " << setw(8) << action_str;
        }
        
        // 打印GOTO表部分
        for (const string& non_terminal : non_terminal_list) {
            auto it = goto_table[state].find(non_terminal);
            string goto_str = (it != goto_table[state].end()) ? to_string(it->second) : "";
            
            cout << " | " << setw(8) << goto_str;
        }
        
        cout << endl;
    }

    if (has_conflicts) {
        cout << "\n警告：分析表中存在冲突，这可能导致SLR(1)分析器无法正确工作。" << endl;
    }
}

// 将SLR(1)分析表输出到CSV文件
void LRParser::exportSLRTableToCSV(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "无法打开文件: " << filename << endl;
        return;
    }
    
    // 创建终结符列表和非终结符列表，用于表头
    vector<string> terminal_list;
    vector<string> non_terminal_list;
    
    for (const string& terminal : terminals) {
        terminal_list.push_back(terminal);
    }
    
    for (const string& non_terminal : non_terminals) {
        non_terminal_list.push_back(non_terminal);
    }
    
    // 写入表头
    file << "状态";
    
    // 写入ACTION表部分的表头
    for (const string& terminal : terminal_list) {
        file << "," << terminal;
    }
    
    // 写入GOTO表部分的表头
    for (const string& non_terminal : non_terminal_list) {
        file << "," << non_terminal;
    }
    
    file << endl;
    
    // 写入表内容
    for (size_t state = 0; state < action_table.size(); state++) {
        file << state;
        
        // 写入ACTION表部分
        for (size_t i = 0; i < terminal_list.size(); i++) {
            string action_str;
            const ActionEntry& entry = action_table[state][i];
            
            if (entry.type == SHIFT) {
                action_str = "s" + to_string(entry.value);
            } else if (entry.type == REDUCE) {
                action_str = "r" + to_string(entry.value);
            } else if (entry.type == ACCEPT) {
                action_str = "acc" + to_string(entry.value);
            } else {
                action_str = "";
            }
            
            file << "," << action_str;
        }
        
        // 写入GOTO表部分
        for (const string& non_terminal : non_terminal_list) {
            auto it = goto_table[state].find(non_terminal);
            string goto_str = (it != goto_table[state].end()) ? to_string(it->second) : "";
            
            file << "," << goto_str;
        }
        
        file << endl;
    }
    
    file.close();
}

// 打印所有产生式
void LRParser::printProductions() const {
    cout << "产生式列表:" << endl;
    for (const Production& prod : productions) {
        cout << "[" << prod.id << "] " << prod.left << " → ";
        if (prod.right.empty()) {
            cout << "ε";
        } else {
            for (const string& symbol : prod.right) {
                cout << symbol << " ";
            }
        }
        cout << endl;
    }
    cout << "----------------" << endl;
}

ParseTreeNode* LRParser::parseTokens(const vector<Token>& tokens, bool check) {
    // 清理之前的解析树
    if (parse_tree_root) {
        delete parse_tree_root;
        parse_tree_root = nullptr;
    }
    
    // 检查分析表是否已构建
    if (action_table.empty()) {
        if (!check)
            cout << "分析表尚未构建！" << endl;
        return nullptr;
    }
    
    // 初始化解析栈和输入缓冲区
    vector<int> state_stack;                    // 状态栈
    vector<ParseTreeNode*> symbol_stack;       // 符号栈（存储解析树节点）
    vector<Token> input_buffer = tokens;      // 输入缓冲区
    
    // 初始状态
    state_stack.push_back(0);
    int input_index = 0;
    
    // 创建终结符到索引的映射
    map<string, int> terminal_indices;
    int idx = 0;
    for (const string& terminal : terminals) {
        terminal_indices[terminal] = idx++;
    }
    if (!check)
        cout << "开始解析..." << endl;
    bool panick = false;
    while (true) {
        // 获取当前状态和输入符号
        int current_state = state_stack.back();
        Token current_input = input_buffer[input_index];
        if (!check)
            cout << "状态: " << current_state << ", 输入: " << current_input.toString();
        
        // 检查输入符号是否在分析表中
        if (terminal_indices.find(current_input.getId()) == terminal_indices.end()) {
            if (!check)
                cout << " -> 错误：未知的输入符号 " << current_input.getId() << endl;
            err(current_input, "Unknown input token: " + current_input.getId());
            input_index++;
            continue;
        }
        
        // 查找ACTION表中的操作
        int terminal_idx = terminal_indices[current_input.getId()];
        const ActionEntry& action = action_table[current_state][terminal_idx];
        
        if (action.type == SHIFT) {
            // 移进操作
            if (!check) cout << " -> 移进到状态 " << action.value << endl;
            
            // 将输入符号移进栈中
            state_stack.push_back(action.value);
            panick = false;
            
            // 创建终结符节点并压入符号栈
            ParseTreeNode* terminal_node = new ParseTreeNode(current_input.getId(), current_input);
            symbol_stack.push_back(terminal_node);
            
            // 移动输入指针
            if (current_input.getId() == "EOF") return nullptr;
            input_index++;
            
        } else if (action.type == REDUCE) {
            // 归约操作
            const Production& reduction = productions[action.value];
            if (!check) cout << " -> 用产生式 " << action.value << " 归约: " << reduction.left << " -> ";
            if (reduction.right.empty()) {
                //cout << "ε";
            } else {
                for (const string& sym : reduction.right) {
                   if (!check) cout << sym << " ";
                }
            }
            if (!check)cout << endl;
            
            // 创建非终结符节点
            ParseTreeNode* non_terminal_node;
            
            // 弹出相应数量的状态和符号
            int pop_count = reduction.right.size();
            
            // 处理空产生式的情况
            if (reduction.right.empty()) {
                // 对于空产生式，创建一个ε节点
                non_terminal_node = new ParseTreeNode(reduction.left, Token(), Token());
            } else {
                // 收集归约涉及的符号节点（它们将成为新节点的子节点）
                vector<ParseTreeNode*> reduction_nodes;
                for (int i = 0; i < pop_count; i++) {
                    state_stack.pop_back();
                    reduction_nodes.push_back(symbol_stack.back());
                    symbol_stack.pop_back();
                }
                
                // 由于栈是后进先出，需要反转子节点顺序
                reverse(reduction_nodes.begin(), reduction_nodes.end());
                non_terminal_node = new ParseTreeNode(reduction.left, reduction_nodes[0]->start, reduction_nodes[reduction_nodes.size()-1]->end);
                non_terminal_node->children = reduction_nodes;
            }
            
            // 压入新的非终结符节点
            symbol_stack.push_back(non_terminal_node);
            panick = false;
            
            // 查找GOTO表确定新状态
            int new_state = state_stack.back();
            if (goto_table[new_state].find(reduction.left) != goto_table[new_state].end()) {
                state_stack.push_back(goto_table[new_state][reduction.left]);
            } else {
                if (!check) cout << "错误：GOTO表中找不到对应项 (" << new_state << ", " << reduction.left << ")" << endl;
                err(current_input, "Unknown Reduce Item near: " + current_input.getId());
                continue;
            }
            
        } else if (action.type == ACCEPT) {
            // 接受操作
            // 归约操作
            const Production& reduction = productions[action.value];
            if (!check) cout << " -> 用产生式 " << action.value << " 归约: " << reduction.left << " -> ";
            if (reduction.right.empty()) {
                if (!check) cout << "ε";
            } else {
                for (const string& sym : reduction.right) {
                    if (!check) cout << sym << " ";
                }
            }
            if (!check) cout << endl;
            
            // 创建非终结符节点
            ParseTreeNode* non_terminal_node;
            
            // 弹出相应数量的状态和符号
            int pop_count = reduction.right.size();
            
            // 处理空产生式的情况
            if (reduction.right.empty()) {
                // 对于空产生式，创建一个ε节点
                non_terminal_node = new ParseTreeNode(reduction.left, Token(), Token());
            } else {
                // 收集归约涉及的符号节点（它们将成为新节点的子节点）
                vector<ParseTreeNode*> reduction_nodes;
                for (int i = 0; i < pop_count; i++) {
                    state_stack.pop_back();
                    reduction_nodes.push_back(symbol_stack.back());
                    symbol_stack.pop_back();
                }
                
                // 由于栈是后进先出，需要反转子节点顺序
                reverse(reduction_nodes.begin(), reduction_nodes.end());
                non_terminal_node = new ParseTreeNode(reduction.left, reduction_nodes[0]->start, reduction_nodes[reduction_nodes.size()-1]->end);
                non_terminal_node->children = reduction_nodes;
            }
            
            // 压入新的非终结符节点
            symbol_stack.push_back(non_terminal_node);
            panick = false;
            if (!check) cout << " -> 接受！解析成功。" << endl;
            
            // 解析成功，返回解析树根节点
            if (!symbol_stack.empty()) {
                parse_tree_root = symbol_stack.back();
                return parse_tree_root;
            } else {
                if (!check) cout << "错误：符号栈为空，无法获取解析树根节点。" << endl;
                err(current_input, "Stack is null.");
                return nullptr;
            }
            
        } else {
            // 错误
            if (!check) cout << " -> 错误：无效操作" << endl;
            if (!panick)
                err(current_input, "near "+ current_input.getId() + ".");
            panick = true;
            if (current_input.getId() == "EOF") return nullptr;
            input_index++;
            continue;
        }
        
        if (!check) cout << endl;
    }
}

// 打印解析树
void LRParser::printParseTree() const {
    if (parse_tree_root) {
        parse_tree_root->print();
    } else {
        //cout << "解析失败!" << endl;
    }
}

// 将解析树导出为JSON文件
void LRParser::exportParseTreeToJSON(const string& filename) const {
    if (!parse_tree_root) {
        cout << "解析树为空，无法导出。" << endl;
        return;
    }
    
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "无法打开文件: " << filename << endl;
        return;
    }
    
    file << parse_tree_root->toJSON() << endl;
    file.close();
}

void LRParser::printErrors() {
    for (auto e : errors) {
        cout << e.toString() << endl;
    }
}

void LRParser::outputErrors(string file) {
    ofstream f(file);
    if (!f.is_open()) {
        cerr << "无法打开文件: " << file << endl;
        return;
    }
    for (auto e : errors) {
        f << e.toString() << endl;
    }
    f.close();
}