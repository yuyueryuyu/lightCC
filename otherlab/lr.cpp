#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm>
#include <queue>
#include <fstream>

using namespace std;

// 表示一个产生式规则
struct Production {
    string left;            // 产生式左部
    vector<string> right;   // 产生式右部
    
    bool operator==(const Production& other) const {
        return left == other.left && right == other.right;
    }
    bool operator<(const Production& other) const {
        if (left != other.left) return left < other.left;
        return right < other.right;
    }

    void print() {
        cout << left << " -> ";
        for (string str : right) {
            cout << str;
        }
        cout << endl;
    }
};

// 表示一个项目
struct Item {
    Production production;  // 对应的产生式
    int dot_position;       // 点的位置
    
    bool operator==(const Item& other) const {
        return production == other.production && dot_position == other.dot_position;
    }
    bool operator<(const Item& other) const {
        if (!(production == other.production)) return production < other.production;
        return dot_position < other.dot_position;
    }
};
// 表示项集
struct ItemSet {
    set<Item> items;
    map<string, int> goto_transitions;  // 转移函数：符号 -> 项集编号
    
    bool operator==(const ItemSet& other) const {
        return items == other.items;
    }
};

class LRParser {
private:
    vector<Production> productions;        // 所有产生式
    string start_symbol;                   // 开始符号
    set<string> non_terminals;             // 非终结符集合
    set<string> terminals;                 // 终结符集合
    vector<ItemSet> canonical_collection;  // 规范LR(1)项集族

    // 解析输入文法
    void parseGrammar(const vector<string>& input) {
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
    }
    
    // 计算闭包
    set<Item> closure(const set<Item>& items) {
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
    set<Item> computeGoto(const set<Item>& items, const string& symbol) {
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
    
    // 构建规范LR(1)项集族
    void buildCanonicalCollection() {
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
    
    // 辅助函数：修剪字符串两端的空白
    string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
    
    // 辅助函数：按分隔符分割字符串
    vector<string> split(const string& str, const string& delimiter) {
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
    vector<string> tokenize(const string& str) {
        vector<string> tokens;
        istringstream iss(str);
        string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    // 打印项
    string itemToString(const Item& item) const {
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
        
        // 在LR(0)解析中，我们不显示向前看符号
        //result += ", " + *item.lookahead.begin();
        
        return result;
    }

public:
    // 解析输入并构建LR(0)项集族
    void buildParser(const vector<string>& input) {
        parseGrammar(input);
        buildCanonicalCollection();
    }
    
    // 打印项集族
    void printCanonicalCollection() const {
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
};

int main(int argc, char* argv[]) {
    vector<string> grammar_input;
    string line;
    if (argc > 1) {
        string fileName = argv[1];

        ifstream file(fileName);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open input file: " + fileName);
        }
        while (getline(file, line) && !line.empty()) {
            grammar_input.push_back(line);
        }
    } else {
        cout << "请输入文法" << endl;
        while (getline(cin, line) && !line.empty()) {
            grammar_input.push_back(line);
        }
    }
    
    LRParser parser;
    parser.buildParser(grammar_input);
    parser.printCanonicalCollection();
    
    return 0;
}