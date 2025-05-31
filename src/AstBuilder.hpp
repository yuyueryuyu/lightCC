#ifndef AST_BUILDER_HPP
#define AST_BUILDER_HPP

#include <vector>
#include <string>
#include <iostream>
#include "util/parsetree.hpp"  
#include "util/astnodes.hpp"     
#include "util/error.hpp"    

class AstBuilder {
private:
    std::vector<Error> errors;
    
    void err(ParseTreeNode* node, std::string errMsg) {
        size_t pos[4] = {node->start.getPos()[0], node->start.getPos()[1], node->end.getPos()[2], node->end.getPos()[3]};
        Error error("Semantic", pos, errMsg);
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
    void printErrors() {
        for (auto e : errors) {
            std::cout << e.toString() << std::endl;
        }
    }
    bool hasErr() { return !errors.empty(); }

    void clear() { errors.clear(); }
};

#endif