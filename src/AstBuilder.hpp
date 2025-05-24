#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include <vector>
#include <string>
#include "util/parsetree.hpp"  
#include "util/astnodes.hpp"     
#include "util/error.hpp"    

class AstBuilder {
private:
    std::vector<Error> errors;
    
    void err(Token token, std::string errMsg) {
        Error error("Semantic", token.getPos(), errMsg);
        errors.push_back(error);
    }
    
    std::vector<Node*> visitDecls(ParseTreeNode* node);
    std::vector<Node*> visitStmts(ParseTreeNode* node);
    std::vector<Node*> visitParams(ParseTreeNode* node);
    std::vector<Node*> visitArgs(ParseTreeNode* node);

public:
    Node* visit(ParseTreeNode* node);
    
    // Optionally add a method to get collected errors
    std::vector<Error> getErrors() const { return errors; }
    void clearErrors() { errors.clear(); }

    void outputErrors(std::string file);
    bool hasErr() { return !errors.empty(); }

    void clear() { errors.clear(); }
};

#endif