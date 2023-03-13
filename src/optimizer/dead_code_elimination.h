#pragma once
#include <optimizer.h>
#include <cfg.h>
#include <set>
/*
*  死代码删除优化
*  依赖性质：
*  IR为SSA形式
*/
class DeadCodeElimination final :public Pass
{
private:
    int m_sym_count;                                  //SSA符号个数
    std::vector<IRInstr*> m_sym_to_def_instr;         //SSA符号到其定值指令的映射
    std::queue<IRInstr*> m_working_queue;             //活跃变量分析的工作队列
    bool is_ssa_var(IRSymbol* sym);                   //是否为SSA形式的变量
    bool has_lvalue(const IRInstr& instr);            //是否含有SSA形式的左值
    std::vector<IRSymbol*> rvalue_list(const IRInstr& instr);   //取出所有SSA形式的右值
    bool is_initially_alive(const IRInstr& instr);    //是否在最初就为活跃指令
    void work_unit(IRUnit& unit);                     //顶层-对IRUnit执行死代码删除
    void dead_block_eliminate(IRUnit& unit);          //1.删除不可达基本块
    void dead_block_eliminate_mark(IRBlock* entry);   //1-1.bfs标记不可达基本块
    void dead_block_eliminate_delete(IRBlock* entry); //1-2.dfs删除不可达基本块
    void initialize_index(IRUnit& unit);              //2.初始化符号的编号，维护符号到定值指令的映射，向工作队列添加活跃指令。
    void active_instruction_analysis();               //3.活跃指令分析
    void dead_instr_elimination(IRUnit& unit);        //5.死代码删除
    void global_active_instruction_analysis(IRUnit& unit);//4.全局变量相关活跃指令分析
    void global_active_instruction_analysis_block(IRBlock* block, int unit_tag);  //4-1.基本块内的全局变量相关活跃指令分析。返回值为是否标注了新的死代码
    //const int m_global_analysis_max_iteration = 3;
public:
    DeadCodeElimination(bool i_emit) :Pass(PassType::DeadCodeElimination,PassTarget::CFG, i_emit) {}
    void run();
};
