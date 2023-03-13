#include <block_simplification.h>
#include <string>
#include <iostream>
#include <cassert>
void BlockSimplification::run()
{
    std::cout << "Running pass: Block Simplification" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    assert(m_cfg != nullptr);
    visited.clear();
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            unit.set_dominator_tree_info_valid(false);
            //1、进行不可达块删除
            unreachable_block_elimination(unit.get_exit(), unit.get_entry());
            //2、进行空块删除
            visited.clear();
            empty_block_elimination(unit.get_entry());
            //3、进行相邻块合并
            visited.clear();
            block_merge(unit.get_entry());
        }
    }
}
bool BlockSimplification::is_block_empty(IRBlock* block)
{
    if (block->get_instr_list_const().size() == 0) {
        assert(false);//不允许出现没有任何指令的空块，最少也需要有一个跳转指令
        return true;
    }
    if (block->get_instr_list_const().size() == 1&& block->get_instr_list_const().back().type()==IRType::BlockGoto)
        return true;
    return false;
}
//不可达基本块删除
void BlockSimplification::unreachable_block_elimination(IRBlock* block, IRBlock* entry)
{
    visited.insert(block);
    auto pred_set = block->get_pred();
    for (auto pred: pred_set) {
        if (visited.find(pred) == visited.end())
            unreachable_block_elimination(pred,entry);
    }
    //判断不可达基本块并进行删除
    if (block != entry && block->in_degree() == 0) {
        auto out_degree = block->out_degree();
        for (int i = 0; i < out_degree; ++i)
            block->delete_edge(i);
        delete block;
    }
}
void BlockSimplification::block_merge(IRBlock* block)
{
    visited.insert(block);
    for (int i = 0; i < block->out_degree(); ++i) {
        if (visited.find(block->get_succ(i)) == visited.end())
            block_merge(block->get_succ(i));
    }
    //合并条件：当前块出度为1，子块入度为1。且当前块和子块都不能是entry 和 exit
    if (!block->is_entry() && block->out_degree() == 1) {
        IRBlock* to = block->get_succ();
        if (!to->is_exit() && to->in_degree() == 1) {
            /*有bug，勿使用
            //删除to中的phi函数，将它们重写为赋值语句
            for (auto& instr : to->get_instr_list()) {
                if (instr.type() == IRType::PhiFunc) {
                    assert(instr.a()->phi_params().size() == 1);
                    instr.rebind_a(instr.a()->phi_params().front().sym);
                    instr.rewrite_type(IRType::Assign);
                }
                else break;
            }*/
            
            if (block->get_instr_list_const().crbegin()->type() == IRType::BlockGoto)
                block->get_instr_list().pop_back();
            block->get_instr_list().splice(block->get_instr_list().end(), to->get_instr_list());
            block->delete_edge(0);
            //to的out_degree函数在for循环中会被修改，所以要先计算后使用
            const int out_degree = to->out_degree();
            for (int k = 0; k < out_degree; ++k) {
                auto next = to->get_succ(k);
                to->delete_edge(k);
                block->set_edge(k, next);
            }
            delete to;
        }
        
    }
}
void BlockSimplification::empty_block_elimination(IRBlock* block)
{
    visited.insert(block);
    for (int i = 0; i < block->out_degree(); ++i) {
        if(visited.find(block->get_succ(i))==visited.end())
            empty_block_elimination(block->get_succ(i));
    }
    //空块条件：出度为1，仅存在goto语句
    if (!block->is_entry()&&block->out_degree() == 1 && is_block_empty(block)){
        //删除空块
        IRBlock* to = block->get_succ();
        block->delete_edge(0);
        for (IRBlock* from : block->get_pred()){
            //当前节点在前驱节点边表中的下标
            int pre_index=-1;
            assert(!(from->get_succ(0) == block && from->get_succ(1) == block));
            if (from->get_succ(0) == block)
                pre_index = 0;
            else if (from->get_succ(1) == block)
                pre_index = 1;
            assert(pre_index != -1);
            from->delete_edge_oneway(pre_index);
            //建立新边
            from->set_edge(pre_index, to);
            //检测是否存在相同边，如果存在，则将类型改为uncond goto
            if (from->out_degree() == 2 && from->get_succ(0) == from->get_succ(1)) {
                from->delete_edge(1);
                assert(from->get_instr_list_const().crbegin()->type() == IRType::BlockCondGoto);
                from->get_instr_list().rbegin()->rewrite_type(IRType::BlockGoto);
            }
        }
        //删除本节点
        delete block;
    }
}
