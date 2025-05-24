#ifndef PARSERULE_HPP
#define PARSERULE_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

// 表示一个产生式规则
struct Production {
    std::string left;            // 产生式左部
    std::vector<std::string> right;   // 产生式右部
    int id;                 // 产生式编号
    
    bool operator==(const Production& other) const;
    bool operator<(const Production& other) const;

    void print();
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
    std::set<Item> items;
    std::map<std::string, int> goto_transitions;  // 转移函数：符号 -> 项集编号
    
    bool operator==(const ItemSet& other) const {
        return items == other.items;
    }
};

// SLR(1)分析表中的操作类型
enum ActionType {
    SHIFT,
    REDUCE,
    ACCEPT,
    ERR
};

// SLR(1)分析表中的条目
struct ActionEntry {
    ActionType type;
    int value;  // SHIFT: 状态编号, REDUCE: 产生式编号
    
    ActionEntry() : type(ERR), value(-1) {}
    ActionEntry(ActionType t, int v) : type(t), value(v) {}
};

#endif