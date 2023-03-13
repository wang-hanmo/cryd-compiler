#pragma once

#include <optimizer.h>
#include <../ir/cfg.h>

class LocalArrayVisitSubstitution final :public Pass
{
private:
    int max_tag{0};
    //和变量关联的数组元素 <数组编号,变量编号>
    //std::vector<std::pair<int,int>> associated_element;
    // 记录数组指针和数组的关系
    std::unordered_map<IRSymbol*, IRSymbol*> array_pointer;
    // 对于数组有将其地址赋值给数组指针的，不参与优化
    std::unordered_set<IRSymbol*> unable;
    std::unordered_set<IRBlock*> visited;
    void simplify_array_visit(IRBlock* block);
    void ir_list_set_tag(IRInstrList& program);
    void set_tag(IRUnit* unit);
    void ir_list_init_tag(IRInstrList& program);
    void init_tag(IRUnit* unit);
    void work_before_simplify(IRUnit* unit);
public:
    LocalArrayVisitSubstitution(bool i_emit) :Pass(PassType::LocalArrayVisitSubstitution,PassTarget::CFG, i_emit) {}
    void run();
};