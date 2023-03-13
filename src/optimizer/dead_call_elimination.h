#pragma once
#include <optimizer.h>
#include <cfg.h>
#include <set>
/*
*   死调用删除。删除那些没有副作用，返回值又没有被利用的调用
*   删除那些没有作用的参数
*/
class DeadCallElimination final :public Pass
{
private:
    void dead_call_elimination(IRUnit* unit);
    void dead_param_mark(IRUnit* unit);
    void dead_param_elimination(IRUnit* unit);
    std::vector<std::vector<int>> dead_param_id;
    //构建函数调用图，并判断一个函数调用是否本身携带副作用（不包含调用其他函数导致的副作用）
    bool build_calling_map(IRUnit* unit);
    bool has_side_effect(IRSymbol* sym);
    void build_side_effect_info();
    //反调用图
    std::vector<std::set<int>> m_anti_calling_map;
    //函数数量
    int m_func_count;
    //函数是否有副作用
    std::vector<bool>m_has_side_effect;
public:
    DeadCallElimination(bool i_emit) :Pass(PassType::DeadCallElimination,PassTarget::CFG, i_emit) {}
    void run();
};
