#include <interprocedural_data_flow_analysis.h>

//计算函数副作用信息,会对函数和全局变量重新编号
void InterproceduralDataFlowAnalysis::compute_side_effect_info()
{
	procedure_numbering();
	global_var_numbering();
    m_has_side_effect.clear();
    m_affected_by_env.clear();
    m_converse_call_map.clear();
    m_call_map.clear();
    m_global_var_use.clear();
    m_global_var_def.clear();
    m_affected_by_env.resize(m_func_count);
    m_has_side_effect.resize(m_func_count);
    m_converse_call_map.resize(m_func_count);
    m_call_map.resize(m_func_count);
    m_converse_call_map.resize(m_func_count);
    for (int i = 0; i < m_func_count; ++i) {
        m_global_var_use.push_back(BitMap(m_global_var_count));
        m_global_var_def.push_back(BitMap(m_global_var_count));
    }
    //函数自身产生的副作用
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            build_call_map_and_local_side_effect(&unit);
        }
    }
    //传播由调用产生的副作用
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m_func_count;++i) {
            for (int caller : m_converse_call_map[i]) {
                const auto& res_def = m_global_var_def[caller] | m_global_var_def[i];
                if (res_def != m_global_var_def[caller]) {
                    m_global_var_def[caller] = res_def;
                    changed = true;
                }
            }
        }
    }
    changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m_func_count; ++i) {
            for (int caller : m_converse_call_map[i]) {
                if (!changed) {
                    const auto& res_use = m_global_var_use[caller] | m_global_var_use[i];
                    if (res_use != m_global_var_use[caller]) {
                        m_global_var_use[caller] = res_use;
                        changed = true;
                    }
                }
                else m_global_var_use[caller] |= m_global_var_use[i];
            }
        }
    }
    changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m_func_count; ++i) {
            if (!m_has_side_effect[i])
                continue;
            for (int caller : m_converse_call_map[i]) {
                if (!m_has_side_effect[caller]) {
                    changed = true;
                    m_has_side_effect[caller] = true;
                }
            }
        }
    }
    changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m_func_count; ++i) {
            if (!m_affected_by_env[i])
                continue;
            for (int caller : m_converse_call_map[i]) {
                if (!m_affected_by_env[caller]) {
                    changed = true;
                    m_affected_by_env[caller] = true;
                }
            }
        }
    }
}
//对函数进行编号，放入tag中
void InterproceduralDataFlowAnalysis::procedure_numbering()
{
    m_procedure.clear();
    m_func_count = 0;
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            unit.get_definations().begin()->a()->set_tag(m_func_count);
            unit.set_tag(m_func_count++);
            m_procedure.push_back(&unit);
        }
    }
}
//对非数组的全局变量进行编号，放入tag中
void InterproceduralDataFlowAnalysis::global_var_numbering()
{
    m_global_var.clear();
    
    m_global_var_count = 0;
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::VarDef) {
            for (auto& instr : unit.get_definations()) {
                if (instr.a()->is_non_array()) {
                    instr.a()->set_tag(m_global_var_count++);
                    m_global_var.push_back(instr.a());
                }
            }
        }
}
void InterproceduralDataFlowAnalysis::build_call_map_and_local_side_effect(IRUnit* unit)
{
    const int func_index = unit->get_tag();
    bool func_has_side_effect = false;
    bool func_affected_by_env = false;
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list_const()) {
            if (instr.r() != nullptr) {
                //全局变量定值
                if (instr.r()->kind() == IRSymbolKind::Global) {
                    func_has_side_effect = true;
                    if (instr.r()->is_non_array()) {
                        m_global_var_def[func_index].set(instr.r()->get_tag());
                    }
                }
                //参数数组定值
                if (instr.r()->kind() == IRSymbolKind::Param && instr.r()->array_length() >= 0)
                    func_has_side_effect = true;   
            }
            //全局变量、数组访问
            for (auto sym : { instr.a(),instr.b() }) {
                if (sym != nullptr && sym->kind() == IRSymbolKind::Global &&
                    sym->global_sym()->get_kind() == VarKind::Global) {
                    func_affected_by_env = true;
                }
            }
            //全局变量访问
            if (instr.a() != nullptr && instr.a()->kind() == IRSymbolKind::Global && instr.a()->is_non_array() &&
                instr.a()->global_sym()->get_kind()==VarKind::Global) {
                m_global_var_use[func_index].set(instr.a()->get_tag());
            }
            if (instr.b() != nullptr && instr.b()->kind() == IRSymbolKind::Global && instr.b()->is_non_array() &&
                instr.b()->global_sym()->get_kind() == VarKind::Global) {
                m_global_var_use[func_index].set(instr.b()->get_tag());
            }
            if (instr.type() == IRType::Call || instr.type() == IRType::CallWithRet) {
                if (!(instr.a()->global_sym()->is_internal_function())) {
                    m_converse_call_map[instr.a()->get_tag()].insert(unit->get_tag());
                    m_call_map[unit->get_tag()].insert(instr.a()->get_tag());
                }
                else {
                    func_has_side_effect = true;
                    func_affected_by_env = true;
                }
            }
        }
        for (int k = 0; k < now->out_degree(); ++k) {
            if (visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
    m_has_side_effect[unit->get_tag()] = func_has_side_effect;
    m_affected_by_env[unit->get_tag()] = func_affected_by_env;
}