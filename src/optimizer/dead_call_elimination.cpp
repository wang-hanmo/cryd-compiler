#include <dead_call_elimination.h>
#include <string>
#include <iostream>
#include <cassert>
void DeadCallElimination::run()
{
    std::cout << "Running pass: Dead Call Elimination" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    assert(m_cfg != nullptr);
    build_side_effect_info();
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            dead_call_elimination(&unit);
            //dead_param_mark(&unit);
        }
    }
    //dead param elimination
}
void DeadCallElimination::dead_param_mark(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto instr:now->get_instr_list()) {
            
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void DeadCallElimination::dead_call_elimination(IRUnit* unit)
{
    std::set<int> calling_set;
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto iter = now->get_instr_list().begin(); iter != now->get_instr_list().end();) {
            if (iter->type() == IRType::Call && 
                !(iter->a()->global_sym()->is_internal_function()) && 
                !m_has_side_effect[iter->a()->get_tag()]) {
                int param_count = iter->b()->int_value();
                for (int i = 0; i < param_count; ++i)
                    --iter;
                for (int i = 0; i <= param_count; ++i)
                    iter = now->get_instr_list().erase(iter);
            }
            else ++iter;
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void DeadCallElimination::build_side_effect_info()
{
    m_has_side_effect.clear();
    m_func_count = 0;
    m_anti_calling_map.clear();
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            unit.get_definations().begin()->a()->set_tag(m_func_count);
            unit.set_tag(m_func_count++);
            m_anti_calling_map.push_back({});
        }
    }
    std::queue<int> q;
    //维护由函数自身产生的side affect
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            m_has_side_effect.push_back(build_calling_map(&unit));
            if (m_has_side_effect.back())
                q.push(unit.get_tag());
        }
    }
    //将side affect顺着调用图传播
    while (!q.empty()) {
        int now = q.front();
        q.pop();
        for (auto next : m_anti_calling_map[now]) {
            if (!m_has_side_effect[next]) {
                m_has_side_effect[next] = true;
                q.push(next);
            }
        }
    }
}
bool DeadCallElimination::has_side_effect(IRSymbol* sym)
{
    if (sym->kind() == IRSymbolKind::Global)
        return true;
    if (sym->kind() == IRSymbolKind::Param && sym->array_length() >= 0)
        return true;
    if (sym->kind() == IRSymbolKind::Local && sym->array_length() == IRArrayLength::IR_ARRAY_POINTER)
        return true;
    return false;
}
bool DeadCallElimination::build_calling_map(IRUnit* unit)
{
    bool func_has_side_effect = false;
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list_const()) {
            if (instr.r() != nullptr && has_side_effect(instr.r()))
                func_has_side_effect = true;
            if (instr.type() == IRType::Call || instr.type() == IRType::CallWithRet) {
                if (!(instr.a()->global_sym()->is_internal_function())) {
                    m_anti_calling_map[instr.a()->get_tag()].insert(unit->get_tag());
                }else {
                    func_has_side_effect = true;
                }
            }
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
    return func_has_side_effect;
}