#pragma once
#include <optimizer.h>
#include <cfg.h>
#include <vector>
#include <tuple>
/*
*   模运算展开
*/
class ModExpansion final: public Pass
{
private:
    bool is_power_of_2(int num);
    IRInstrList::iterator mod_expansion(IRInstrList& instr_list, IRInstrList::iterator instr);
    void work_cfg(IRUnit* entry);
    void find_max_index(IRUnit* unit);
    void work_ir_list(IRInstrList& program);
    void ir_list_find_index(IRInstrList& program);
    int m_max_temp_index{0};
public:
    ModExpansion(bool i_emit) :Pass(PassType::ModExpansion,PassTarget::CFG, i_emit) {}
    int get_max_temp_index()  {return m_max_temp_index;}
    void run();
};