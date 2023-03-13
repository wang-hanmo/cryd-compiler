#pragma once

#include <optimizer.h>
#include <../ir/cfg.h>

class ArrayVisitSubstitutionOnSSA final :public Pass
{
private:
    int max_tag{0};
    //和变量关联的数组元素 <数组编号,变量编号>
    std::unordered_set<IRBlock*> visited;
    void simplify_array_visit(IRBlock* block);
    void ir_list_set_tag(IRInstrList& program);
    void set_tag(IRUnit* unit);
    void ir_list_init_tag(IRInstrList& program);
    void init_tag(IRUnit* unit);
    void work_before_simplify(IRUnit* unit);
public:
    ArrayVisitSubstitutionOnSSA(bool i_emit) :Pass(PassType::ArrayVisitSubstitutionOnSSA,PassTarget::CFG, i_emit) {}
    void run();
};