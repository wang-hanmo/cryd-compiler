#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <ir_symbol_table.h>
#include <ir_instr.h>
#include <vector>
#include <queue>

enum LoopEnd:int
{
    LoopNoneEnd = -1,
    LoopValueEnd = 0,
    LoopVarEnd = 1,
};

class LoopUnrolling final: public Pass
{
private:
    void resize_table();
    void clear_table();
    bool is_ssa_var(IRSymbol* sym);
    bool has_lvalue(IRInstr& instr);
    void initial_instr_sym_tag(IRInstrList& program);
    void initial_decl_sym_tag(IRInstrList& program);
    void ir_list_find_index(IRInstrList& program);    //计算全局ssa最大下标以及temp类型最大下标
    void rebind_temp_index(IRInstr& instr);           //修改temp形式变量下标
    void rebind_ssa_index(IRInstr& instr);            //修改ssa类型变量下标
    void rebind_phi(IRInstr& instr,IRBlock* header);    //修改头结点phi函数
    void compute_local_ssa_max_index(IRInstr& instr);    //计算块内局部最大ssa形式下标
    bool is_computed_var(IRSymbol* sym);
    bool is_computed_expression(IRInstr& instr);
    bool can_leave_loop(int start,int target,IROper op,bool leave_condition); //判断是否可以离开循环
    int get_binary_value_result(int num1 ,int num2 ,IROper op);
    int get_unary_value_result(int num1,IROper op);
    int get_computed_var(IRSymbol* sym);
    void compute_instr_value(IRInstr& instr);
    int compute_initial_value(IRSymbol* sym,IRBlock* header);
    bool has_initial_value(IRSymbol* sym,IRBlock* header);   //判断循环变量是否有初值
    void update_ssa_index(IRBlock* block,IRBlock* pre_block);   
    void update_ssa_index_of_otherblock(IRBlock* header);//更新头结点之后的块中ssa下标
    void change_header_block_edge(IRBlock* header);    //改变头结点的边
    void work_loop(IRBlock* header,int size,int execute_time);   //复制循环体语句
    int predict_time(IRBlock* header,int predict_length);  //预测循环时间，无法预测或者循环次数过长不展开
    LoopEnd check_loop(const std::set<IRBlock*>& node_set,IRBlock* header);      //检测是否为单循环体
    void work_cfg(IRUnit* entry);
    void find_max_index(IRUnit* unit);
    void initial_sym_tag(IRUnit* unit);  //初始化symbol的tag字段
    
    bool check_inc_by_one(IRBlock* header);   //判断是否为加一的循环
    IRBlock* create_unroll_block(IRBlock* header,IRBlock* cond_block,int size,int unroll_size);   
    void change_new_block_edge(IRBlock* header,IRBlock* unroll_block,IRBlock* cond_block);  
    void vis_cfg(IRUnit* unit);
    IRBlock* create_cond_block(IRBlock* header,int unroll_size);

    int m_max_temp_index{0};   //全局最大temp类型变量下标
    int m_sym_count{0};        //symbol个数

    std::vector<IRInstr*>m_use_def_chain;    //ssa形式下标到定值语句的映射
    std::vector<IRSymbol*>m_local_temp_index_mp;  //存储局部temp类型下标
    std::vector<IRSymbol*> m_local_ssa_index_mp;   //局部ssa形式下的最大下标对应符号
    std::vector<int> m_ssa_sym_index_mp;    //ssa形式下各symbol的最大下标
    std::vector<int> m_sym_value_table;  //存储预测循环执行次数过程中符号当前的计算结果
    std::vector<bool>m_has_sym_value_table; //判断该符号是否有被计算过
    std::set<int> m_visited_instr; //判断定值语句是否访问过
    IRInstrList m_store_instrlist;      //存储复制语句
    //IRInstrList m_store_instrlist2;     //存储复制语句
    std::vector<IRSymbol*> m_stop_spread_sym; 
    std::vector<bool> m_is_stop_spread;

public:
    LoopUnrolling(bool i_emit) :Pass(PassType::LoopUnrolling,PassTarget::CFG, i_emit) {}
    void run();
};