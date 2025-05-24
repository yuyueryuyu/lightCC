#include <bits/stdc++.h>
#include "AstBuilder.hpp" 
using namespace std;

Node* AstBuilder::visit(ParseTreeNode* node) {
    if (node->symbol == "ε") {
        return nullptr;
    } else if (node->symbol == "Program") {
        vector<Node*> decls_nodes = visitDecls(node->children[0]);
        vector<Node*> stmts_nodes = visitStmts(node->children[1]);
        
        // 将Node*转换为Decl*和Stmt*
        vector<Decl*> decls;
        for (Node* n : decls_nodes) {
            decls.push_back(dynamic_cast<Decl*>(n));
        }
        vector<Stmt*> stmts;
        for (Node* n : stmts_nodes) {
            stmts.push_back(dynamic_cast<Stmt*>(n));
        }
        
        return new Program(node->token, decls, stmts);
    } else if (node->symbol == "Decl") {
        if (node->children.size() == 2) {
            // Type ID
            Type* type = dynamic_cast<Type*>(visit(node->children[0]));
            Id* id = new Id(node->children[1]->token, node->children[1]->token_value);
            return new VarDecl(node->token, type, id, 0);
        } else if (node->children.size() == 5) {
            // Type ID LBK NUM RBK
            Type* type = dynamic_cast<Type*>(visit(node->children[0]));
            Id* id = new Id(node->children[1]->token, node->children[1]->token_value);
            int dimension = stoi(node->children[3]->token_value);
            if (dimension <= 0) {
                err(node->token, "dimension is not positive");
                dimension = 1;
            }
            return new VarDecl(node->token, type, id, dimension);
        } else if (node->children.size() == 9) {
            // Type ID LPA Params RPA LBR Decls Stmts RBR
            Type* retType = dynamic_cast<Type*>(visit(node->children[0]));
            Id* id = new Id(node->children[1]->token, node->children[1]->token_value);
            vector<Node*> params_nodes = visitParams(node->children[3]);
            vector<Node*> decls_nodes = visitDecls(node->children[6]);
            vector<Node*> stmts_nodes = visitStmts(node->children[7]);
            
            // 转换为对应类型
            vector<Decl*> params;
            for (Node* n : params_nodes) {
                params.push_back(dynamic_cast<Decl*>(n));
            }
            vector<Decl*> decls;
            for (Node* n : decls_nodes) {
                decls.push_back(dynamic_cast<Decl*>(n));
            }
            vector<Stmt*> stmts;
            for (Node* n : stmts_nodes) {
                stmts.push_back(dynamic_cast<Stmt*>(n));
            }
            
            return new FuncDecl(node->token, retType, id, params, decls, stmts);
        }
    } else if (node->symbol == "Type") {
        return new Type(node->token, node->children[0]->token_value);
    } else if (node->symbol == "Param") {
        if (node->children.size() == 2) {
            // Type ID
            Type* type = dynamic_cast<Type*>(visit(node->children[0]));
            Id* id = new Id(node->children[1]->token, node->children[1]->token_value);
            return new VarDecl(node->token, type, id, 0);
        } else if (node->children.size() == 4) {
            // Type ID LBK RBK
            Type* type = dynamic_cast<Type*>(visit(node->children[0]));
            Id* id = new Id(node->children[1]->token, node->children[1]->token_value);
            return new VarDecl(node->token, type, id, -1); // -1表示数组参数
        } else if (node->children.size() == 5) {
            // Type ID LPA Type RPA (函数指针参数
            Type* type = dynamic_cast<Type*>(visit(node->children[0]));
            Type* paramType = dynamic_cast<Type*>(visit(node->children[3]));
            Id* id = new Id(node->children[1]->token, node->children[1]->token_value);
            vector<Decl *> params;
            params.push_back(new VarDecl(node->children[3]->token, paramType, nullptr, 0));
            vector<Decl *> decls;
            vector<Stmt *> stmts;
            return new FuncDecl(node->token, type, id, params, decls, stmts);
        }
    } else if (node->symbol == "Stmt") {
        if (node->children.size() == 0) {
            // ε
            return nullptr;
        } else if (node->children.size() == 3 && node->children[1]->symbol == "ASG") {
            // ID ASG Expr
            Id* target = new Id(node->children[0]->token, node->children[0]->token_value);
            Expr* value = dynamic_cast<Expr*>(visit(node->children[2]));
            return new Assign(node->token, target, value);
        } else if (node->children.size() == 6 && node->children[1]->symbol == "LBK") {
            // ID LBK Expr RBK ASG Expr
            Id* id = new Id(node->children[0]->token, node->children[0]->token_value);
            Expr* dimension = dynamic_cast<Expr*>(visit(node->children[2]));
            Index* target = new Index(node->children[0]->token, id, dimension);
            Expr* value = dynamic_cast<Expr*>(visit(node->children[5]));
            return new Assign(node->token, target, value);
        } else if (node->children.size() == 5 && node->children[0]->symbol == "IF") {
            // IF LPA Cond RPA Stmt
            Expr* cond = dynamic_cast<Expr*>(visit(node->children[2]));
            Stmt* thenStmt = dynamic_cast<Stmt*>(visit(node->children[4]));
            return new If(node->token, cond, thenStmt);
        } else if (node->children.size() == 7 && node->children[0]->symbol == "IF") {
            // IF LPA Cond RPA Stmt ELSE Stmt
            Expr* cond = dynamic_cast<Expr*>(visit(node->children[2]));
            Stmt* thenStmt = dynamic_cast<Stmt*>(visit(node->children[4]));
            Stmt* elseStmt = dynamic_cast<Stmt*>(visit(node->children[6]));
            return new If(node->token, cond, thenStmt, elseStmt);
        } else if (node->children.size() == 5 && node->children[0]->symbol == "WHILE") {
            // WHILE LPA Cond RPA Stmt
            Expr* cond = dynamic_cast<Expr*>(visit(node->children[2]));
            Stmt* body = dynamic_cast<Stmt*>(visit(node->children[4]));
            return new While(node->token, cond, body);
        } else if (node->children.size() == 2 && node->children[0]->symbol == "RETURN") {
            // RETURN Expr
            Expr* value = dynamic_cast<Expr*>(visit(node->children[1]));
            return new Return(node->token, value);
        } else if (node->children.size() == 3 && node->children[0]->symbol == "LBR") {
            // LBR Stmts RBR
            vector<Node*> stmts_nodes = visitStmts(node->children[1]);
            vector<Stmt*> stmts;
            for (Node* n : stmts_nodes) {
                stmts.push_back(dynamic_cast<Stmt*>(n));
            }
            return new Block(node->token, stmts);
        } else if (node->children.size() == 4 && node->children[1]->symbol == "LPA") {
            // ID LPA Args RPA
            Id* id = new Id(node->children[0]->token, node->children[0]->token_value);
            vector<Node*> args_nodes = visitArgs(node->children[2]);
            vector<Expr*> args;
            for (Node* n : args_nodes) {
                args.push_back(dynamic_cast<Expr*>(n));
            }
            Call* call = new Call(node->children[0]->token, id, args);
            return new ExprEval(node->token, call);
        }
    } else if (node->symbol == "Expr") {
        if (node->children.size() == 1) {
            if (node->children[0]->symbol == "NUM") {
                // NUM
                int value = stoi(node->children[0]->token_value);
                return new Int(node->children[0]->token, value);
            } else if (node->children[0]->symbol == "FLO") {
                // FLO
                float value = stof(node->children[0]->token_value);
                return new Float(node->children[0]->token, value);
            } else if (node->children[0]->symbol == "ID") {
                // ID
                return new Id(node->children[0]->token, node->children[0]->token_value);
            }
        } else if (node->children.size() == 3) {
            if (node->children[0]->symbol == "LPA") {
                // LPA Expr RPA
                return visit(node->children[1]);
            } else if (node->children[1]->symbol == "ADD" || node->children[1]->symbol == "MUL") {
                // Expr ADD/MUL Expr
                Expr* left = dynamic_cast<Expr*>(visit(node->children[0]));
                char op = (node->children[1]->symbol == "ADD") ? '+' : '*';
                Expr* right = dynamic_cast<Expr*>(visit(node->children[2]));
                return new Binary(node->children[1]->token, op, left, right);
            }
        } else if (node->children.size() == 4) {
            if (node->children[1]->symbol == "LBK") {
                // ID LBK Expr RBK
                Id* id = new Id(node->children[0]->token, node->children[0]->token_value);
                Expr* dimension = dynamic_cast<Expr*>(visit(node->children[2]));
                return new Index(node->children[0]->token, id, dimension);
            } else if (node->children[1]->symbol == "LPA") {
                // ID LPA Args RPA
                Id* id = new Id(node->children[0]->token, node->children[0]->token_value);
                vector<Node*> args_nodes = visitArgs(node->children[2]);
                vector<Expr*> args;
                for (Node* n : args_nodes) {
                    args.push_back(dynamic_cast<Expr*>(n));
                }
                return new Call(node->children[0]->token, id, args);
            }
        }
    } else if (node->symbol == "Cond") {
        if (node->children.size() == 1) {
            // Expr
            return visit(node->children[0]);
        } else if (node->children.size() == 3) {
            // Expr ROP Expr
            Expr* left = dynamic_cast<Expr*>(visit(node->children[0]));
            char op = '='; 
            if (node->children[1]->token_value == "<") op = '<';
            else if (node->children[1]->token_value == ">") op = '>';
            else if (node->children[1]->token_value == "==") op = '=';
            else if (node->children[1]->token_value == "!=") op = '!';
            else if (node->children[1]->token_value == "<=") op = 'l';
            else if (node->children[1]->token_value == ">=") op = 'g';
            
            Expr* right = dynamic_cast<Expr*>(visit(node->children[2]));
            return new Binary(node->children[1]->token, op, left, right);
        }
    } else if (node->symbol == "Arg") {
        if (node->children.size() == 1) {
            // Expr
            return visit(node->children[0]);
        } else if (node->children.size() == 3) {
            if (node->children[1]->symbol == "LBK") {
                // ID LBK RBK
                Id* id = new Id(node->children[0]->token, node->children[0]->token_value);
                return new Index(node->children[0]->token, id, nullptr); // 空索引表示整个数组
            } else if (node->children[1]->symbol == "LBR") {
                // ID LBR RBR
                return new Id(node->children[0]->token, node->children[0]->token_value);
            }
        }
    }
    return nullptr;
}

vector<Node*> AstBuilder::visitDecls(ParseTreeNode* node) {
    vector<Node*> result;
    if (node->symbol == "ε") {
        return result;
    } else if (node->symbol == "Decls") {
        if (node->children.size() == 0) {
            return result;
        } else {
            // Decls Decl SCO
            result = visitDecls(node->children[0]);
            Node* decl = visit(node->children[1]);
            if (decl != nullptr) {
                result.push_back(decl);
            }
        }
    }
    return result;
}

vector<Node*> AstBuilder::visitStmts(ParseTreeNode* node) {
    vector<Node*> result;
    if (node->symbol == "ε") {
        return result;
    } else if (node->symbol == "Stmts") {
        if (node->children.size() == 1) {
            // Stmt
            Node* stmt = visit(node->children[0]);
            if (stmt != nullptr) {
                result.push_back(stmt);
            }
        } else if (node->children.size() == 3) {
            // Stmts SCO Stmt
            result = visitStmts(node->children[0]);
            Node* stmt = visit(node->children[2]);
            if (stmt != nullptr) {
                result.push_back(stmt);
            }
        }
    }
    return result;
}

vector<Node*> AstBuilder::visitParams(ParseTreeNode* node) {
    vector<Node*> result;
    if (node->symbol == "ε") {
        return result;
    } else if (node->symbol == "Params") {
        if (node->children.size() == 0) {
            return result;
        } else {
            // Params Param SCO
            result = visitParams(node->children[0]);
            Node* param = visit(node->children[1]);
            if (param != nullptr) {
                result.push_back(param);
            }
        }
    }
    return result;
}

vector<Node*> AstBuilder::visitArgs(ParseTreeNode* node) {
    vector<Node*> result;
    if (node->symbol == "ε") {
        return result;
    } else if (node->symbol == "Args") {
        if (node->children.size() == 0) {
            return result;
        } else {
            // Args Arg CMA
            result = visitArgs(node->children[0]);
            Node* arg = visit(node->children[1]);
            if (arg != nullptr) {
                result.push_back(arg);
            }
        }
    }
    return result;
}

void AstBuilder::outputErrors(string file) {
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