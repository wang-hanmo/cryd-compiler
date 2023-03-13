#include <global_variable_localization.h>
#include <linear_ir_manager.h>
//#define DEBUG_INFO_OPTIMIZER
void GlobalVarLocalization::run()
{
    std::cout << "Running pass: Global Variable Localization" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    //计算函数副作用信息
    m_idfa->compute_side_effect_info();
    auto global_var_count = m_idfa->get_global_var_count();
    auto func_count = m_idfa->get_func_count();
    int main_func_index = -1;
    IRUnit* main_func = nullptr;
    m_should_localization.clear();
    m_should_localization.resize(global_var_count);
    for (int j = 0; j < func_count; ++j) {
        if (m_idfa->get_procedure(j)->get_definations_const().front().a()->global_sym()->get_name() == "main") {
            main_func = m_idfa->get_procedure(j);
            main_func_index = j;
            break;
        }
    }
    assert(main_func != nullptr);
    for (int i = 0; i < global_var_count; ++i) {
        int use_count = 0;
        for (int j = 0; j < func_count; ++j) {
            if (m_idfa->get_global_var_def(j).test(i) || m_idfa->get_global_var_use(j).test(i)) {
                use_count++;
            }
        }
        //只在一个函数中被定值/使用，并且是main函数
        if (use_count == 1 && !m_idfa->is_direct_recursion_function(main_func->get_tag())) {
            if (m_idfa->get_global_var_def(main_func_index).test(i) || m_idfa->get_global_var_use(main_func_index).test(i)) {
                m_should_localization[i] = true;
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_symbol(m_idfa->get_global_var(i));
                std::cout << std::endl;
#endif
            }
        }
    }
    work_unit(main_func);
}
void GlobalVarLocalization::work_unit(IRUnit* unit)
{
    m_local_sym.clear();
    auto global_var_count = m_idfa->get_global_var_count();
    m_local_sym.resize(global_var_count);
    int index = unit->find_max_index_for_tlp_var()+1;
    for (int i = 0; i < global_var_count;++i) {
        if (m_should_localization[i]) {
            IRSymbol* new_sym = m_ir_sym_table->create_local(m_idfa->get_global_var(i)->basic_type(),index++);
            unit->get_definations().push_back(IRInstr::create_local_decl(new_sym));
            m_local_sym[i] = new_sym;
        }
    }
    //加入初值语句
    auto entry_bb = unit->get_entry();
    auto first_bb = unit->get_entry()->get_succ(0);
    auto new_bb = new IRBlock();
    new_bb->add_instr(IRInstr::create_block_goto());
    entry_bb->delete_edge(0);
    entry_bb->set_edge(0, new_bb);
    new_bb->set_edge(0, first_bb);
    for (int i = 0; i < global_var_count; ++i) {
        if (m_should_localization[i]) {
            if (m_idfa->get_global_var(i)->global_sym()->is_literally_initialized()) {
                new_bb->add_instr_to_front(IRInstr::create_assign(
                    m_local_sym[i],
                    m_ir_sym_table->create_value(m_local_sym[i]->basic_type(), m_idfa->get_global_var(i)->global_sym()->get_init_value().front().val)
                ));
            }
            else {
                new_bb->add_instr_to_front(IRInstr::create_assign(
                    m_local_sym[i],
                    m_ir_sym_table->create_value_0(m_local_sym[i]->basic_type())
                ));
            }
        }
    }
    //替换
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list())
            if (instr.type() != IRType::Call && instr.type() != IRType::CallWithRet) {

#ifdef DEBUG_INFO_OPTIMIZER
                bool changed = false;
#endif
                IRSymbol* sym = instr.a();
                if (sym != nullptr && sym->kind() == IRSymbolKind::Global && sym->is_non_array() && m_should_localization[sym->get_tag()]) {
#ifdef DEBUG_INFO_OPTIMIZER
                    changed = true;
#endif
                    instr.rebind_a(m_local_sym[sym->get_tag()]);
                }
                sym = instr.b();
                if (sym != nullptr && sym->kind() == IRSymbolKind::Global && sym->is_non_array() && m_should_localization[sym->get_tag()]) {
#ifdef DEBUG_INFO_OPTIMIZER
                    changed = true;
#endif
                    instr.rebind_b(m_local_sym[sym->get_tag()]);
                }
                sym = instr.r();
                if (sym != nullptr && sym->kind() == IRSymbolKind::Global && sym->is_non_array() && m_should_localization[sym->get_tag()]) {
#ifdef DEBUG_INFO_OPTIMIZER
                    changed = true;
#endif
                    instr.rebind_r(m_local_sym[sym->get_tag()]);
                }
#ifdef DEBUG_INFO_OPTIMIZER
                if (changed) {
                    LinearIRManager::print_ir_instr(instr);
                }
#endif
            }        
        for (int k = 0; k < now->out_degree(); ++k) {
            if (visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
    //删掉全局变量定义
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::VarDef) {
            auto& defs = unit.get_definations();
            for (auto instr = defs.begin(); instr != defs.end();) {
                if (instr->a()->is_non_array() && m_should_localization[instr->a()->get_tag()]) {
                    instr = defs.erase(instr);
                }
                else ++instr;
            }
        }
    }
}