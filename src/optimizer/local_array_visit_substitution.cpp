#include <local_array_visit_substitution.h>

using namespace std;

void LocalArrayVisitSubstitution::run()
{
    std::cout << "Running pass: Local Array Visit Substitution" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    visited.clear();
    assert(m_cfg != nullptr);
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            init_tag(&unit);
            set_tag(&unit);
            simplify_array_visit(unit.get_entry());
        }
    }
}


void LocalArrayVisitSubstitution::simplify_array_visit(IRBlock* block)
{
    visited.insert(block);
    for (int i = 0; i < block->out_degree(); ++i) {
        if(visited.find(block->get_succ(i)) == visited.end())
            simplify_array_visit(block->get_succ(i));
    }
    IRInstrList& instr_list = block->get_instr_list();
    // 存储某数组某元素当前的值
    vector<unordered_map<int, IRSymbol*>> array_map(max_tag);
    for (auto& instr: instr_list) {
        if (instr.type() == IRType::ArrayStore) {
            // r[a] = b
            auto r = instr.r();
            auto a = instr.a();
            auto b = instr.b();
            // 简单处理
            if (r->kind() == IRSymbolKind::Global || unable.find(r) != unable.end() ||
            (r->kind() == IRSymbolKind::Param || r->array_length() == 0))
                continue;
            if (a->kind() == IRSymbolKind::Value) {
                // 这里先简单处理下，只考虑b是temp和value的情况
                if (b->kind() == IRSymbolKind::Temp|| b->kind() == IRSymbolKind::Value)
                    array_map[r->get_tag()][a->value().int_value]=b;
                else
                    array_map[r->get_tag()].erase(a->value().int_value);
            } else {
                array_map[r->get_tag()].clear();
            }
        } else if (instr.type() == IRType::ArrayLoad) {
            // r = a[b];
            auto r = instr.r();
            auto a = instr.a();
            auto b = instr.b();
            if (a->kind() != IRSymbolKind::Param && a->array_length() == 0)
                continue;
            if (b->kind() == IRSymbolKind::Value) {
                auto& arr = array_map[a->get_tag()];
                if (auto iter = arr.find(b->value().int_value);iter != arr.end()) {
                    auto sym= iter->second;
                    if (sym->is_value())
                        sym = m_ir_sym_table->create_value(sym->basic_type(), sym->value());
                    instr.rebind_a(sym);
                    instr.rewrite_type(IRType::Assign);
                }
            }
        } else if (instr.type() == IRType::RParam) {
            // 遇到函数调用且需要清空array_map
            auto a = instr.a();
            if (a->array_length() != IRArrayLength::IR_NONE_ARRAY)
                array_map[a->get_tag()].clear();
        }
        // } else if (instr.type() == IRType::Assign || instr.type() == IRType::BinaryCalc) {
        //     // i32* r = i32* a   |  i32* r = i32* a + 1   
        //     // 所有local、temp数组指针都指向local数组或者是函数形参数组指针
        //     auto r = instr.r();
        //     auto a = instr.a();
        //     if (r->array_length() == IRArrayLength::IR_ARRAY_POINTER) {
        //         if (a->kind() == IRSymbolKind::Param || a->array_length() > 0)
        //             array_pointer.emplace(r, a);
        //         else {
        //             assert(array_pointer.find(a) != array_pointer.end());
        //             auto base = array_pointer.at(a);
        //             array_pointer.emplace(r, base);
        //         }
        //     }
        // }
    }
}

void LocalArrayVisitSubstitution::ir_list_set_tag(IRInstrList& program)
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

void LocalArrayVisitSubstitution::set_tag(IRUnit* unit)
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
void LocalArrayVisitSubstitution::init_tag(IRUnit* unit)
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
void LocalArrayVisitSubstitution::work_before_simplify(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        auto instr_list = now->get_instr_list();
        for (auto& instr: instr_list) {
            if (instr.type() == IRType::Assign || instr.type() == IRType::BinaryCalc) {
                auto a = instr.a();
                if (a->kind() == IRSymbolKind::Param || a->array_length() > 0)
                    unable.emplace(a);
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
void LocalArrayVisitSubstitution::ir_list_init_tag(IRInstrList& program)
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