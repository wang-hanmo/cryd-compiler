#include <array_visit_substitution_on_ssa.h>

using namespace std;

void ArrayVisitSubstitutionOnSSA::run()
{
    std::cout << "Running pass: Array Visit Substitution On SSA" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    visited.clear();
    assert(m_cfg != nullptr);
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            m_idfa->compute_side_effect_info();
            init_tag(&unit);
            set_tag(&unit);
            simplify_array_visit(unit.get_entry());
        }
    }
}


void ArrayVisitSubstitutionOnSSA::simplify_array_visit(IRBlock* block)
{
    visited.insert(block);
    for (int i = 0; i < block->out_degree(); ++i) {
        if(visited.find(block->get_succ(i)) == visited.end())
            simplify_array_visit(block->get_succ(i));
    }
    IRInstrList& instr_list = block->get_instr_list();
    // 存储某数组某元素当前的值
    vector<unordered_map<IRSymbol*, IRSymbol*>> array_map(max_tag);
    for (auto& instr: instr_list) {
        if (instr.type() == IRType::ArrayStore) {
            // r[a] = b
            for (auto& arr_map: array_map)
                arr_map.clear();
        } else if (instr.type() == IRType::ArrayLoad) {
            // r = a[b];
            auto r = instr.r();
            auto a = instr.a();
            auto b = instr.b();
            auto& arr = array_map[a->get_tag()];
            if (auto iter = arr.find(b);iter != arr.end()) {
                auto sym= iter->second;
                if (sym->is_value())
                    sym = m_ir_sym_table->create_value(sym->basic_type(), sym->value());
                instr.rebind_a(sym);
                instr.rewrite_type(IRType::Assign);
            } else {
                array_map[a->get_tag()].emplace(b, r);
            }
        } else if (instr.type() == IRType::RParam) {
            // 遇到函数调用且需要清空array_map
            auto a = instr.a();
            if (a->array_length() != IRArrayLength::IR_NONE_ARRAY)
                array_map[a->get_tag()].clear();
        } else if (instr.type() == IRType::Call || instr.type() == IRType::CallWithRet) {
            if (instr.a()->global_sym()->is_internal_function() || m_idfa->has_side_effect(instr.a()->get_tag())) {
                for (auto& arr_map: array_map)
                    arr_map.clear();
            }
        }
    }
}

void ArrayVisitSubstitutionOnSSA::ir_list_set_tag(IRInstrList& program)
{
    for(auto&instr:program) {
        auto r = instr.r();
        auto a = instr.a();
        if(r != nullptr && !r->is_non_array() && r->get_tag() == -1)
            r->set_tag(max_tag++);
        if(a != nullptr && !a->is_non_array() && a->get_tag() == -1)
            a->set_tag(max_tag++);
    }
}

void ArrayVisitSubstitutionOnSSA::set_tag(IRUnit* unit)
{
    max_tag = 0;
    ir_list_set_tag(unit->get_definations());
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        ir_list_set_tag(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void ArrayVisitSubstitutionOnSSA::init_tag(IRUnit* unit)
{
    ir_list_init_tag(unit->get_definations());
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        ir_list_init_tag(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void ArrayVisitSubstitutionOnSSA::ir_list_init_tag(IRInstrList& program)
{
    for(auto&instr:program) {
        auto r = instr.r();
        auto a = instr.a();
        if(r != nullptr && !r->is_non_array())
            r->set_tag(-1);
        if(a != nullptr && !a->is_non_array())
            a->set_tag(-1);
    }
}