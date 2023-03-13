#pragma once
#include <iostream>
#include <cfg.h>
#include <vector>
#include <bitmap.h>
#include <set>
#include <queue>
/*
    过程间数据流分析
    全局变量相关优化
    使用tag情况：
    unit以及函数定义语句的tag用于对函数进行编号
    全局变量的tag用于对全局变量进行编号
*/
class InterproceduralDataFlowAnalysis
{
private:
    bool m_global_symbol_optimization_is_valid;  //是否开启全局符号相关优化          (可能造成编译时间显著提升）
    // bool m_side_effect_info_is_valid;            //函数副作用信息是否有效
    //调用图
    std::vector<std::set<int>> m_converse_call_map;
    std::vector<std::set<int>> m_call_map;
    /*  副作用信息   */
    std::vector<BitMap> m_global_var_use;        //函数对全局非数组变量的使用信息(在集合内的可能使用，不在集合内的一定不会使用)
    std::vector<BitMap> m_global_var_def;        //函数对全局非数组变量的定值信息(在集合内的可能定值，不在集合内的一定不会定值）
    std::vector<bool> m_has_side_effect;         //函数是否有副作用(对全局变量或参数数组定值,调用库函数）
    std::vector<bool> m_affected_by_env;         //函数是否受环境影响(两次调用参数相同返回值不同)（对全局变量进行访问,调用库函数）
    std::vector<IRSymbol*> m_global_var;         //全局变量编号到符号的映射
    std::vector<IRUnit*> m_procedure;            //函数编号到函数的映射
    //函数数量
    int m_func_count;
    //非数组全局变量数量
    int m_global_var_count;
    IRProgram* m_cfg{ nullptr };
    IRSymbolTable* m_ir_sym_table{ nullptr };
    void build_call_map_and_local_side_effect(IRUnit* unit);
public:
    //对函数进行编号，放入tag中
    void procedure_numbering();
    //对非数组的全局变量进行编号，放入tag中
    void global_var_numbering();
    //计算函数副作用信息,但不考虑函数进入时值就被覆盖的情况（为了降低复杂度）,会对函数和全局变量重新编号
    void compute_side_effect_info();
    //是直接调用自己的递归函数
    bool is_direct_recursion_function(int index) { return m_call_map[index].find(index) != m_call_map[index].end(); }
    int get_func_count()const { return m_func_count; }
    int get_global_var_count()const { return m_global_var_count; }
    const BitMap& get_global_var_use(int index) { return m_global_var_use[index]; }
    const BitMap& get_global_var_def(int index) { return m_global_var_def[index]; }
    IRUnit* get_procedure(int index) { return m_procedure[index]; }
    IRSymbol* get_global_var(int index) { return m_global_var[index]; }
    bool has_side_effect(int index) { return m_has_side_effect[index]; }
    bool affected_by_env(int index) { return m_affected_by_env[index]; }
    void set_target(IRProgram* cfg, IRSymbolTable* ir_sym_table) { m_cfg = cfg; m_ir_sym_table = ir_sym_table; }
    void enable_global_symbol_optimization() { m_global_symbol_optimization_is_valid = true; }
    bool global_symbol_optimization_is_valid() { return m_global_symbol_optimization_is_valid; }
};
