#include <bits/stdc++.h>

#include "util/dfa.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AstBuilder.hpp"
#include "AstPrinter.hpp"
#include "TypeChecker.hpp"
#include "IRBuilder.hpp"
using namespace std;

void compile(Lexer& lexer, LRParser& parser, string input, string filename) {
    lexer.lex(input);
                    
    if (lexer.hasErr()) {
        lexer.outputErrors(filename + ".err");
        lexer.clear();
        return;
    }
    lexer.outputTokens(filename + ".tokens");
    vector<Token> tokens(lexer.getTokens());
    lexer.clear();

    ParseTreeNode* tree = parser.parseTokens(tokens);

    if (!tree) {
        parser.outputErrors(filename + ".err");
        parser.clear();
        return;
    }
    // 打印解析树
    parser.printParseTree();
    
    // 导出解析树到JSON文件
    parser.exportParseTreeToJSON(filename+ ".cst");
    parser.clear();

    AstBuilder builder;
    Program* prog = dynamic_cast<Program*>(builder.visit(tree));
    if (builder.hasErr()) {
        builder.outputErrors(filename + ".err");
        builder.clear();
        return;
    }
    builder.clear();

    TypeChecker checker;
    checker.visitNode(prog);

    if (checker.hasErr()) {
        checker.outputErrors(filename+ ".err");
        checker.clear();
        return;
    }
    checker.clear();
    
    PrettyPrinter printer;
    ofstream f(filename + ".ast");
    if (!f.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }
    printer.output(prog, f);
    printer.print(prog);
    cout << endl;
    f.close();

    IRBuilder irBuilder;
    auto irProg = irBuilder.visitProgram(prog);
    irProg->print(cout);

    ofstream fir(filename + ".ir");
    if (!fir.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }
    irProg->print(fir);
    cout << endl;
    fir.close();
}

int main(int argc, char* argv[]) {
    if (argc <= 3) {
        cout << "help: compiler [lexical file] [grammar file] [file/directory to compiler]" << endl;
        return 0;
    }
    string filename = argv[1];
    DFA dfa(filename);
    
    Lexer lexer(dfa);
    if (!dfa.validate()) {
        return 1;
    }

    vector<string> grammar_input;
    string grammar_file = argv[2];
    string line;
    ifstream file(grammar_file);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open input file: " +grammar_file);
    }
    while (getline(file, line) && !line.empty()) {
        grammar_input.push_back(line);
    }
    
    LRParser parser;
    parser.buildParser(grammar_input);
    
    cout << "\n=== 产生式列表 ===" << endl;
    parser.printProductions();
    
    cout << "\n=== 项集族 ===" << endl;
    parser.printCanonicalCollection();
    
    cout << "\n=== FIRST集合 ===" << endl;
    parser.printFirstSets();
    
    cout << "\n=== FOLLOW集合 ===" << endl;
    parser.printFollowSets();
    
    // 输出分析表摘要到控制台
    cout << "\n=== SLR(1)分析表 ===" << endl;
    
    // 导出完整分析表到CSV文件
    parser.exportSLRTableToCSV("slr_table.csv");
    string inputfile = argv[3];

    {
        ifstream file(inputfile);

        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            exit(1);
        }

        if (filesystem::is_directory(inputfile)) {
            for (const auto& entry : filesystem::directory_iterator(inputfile)) {
                if (entry.is_regular_file() && entry.path().extension() == ".err") {
                    filesystem::remove(entry.path());
                }
                if (entry.is_regular_file() && entry.path().extension() == ".tokens") {
                    filesystem::remove(entry.path());
                }
                if (entry.is_regular_file() && entry.path().extension() == ".cst") {
                    filesystem::remove(entry.path());
                }
                if (entry.is_regular_file() && entry.path().extension() == ".ast") {
                    filesystem::remove(entry.path());
                }
                if (entry.is_regular_file() && entry.path().extension() == ".ir") {
                    filesystem::remove(entry.path());
                }
            }
            for (const auto& entry : filesystem::directory_iterator(inputfile)) {
                if (entry.is_regular_file() && entry.path().extension() == ".src") {
                    ifstream prog(entry.path());
                    if (!prog.is_open()) {
                        cerr << "无法打开文件: " << filename << endl;
                        continue;
                    }
                    stringstream buf;
                    buf << prog.rdbuf();
                    prog.close();

                    compile(lexer, parser, buf.str(), entry.path().string());
                }
            }
        } else {
            stringstream buf;
            buf << file.rdbuf();
            compile(lexer, parser, buf.str(), "test");
        }
        file.close();
    }
    
    
    return 0;
}