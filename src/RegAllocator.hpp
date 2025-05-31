#ifndef REGALLOCATOR_HPP
#define REGALLOCATOR_HPP
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <stack>
#include <set>
#include "util/ir.hpp"
#include "util/storage.hpp"

struct RegGraphNode {
    IRSym* sym;
    std::set<RegGraphNode*> links;

    RegGraphNode(IRSym* sym) : sym(sym) {}
    RegGraphNode(const RegGraphNode& node) : sym(node.sym) {
        for (auto link : node.links) {
            links.insert(link);           
        }
    }
};

class RegAllocator {
public:
    RegAllocator() = default;
    
    void visitProgram(IRProgram* prog) {
        for (auto var : prog->getGlobal()) {
            var->getSym()->setStorage(new StaticStorage());
        }
        for (auto func : prog->getFunc()) {
            visitFunc(func);
        }
    }

    void visitFunc(IRFunc* func) {
        func->getSym()->setStorage(new StaticStorage());
        int float_num = 0;
        int int_num = 0;
        int stack_num = 0;
        for (size_t i = 0; i < func->getParams().size(); i++) {
            auto param = func->getParams()[i];
            if (param->getType()->equals(&FLOAT_TYPE)) {
                if (float_num <= 7) param->setStorage(new RegStorage(getFA(float_num++)));
                else param->setStorage(new StackStorage(4 * (stack_num++)));
            } else {
                if (int_num <= 7) param->setStorage(new RegStorage(getA(int_num++)));
                else param->setStorage(new StackStorage(4 * (stack_num++)));
            }
        }
        func->setParamSize(stack_num*4);
        int curSize = -8;
        auto entry = func->getEntryBlock();
        for (auto instr : entry->getInstrs()) {
            if (auto alloc = dynamic_cast<IRAlloc*>(instr)) {
                curSize -= alloc->getAllocType()->getSize();
                alloc->setPosition(curSize);
            }
        }
        for (auto block : func->getBlocks()) {
            curSize = visitBlockInt(block, curSize);
            curSize = visitBlockFlo(block, curSize);
        }
        int argSize = 0;
        for (auto call : func->getCalls()) {
            argSize = argSize >= call->getParamSize() ? argSize : call->getParamSize();
        }
        func->setSize(curSize-argSize);
    }

    int visitBlockInt(BasicBlock* block, int curSize) {
        std::vector<IRSym*> live_var[block->getInstrs().size()+1];
        std::map<IRSym*, RegGraphNode*> graph;
        std::vector<RegGraphNode*> v_graph;
        for (int i = block->getInstrs().size()-1; i >= 0; i--) {
            auto instr = block->getInstrs()[i];
            auto defs = instr->getDef();
            auto uses = instr->getUse();
            auto next = live_var[i+1];
            for (auto def : defs) {
                if (def->getType()->equals(&FLOAT_TYPE)) continue;
                graph[def] = new RegGraphNode(def);
                v_graph.push_back(graph[def]);
            }
            for (auto sym : next) {
                if (sym->getType()->equals(&FLOAT_TYPE)) continue;
                for (auto def : defs) {
                    if (def->getType()->equals(&FLOAT_TYPE)) continue;
                    if (sym == def)
                        continue;
                }
                live_var[i].push_back(sym);
            }
            for (auto use : uses) {
                if (use->getType()->equals(&FLOAT_TYPE)) continue;
                if (use->getStorage() == nullptr) {
                    live_var[i].push_back(use);
                }
            }
        }

        for (auto v : live_var) {
            for (size_t i = 0; i < v.size(); i++) {
                for (size_t j = i+1; j < v.size(); j++) {
                    graph[v[i]]->links.insert(graph[v[j]]);
                    graph[v[j]]->links.insert(graph[v[i]]);
                }
            }   
        }
        
        std::sort(v_graph.begin(), v_graph.end(), [](RegGraphNode* a, RegGraphNode* b){
            return a->links.size() < b->links.size();
        });
        std::stack<IRSym*> sym_stack;
        // 可供分配寄存器: T0 - T5共6个， T6作为常量寄存器。
        size_t max_reg = 6; 
        for (size_t i = 0; i < v_graph.size(); i++) {
            auto node = v_graph[i];
            if (node->links.size() < max_reg) {
                sym_stack.push(node->sym);
            } else {
                curSize -= 4;
                node->sym->setStorage(new StackStorage(curSize));
            }

            for (size_t j = i+1; j < v_graph.size(); j++) {
                auto other = v_graph[j];
                for (auto out = other->links.begin(); out != other->links.end(); out++) {
                    if (*out == node) {
                        other->links.erase(out);
                        break;
                    }
                }
            }
        }

        for (auto v : v_graph) {
            v->links.clear();
        }

        for (auto v : live_var) {
            for (size_t i = 0; i < v.size(); i++) {
                for (size_t j = i+1; j < v.size(); j++) {
                    graph[v[i]]->links.insert(graph[v[j]]);
                    graph[v[j]]->links.insert(graph[v[i]]);
                }
            }   
        }

        while (!sym_stack.empty()) {
            auto sym = sym_stack.top();
            sym_stack.pop();
            int x = 0;
            for (auto link : graph[sym]->links) {
                if (link->sym->getStorage() == nullptr) continue;
                if (auto reg = dynamic_cast<RegStorage*>(link->sym->getStorage())) {
                    auto preg = reg->getReg();
                    for (int i = 0; i < 6; i++) {
                        if (preg == getT(i)) {
                            x = x | (1 << i);
                        }
                    }
                }
            }
            int i = 0;
            while (x % 2 != 0) {
                i++;
                x >>= 1;
            }
            sym->setStorage(new RegStorage(getT(i)));
        }

        return curSize;
    }

    int visitBlockFlo(BasicBlock* block, int curSize) {
        std::vector<IRSym*> live_var[block->getInstrs().size()+1];
        std::map<IRSym*, RegGraphNode*> graph;
        std::vector<RegGraphNode*> v_graph;
        for (int i = block->getInstrs().size()-1; i >= 0; i--) {
            auto instr = block->getInstrs()[i];
            auto defs = instr->getDef();
            auto uses = instr->getUse();
            auto next = live_var[i+1];
            for (auto def : defs) {
                if (!def->getType()->equals(&FLOAT_TYPE)) continue;
                graph[def] = new RegGraphNode(def);
                v_graph.push_back(graph[def]);
            }
            for (auto sym : next) {
                if (!sym->getType()->equals(&FLOAT_TYPE)) continue;
                for (auto def : defs) {
                    if (!def->getType()->equals(&FLOAT_TYPE)) continue;
                    if (sym == def)
                        continue;
                }
                live_var[i].push_back(sym);
            }
            for (auto use : uses) {
                if (!use->getType()->equals(&FLOAT_TYPE)) continue;
                if (use->getStorage() == nullptr) {
                    live_var[i].push_back(use);
                }
            }
        }

        for (auto v : live_var) {
            for (size_t i = 0; i < v.size(); i++) {
                for (size_t j = i+1; j < v.size(); j++) {
                    graph[v[i]]->links.insert(graph[v[j]]);
                    graph[v[j]]->links.insert(graph[v[i]]);
                }
            }   
        }
        
        std::sort(v_graph.begin(), v_graph.end(), [](RegGraphNode* a, RegGraphNode* b){
            return a->links.size() < b->links.size();
        });
        std::stack<IRSym*> sym_stack;
        // 可供分配寄存器: FT0 - FT10共11个, F11作为暂存寄存器。
        size_t max_reg = 11; 
        for (size_t i = 0; i < v_graph.size(); i++) {
            auto node = v_graph[i];
            if (node->links.size() < max_reg) {
                sym_stack.push(node->sym);
            } else {
                curSize -= 4;
                node->sym->setStorage(new StackStorage(curSize));
            }

            for (size_t j = i+1; j < v_graph.size(); j++) {
                auto other = v_graph[j];
                for (auto out = other->links.begin(); out != other->links.end(); out++) {
                    if (*out == node) {
                        other->links.erase(out);
                        break;
                    }
                }
            }
        }

        for (auto v : v_graph) {
            v->links.clear();
        }

        for (auto v : live_var) {
            for (size_t i = 0; i < v.size(); i++) {
                for (size_t j = i+1; j < v.size(); j++) {
                    graph[v[i]]->links.insert(graph[v[j]]);
                    graph[v[j]]->links.insert(graph[v[i]]);
                }
            }   
        }

        while (!sym_stack.empty()) {
            auto sym = sym_stack.top();
            sym_stack.pop();
            int x = 0;
            for (auto link : graph[sym]->links) {
                if (link->sym->getStorage() == nullptr) continue;
                if (auto reg = dynamic_cast<RegStorage*>(link->sym->getStorage())) {
                    auto preg = reg->getReg();
                    for (int i = 0; i < 11; i++) {
                        if (preg == getFT(i)) {
                            x = x | (1 << i);
                        }
                    }
                }
            }
            int i = 0;
            while (x % 2 != 0) {
                i++;
                x >>= 1;
            }
            sym->setStorage(new RegStorage(getFT(i)));
        }

        return curSize;
    }
};

#endif