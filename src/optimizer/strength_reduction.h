#pragma once
#include <optimizer.h>
#include <cfg.h>
#include <vector>
#include <tuple>
/*
*   强度削弱
*   目前实现：
*   乘法：2的n次方，2的n次方-1,2的n次方+1
*   除法：任意立即数
*   模：  2的n次方
*/
class StrengthReduction final: public Pass
{
private:
    bool is_power_of_2(int num);
    IRInstrList::iterator const_mul_weaken(IRInstrList& instr_list, IRInstrList::iterator instr);
    std::tuple<int, int, int> choose_multiplier(int d, int N);
    IRInstrList::iterator const_div_weaken(IRInstrList& instr_list, IRInstrList::iterator instr);
    IRInstrList::iterator const_mod_weaken(IRInstrList& instr_list, IRInstrList::iterator instr);
    IRInstrList::iterator const_mla_weaken(IRInstrList& instr_list, IRInstrList::iterator instr);
    void work_cfg(IRUnit* entry);
    void work_ir_list(IRInstrList& program);
    int log2_for_int_pow_of_2(int num);
    int m_max_temp_index{0};
public:
    StrengthReduction(bool i_emit) :Pass(PassType::StrengthReduction,PassTarget::LIR, i_emit) {}
    int get_max_temp_index()  {return m_max_temp_index;}
    void run();
};