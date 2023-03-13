#pragma once
#include <optimizer.h>
#include <cfg_manager.h>
#include <bitmap.h>
/*
    依赖性质：
    使用tag情况：
    IRSymbol的tag用于索引最新的SSA Symbol
*/
class ConvertSSA final :public Pass
{
private:
    //使用bitmap保存当前基本块的支配边界,使用支配树上的dfs序(dfn)作为索引
    std::vector<BitMap> m_dominance_frontier;
    std::vector<IRBlock*> m_idfn;                                   //inverse dfn,通过dfn反向索引节点
    std::vector<BitMap> m_var_rewrite_block;                        //变量的定值信息，只考虑参数和局部变量，按照tag来索引
    std::vector<BitMap> m_var_dominance_frontiers_plus;             //变量的迭代支配边界,tag索引
    //根据变量的tag索引SSA形式下的符号栈，list.back()维护了栈顶元素，栈顶元素表示当前定值过的最新符号
    std::vector<std::list<IRSymbol*>> m_current_var_symbol;
    //维护每个变量最新的SSA下标
    std::vector<int> m_var_ssa_index;
    //1 计算每个block的dfn，并保存dfn到block的反向索引（inverse dfn)
    void compute_idfn(IRBlock* entry);
    //2 计算每个基本块的支配边界
    void compute_dominance_frontiers();
    //3 记录变量定值信息
    void record_var_rewrite(IRUnit* unit);
    //4 计算所有变量的迭代支配边界
    void compute_dfp_for_all_vars();
    //4-1 计算基本块集合的迭代支配边界
    BitMap compute_dominance_frontiers_plus(const BitMap& set);
    //4-1-1 计算基本块集合的支配边界
    BitMap compute_dominance_frontiers_set(const BitMap& set);
    //5 插入phi函数
    void insert_phi_func();
    //6 重写所有变量定义语句
    void rewrite_defs(IRUnit* unit);
    //7 重写所有语句中变量定值和使用信息
    void rewrite_instructions(IRBlock* entry);
    IRSymbol* rewrite_define(IRSymbol* sym);
    IRSymbol* rewrite_use(IRSymbol* sym);
    //是否为要转为SSA的类型
    bool is_ssa_target_type(IRSymbol* sym);
public:
    ConvertSSA(bool i_emit) :Pass(PassType::ConvertSSA,PassTarget::CFG, i_emit) {}
    void run();
};
