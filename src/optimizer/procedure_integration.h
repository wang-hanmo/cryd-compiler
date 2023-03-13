#pragma once
#include <optimizer.h>
#include <cfg_manager.h>
#include <cfg.h>
#include <ir_symbol_table.h>
#include <linear_ir_manager.h>

class ProcedureIntegration final :public Pass
{
private:
    std::unordered_map<IRSymbol*, int> m_procedures;
    std::vector<IRUnit*> m_units;
    std::vector<int> m_instr_num;       //过程中的IR指令数
    std::vector<int> m_symbol_num;      //过程中的符号标号最大值
    std::vector<bool> m_recursive;      //过程是否递归
    std::vector<bool> m_array;          //过程中是否有数组指针参数
    std::vector<int> m_call_num;        //过程被调用的次数
    void initial();
    void search();
    bool is_inline(IRSymbol* func);     //决策函数
    void integrate_procedure(IRBlock* block, int unit_no);
    void insert(IRBlock* block, int instr_no, IRUnit* src_unit, int unit_no, std::unordered_map<IRSymbol*, IRSymbol*> param_map);
    void rename(IRUnit* new_unit, int unit_no, std::vector<IRSymbol*> rparams,  std::unordered_map<IRSymbol*, IRSymbol*>& param_map);
    void delete_dead_func();
public:
    ProcedureIntegration(bool i_emit) :Pass(PassType::ProcedureIntegration,PassTarget::CFG, i_emit) {}
    void run();
};