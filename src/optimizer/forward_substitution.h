#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
/*
    tag使用情况：
    l、p变量混合编码,数组不考虑
    t变量单独编码
    g变量单独编码
    依赖特性：
    按照bfs遍历基本块时，所有temp变量的定值一定在使用前出现
*/
//符号，定义语句对
namespace forward_substitution {
    struct SymDefPair
    {
        IRSymbol* sym{ nullptr };
        IRInstr* def{ nullptr };
        SymDefPair() {}
        SymDefPair(IRSymbol* sym_, IRInstr* def_) :sym(sym_), def(def_) {};
        SymDefPair(IRInstr* def_) :def(def_) { sym = def_->r(); };
        SymDefPair(IRSymbol* sym_) :sym(sym_) {};
        bool operator<(const SymDefPair& rhs)const
        {
            return sym < rhs.sym;
        }
    };
}
class ForwardSubstitution final : public Pass
{
private:
    IRSymbol* check_replace(IRSymbol* sym);
    void delete_related_instr(IRSymbol* sym);
    void work_cfg(IRUnit* unit);
    void work_ir_list(IRInstrList& program);
    void work_assign(IRInstr* instr);
    void work_unary_calc(IRInstr* instr);
    void work_binary_calc(IRInstr* instr);
    void work_ternary_calc(IRInstr* instr);
    void work_block_cond_goto(IRInstr* instr);
    void init();
    void get_use_info_for_temp_vars(IRUnit* unit);      //计算t变量的定值使用信息
    std::vector<int> m_use_count;                       //t变量编号->使用次数
    std::vector<std::vector<IRSymbol*>> m_related_sym_glb;  //g变量->与之关联（在其定值语句中被使用过）的t变量
    std::vector<std::vector<IRSymbol*>> m_related_sym;      //lp变量->与之关联（在其定值语句中被使用过）的t变量
    std::set<forward_substitution::SymDefPair> m_sym_def_set;                 //当前有效的(t变量，定值指令)对集合
    std::set<forward_substitution::SymDefPair> m_sym_def_set_glb;             //同上，但考虑定值中有g变量的情况
    int m_temp_count;
    int m_local_param_count;
    int m_global_count;
    bool is_non_pointer_temp_var(IRSymbol* sym);        //是否为非指针型temp变量
    bool is_not_considered_type(IRSymbol* sym);         //是否为不考虑进行替代的变量（全局变量、数组、指针、寄存器、内存变量）
public:
    ForwardSubstitution(bool i_emit) :Pass(PassType::ForwardSubstitution, PassTarget::LIR, i_emit) {}
    void run();
};