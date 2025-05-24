#include "Lexer.hpp"
#include <iostream>
#include <fstream>
#include "util/dfa.hpp"
#include "util/error.hpp"
#include "util/token.hpp"


using namespace std;

void Lexer::lex(string input) {

    for (size_t i = 0; i < input.size();) {
        auto p = dfa.recognizeString(input.substr(i));
        string token = p.first;
        size_t ori = i;
        i += p.second;
        if (token == "SKIP") {
            continue;
        }
        if (token == "") {
            size_t pos[2]{i, i};
            err(pos, "near " + input.substr(i, 1));
            i++;
            //cout << "(" << token << ", " << str << ")" << endl;
        } else {
            size_t pos[2]{ori, p.second};
            Token t(pos, token, input.substr(ori, p.second));
            tokens.push_back(t);
        }
    }
    size_t pos[2]{input.size(), input.size()};
    Token t(pos, "EOF", "");
    tokens.push_back(t);
}

void Lexer::printErrors() {
    for (auto e : errors) {
        cout << e.toString() << endl;
    }
}

void Lexer::printTokens() {
    for (auto t : tokens) {
        cout << t.toString() << endl;
    }
}

void Lexer::outputTokens(string file) {
    ofstream f(file);
    if (!f.is_open()) {
        cerr << "无法打开文件: " << file << endl;
        return;
    }
    for (auto t : tokens) {
        f << t.toString() << endl;
    }
    f.close();
}

void Lexer::outputErrors(const string& file) {
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