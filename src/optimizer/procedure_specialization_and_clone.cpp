#include <procedure_specialization_and_clone.h>
#include <string>
#include <iostream>
#include <cassert>
void ProcedureSpecializationAndClone::run()
{
    std::cout << "Running pass: Procedure Specialization And Clone" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    assert(m_cfg != nullptr);
    m_func_count = 0;
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef) {
            unit.get_definations_const().begin()->a()->set_tag(m_func_count);
            unit.set_tag(m_func_count++);
        }
    m_return_value_use_info.clear();
    m_return_instrs.clear();
    m_return_value_use_info.resize(m_func_count);
    m_return_instrs.resize(m_func_count);
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef)
            init_calling_info(&unit);
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            auto func_sym = (unit.get_definations_const().cbegin())->a();
            //函数的返回值总是不被使用的话，把类型改为void型
            if (func_sym->basic_type()!=BasicType::Void && m_return_value_use_info[unit.get_tag()]==2) {
                func_sym->set_basic_type(BasicType::Void);
                for (auto instr : m_return_instrs[unit.get_tag()]){
                    instr->rewrite_type(IRType::Return);
                }
            }
        }
    }
}
void ProcedureSpecializationAndClone::init_calling_info(IRUnit* unit)
{
    bool func_has_side_effect = false;
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list()) {
            if (instr.type() == IRType::Call && !instr.a()->global_sym()->is_internal_function())
                m_return_value_use_info[instr.a()->get_tag()] |= 2;
            else if (instr.type() == IRType::CallWithRet && !instr.a()->global_sym()->is_internal_function())
                m_return_value_use_info[instr.a()->get_tag()] |= 1;
            else if (instr.type() == IRType::Return)
                m_return_instrs[unit->get_tag()].push_back(&instr);
            else if (instr.type() == IRType::ValReturn) {
                m_return_instrs[unit->get_tag()].push_back(&instr);
            }
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}