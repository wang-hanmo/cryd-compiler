#include <revert_ssa.h>
#include <cfg_manager.h>
#include <string>
#include <iostream>
#include <cassert>
void RevertSSA::rewrite_instructions(IRBlock* block)
{
    //删除phi函数
    while (!block->get_instr_list().empty()&&block->get_instr_list().front().type() == IRType::PhiFunc)
        block->get_instr_list().pop_front();
    for (auto instr = block->get_instr_list().begin(); instr != block->get_instr_list().end();) {
        //重写使用
        bool rewrite_a = false;
        bool rewrite_b = false;
        bool rewrite_r = false;
        switch (instr->type()) {
        case IRType::BinaryCalc:
            rewrite_a = true;
            rewrite_b = true;
            break;
        case IRType::UnaryCalc:
            rewrite_a = true;
            break;
        case IRType::Assign:
            rewrite_a = true;
            break;
        case IRType::RParam:
            rewrite_a = true;
            break;
        case IRType::ArrayStore:
            rewrite_r = true;
            rewrite_a = true;
            rewrite_b = true;
            break;
        case IRType::ArrayLoad:
            rewrite_a = true;
            rewrite_b = true;
            break;
        case IRType::Call:
        case IRType::CallWithRet:
        case IRType::Return:
            break;
        case IRType::ValReturn:
            rewrite_a = true;
            break;
        case IRType::BlockGoto:
            break;
        case IRType::BlockCondGoto:
            rewrite_a = true;
            break;
        case IRType::PhiFunc:
            //不允许在基本块的非开头位置出现phi函数
            assert(false);
            //assert(instr->a()->phi_params().size()==1);
            //instr = block->get_instr_list().erase(instr);
            break;
        default:
            assert(false);
            Optimizer::optimizer_error("RevertSSA: Unexpected instruction!");
            break;
        }
        if (rewrite_a && instr->a()->ssa_index() > -1)
            instr->rebind_a(instr->a()->def_sym());
        if (rewrite_b && instr->b()->ssa_index() > -1)
            instr->rebind_b(instr->b()->def_sym());
        if (rewrite_r && instr->r()->ssa_index() > -1)
            instr->rebind_r(instr->r()->def_sym());
        //重写定值
        if (instr->type() == IRType::Assign || instr->type() == IRType::ArrayLoad) {
            if (instr->r()->kind() == IRSymbolKind::Local|| instr->r()->kind() == IRSymbolKind::Param) {
                instr->rebind_r(instr->r()->def_sym());
            }
        }
        if (instr->type() == IRType::Assign && instr->r() == instr->a()) {
            instr = block->get_instr_list().erase(instr);
        }
        else ++instr;
    }
    for (auto child : block->get_idom_child()) {
        //递归
        rewrite_instructions(child);
    }
}
void RevertSSA::rewrite_defs(IRUnit* unit)
{
    for (auto& instr : unit->get_definations()) {
        if(instr.a()->kind()==IRSymbolKind::Local|| instr.a()->kind() == IRSymbolKind::Param)
            instr.rebind_a(instr.a()->def_sym());
    }
}
void RevertSSA::run()
{
    std::cout << "Running pass: Revert SSA" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_ir_sym_table == nullptr) {
        Optimizer::optimizer_error("No IR symbol table specified");
    }
    //对每个unit进行逆SSA化
    for (auto& unit : *m_cfg) {
        if (unit.get_type()==IRUnitType::FuncDef) {
            //计算支配树,O(nlogn),n为基本块数
            CFGManager::build_dominator_tree(&unit);
            //重写定义语句
            rewrite_defs(&unit);
            //对支配树做dfs，重写指令
            rewrite_instructions(unit.get_entry());
        }
    }
}