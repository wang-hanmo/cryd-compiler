#pragma once
#include <array>
#include <unordered_set>
#include "arm.h"

class ArmBlock
{
private:
    std::string label;
    std::vector<ArmBlock*> edge;              //正向图的邻接表，下标0表示false转移或无条件转移，下标1表示true转移
    // 基本块的前驱
    std::list<ArmBlock*> pred;
    std::list<ArmInstr> inst_list;
public:
    std::vector<Reg*> save_regs;
    // 需要存放到栈中的临时变量
    std::vector<IRSymbol*> temp_in_stack;
    ArmBlock(std::string _label): label(_label){}
    void push_back(ArmInstr inst);
    void gen_asm(std::ostream& os);
    void set_edge(ArmBlock* bb) {edge.emplace_back(bb);}
    void add_pred(ArmBlock* bb) {pred.emplace_back(bb);}
    int pred_num() {return pred.size();}
    std::vector<ArmBlock*>& get_edge() {return edge;}
    std::string get_label() {return label;}
    std::list<ArmInstr>& get_inst_list() {return inst_list;}
    std::list<ArmBlock*> get_pred_list() {return pred;}
    ArmBlock* get_pred() {return pred.front();}
    ArmBlock* get_edge(int index) {return edge[index];}
};

class ArmFunc
{
private:
    std::string name;
    std::list<ArmBlock*> blocks;
public:
    // 给临时变量和局部变量分配的总空间（不同基本块的临时变量可以共用一块栈空间）
    int stack_size;
    // 栈中的变量相对栈底的偏移
    std::unordered_map<IRSymbol*, int> var_offset;
    std::unordered_set<Reg*> used_regs;
    std::unordered_set<IRSymbol*> fparams_in_stack;
    // 需要存放到栈中的局部变量
    std::vector<IRSymbol*> var_in_stack;
    std::vector<Reg*> save_regs;
    std::vector<Reg*> save_vfp;
    ArmFunc(std::string _name): name(_name){}
    void push_back(ArmBlock* block);
    void gen_asm(std::ostream& os);
    void set_name(std::string name) {this->name = name;}
    void clear_block() {blocks.clear();}
    std::list<ArmBlock*>& get_blocks() {return blocks;}
    std::string get_name() {return name;}
};

class ArmProg
{
private:
    std::list<ArmFunc*> func_list;
public:
    void push_back(ArmFunc* func);
    void gen_asm(std::ostream& os);
    std::list<ArmFunc*>& get_func_list() {return func_list;}
};