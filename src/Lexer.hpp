#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "util/dfa.hpp"
#include "util/error.hpp"
#include "util/token.hpp"

class Lexer {
    private:
        DFA dfa;
        std::vector<Error> errors;
        std::vector<Token> tokens;

        void err(size_t position[], std::string errMsg) {
            Error error("Lexer", position, errMsg);
            errors.push_back(error);
        }

        std::pair<size_t, size_t> posToLineCol(size_t idx, std::string str) {
            size_t line = 1;
            size_t col = 1;
            for (size_t i = 0; i < str.size(); i++) {
                char c = str[i];
                if (i == idx) break;
                if (c == '\n') {
                    line++;
                    col = 0;
                }
                col++;
            }
            return std::make_pair(line, col);
        }
        
    public:
        Lexer(DFA dfa) : dfa(dfa) {}

        void lex(std::string input);

        bool hasErr() {
            return !errors.empty();
        }

        void printErrors();

        void printTokens();

        void outputTokens(std::string file);

        void outputErrors(const std::string& file);

        std::vector<Token> getTokens() {
            return tokens;
        }

        void clear() {
            tokens.clear();
            errors.clear();
        }
};

#endif