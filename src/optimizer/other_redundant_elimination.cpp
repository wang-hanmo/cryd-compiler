#include <other_redundant_elimination.h>
#include <linear_ir_manager.h>
//#define DEBUG_INFO_OPTIMIZER
void OtherRedundantElimination::run()
{
    std::cout << "Running pass: Other Redundant Elimination" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            work_unit(&unit);
        }
    }
}
//删除冗余的memset
void OtherRedundantElimination::work_block(IRBlock* block)
{
    const int max_array_length = 64;
    IRSymbol* target_sym = nullptr;
    bool is_initialized[max_array_length] = {};
    IRInstrList::iterator memset_iter;
    auto& instr_list = block->get_instr_list();
    for (auto instr = instr_list.begin(); instr != instr_list.end(); ++instr) {
        if (target_sym == nullptr) {
            if (instr->type() == IRType::Call && instr->a()->global_sym()->is_internal_function()) {
                if (instr->a()->global_sym()->name() == std::string("memset"))
                {
                    auto array_param = instr;
                    std::advance(array_param, -3);
                    assert(array_param->type() == IRType::RParam);
                    if (array_param->a()->is_array() && array_param->a()->array_length() < max_array_length) {
                        target_sym = array_param->a();
                    }
                    memset_iter = instr;
                }
            }
        } else {
            if (instr->type() == IRType::ArrayStore && 
                instr->r() == target_sym && 
                instr->a()->is_value() && 
                instr->a()->int_value() < max_array_length) {
                is_initialized[instr->a()->int_value()] = true;
            } else {
                bool fully_initialized = true;
                for (int i = 0; i < target_sym->array_length(); ++i) {
                    if (!is_initialized[i]) {
                        fully_initialized = false;
                        break;
                    }
                }
                //被重新初始化了一遍，则删掉memset
                if (fully_initialized) {
                    auto iter = memset_iter;
                    std::advance(iter, -3);
                    for (int i = 0; i < 4; ++i) {
                        iter = instr_list.erase(iter);
                    }
                }
                return;
            }
        }
    }
}
void OtherRedundantElimination::work_unit(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        work_block(now);
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
