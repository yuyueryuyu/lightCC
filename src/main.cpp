#include <bits/stdc++.h>

#include "util/dfa.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "AstBuilder.hpp"
#include "AstPrinter.hpp"
#include "TypeChecker.hpp"
#include "IRBuilder.hpp"
#include "RegAllocator.hpp"
#include "RVWriter.hpp"
using namespace std;

void compile(Lexer& lexer, LRParser& parser, string input, string filename, bool check) {
    lexer.lex(input);
                    
    if (lexer.hasErr()) {
        lexer.printErrors();
        if (!check) {
            lexer.outputErrors(filename + ".err");
        }
        lexer.clear();
        return;
    }
    if (!check) {
        lexer.printTokens();
        lexer.outputTokens(filename + ".tokens");
    }
        
    vector<Token> tokens(lexer.getTokens());
    lexer.clear();

    ParseTreeNode* tree = parser.parseTokens(tokens, check);

    if (parser.hasErr()) {
        parser.printErrors();
        if (!check)
            parser.outputErrors(filename + ".err");
        parser.clear();
        return;
    }
    if (!check) {
        // 打印解析树
        parser.printParseTree();
        
        // 导出解析树
        parser.exportParseTreeToJSON(filename+ ".cst");
    }
    parser.clear();

    AstBuilder builder;
    Program* prog = dynamic_cast<Program*>(builder.visit(tree));
    if (builder.hasErr()) {
        builder.printErrors();
        if (!check)
            builder.outputErrors(filename + ".err");
        builder.clear();
        return;
    }
    builder.clear();

    TypeChecker checker;
    checker.visitNode(prog);

    if (checker.hasErr()) {
        checker.printErrors();
        if (!check)
            checker.outputErrors(filename+ ".err");
        checker.clear();
        return;
    }
    checker.clear();
    

    if (!check) {
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
    }
   

    IRBuilder irBuilder;
    auto irProg = irBuilder.visitProgram(prog);
    if (!check) {
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
    RegAllocator allocator;
    allocator.visitProgram(irProg);
    if (!check) {
        irProg->print(cout);

        ofstream fir(filename + ".alloc");
        if (!fir.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return;
        }
        irProg->print(fir);
        cout << endl;
        fir.close();
    }

    RVWriter writer;
    writer.visitProgram(irProg);
    if (!check) {
        irProg->printRV(cout);

        ofstream fir(filename + ".s");
        if (!fir.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return;
        }
        irProg->printRV(fir);
        cout << endl;
        fir.close();
    }

}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "help: compiler [file/directory to compiler] [-check]" << endl;
        return 0;
    }
    bool check = false;
    if (argc == 3) {
        check = true;
    }

    string filename = filesystem::canonical(argv[0]).parent_path().string() + "/grammar/lex_rule.lex";
    DFA dfa(filename);
    
    Lexer lexer(dfa);
    if (!dfa.validate()) {
        return 1;
    }

    vector<string> grammar_input;
    string grammar_file = filesystem::canonical(argv[0]).parent_path().string() + "/grammar/gram_rule.gra";
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
    if (!check) {
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
    }
    
    string inputfile = argv[1];

    {
        ifstream file(inputfile);

        if (!file.is_open()) {
            //cerr << "无法打开文件: " << filename << endl;
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
                if (entry.is_regular_file() && entry.path().extension() == ".alloc") {
                    filesystem::remove(entry.path());
                }
                if (entry.is_regular_file() && entry.path().extension() == ".s") {
                    filesystem::remove(entry.path());
                }
            }
            for (const auto& entry : filesystem::directory_iterator(inputfile)) {
                if (entry.is_regular_file() && entry.path().extension() == ".src") {
                    ifstream prog(entry.path());
                    if (!prog.is_open()) {
                        //cerr << "无法打开文件: " << filename << endl;
                        continue;
                    }
                    stringstream buf;
                    buf << prog.rdbuf();
                    prog.close();

                    compile(lexer, parser, buf.str(), entry.path().string(), check);
                }
            }
        } else {
            stringstream buf;
            buf << file.rdbuf();
            compile(lexer, parser, buf.str(), "test", check);
        }
        file.close();
    }
    
    
    return 0;
}