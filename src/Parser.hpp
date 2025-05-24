#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <map>
#include <set>
#include <string>

#include "util/dfa.hpp"
#include "util/error.hpp"
#include "util/parserule.hpp"
#include "util/parsetree.hpp"

class LRParser {
private:
    std::vector<Production> productions;        // 所有产生式
    std::string start_symbol;                   // 开始符号
    std::set<std::string> non_terminals;             // 非终结符集合
    std::set<std::string> terminals;                 // 终结符集合
    std::vector<ItemSet> canonical_collection;  // 规范LR(0)项集族
    
    std::map<std::string, std::set<std::string>> first_sets;   // FIRST集合
    std::map<std::string, std::set<std::string>> follow_sets;  // FOLLOW集合
    
    std::vector<std::vector<ActionEntry>> action_table;  // ACTION表
    std::vector<std::map<std::string, int>> goto_table;      // GOTO表

    ParseTreeNode* parse_tree_root;
    
    bool has_conflicts;                      // 是否存在冲突

    std::vector<Error> errors;

    void err(Token token, std::string errMsg) {
        Error error("Parse", token.getPos(), errMsg);
        errors.push_back(error);
    }

    // 解析输入文法
    void parseGrammar(const std::vector<std::string>& input);
    
    // 计算闭包
    std::set<Item> closure(const std::set<Item>& items);
    
    // 计算GOTO函数
    std::set<Item> computeGoto(const std::set<Item>& items, const std::string& symbol);
    
    // 构建规范LR(0)项集族
    void buildCanonicalCollection();

    // 计算FIRST集合
    void computeFirstSets();
    
    // 计算FOLLOW集合
    void computeFollowSets();
    
    // 构建SLR(1)分析表
    void buildSLRTable();
    
    // 辅助函数：修剪字符串两端的空白
    std::string trim(const std::string& str);

    // 辅助函数：按分隔符分割字符串
    std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    
    // 辅助函数：将字符串分割成词法单元
    std::vector<std::string> tokenize(const std::string& str);
    
    // 打印项
    std::string itemToString(const Item& item) const;

public:
    // 解析输入并构建SLR(1)分析表
    void buildParser(const std::vector<std::string>& input);
    
    // 打印项集族
    void printCanonicalCollection() const;
    
    // 打印FIRST集合
    void printFirstSets() const;

    // 打印FOLLOW集合
    void printFollowSets() const;
    
    // 打印SLR(1)分析表
    void printSLRTable() const;
    
    // 将SLR(1)分析表输出到CSV文件
    void exportSLRTableToCSV(const std::string& filename) const;
    
    // 打印所有产生式
    void printProductions() const;

    ParseTreeNode* parseTokens(const std::vector<Token>& tokens);
    
    // 打印解析树
    void printParseTree() const;
    
    // 将解析树导出为JSON文件
    void exportParseTreeToJSON(const std::string& filename) const;

    void printErrors();

    void outputErrors(std::string file);

    void clear() {
        errors.clear();
    }

    // 获取解析树根节点
    ParseTreeNode* getParseTreeRoot() const {
        return parse_tree_root;
    }
    
    // 析构函数中清理解析树
    ~LRParser() {
        if (parse_tree_root) {
            delete parse_tree_root;
        }
    }
};
#endif