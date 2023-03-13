#include <local_copy_propagation.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <cfg_manager.h>
#include <linear_ir_manager.h>
using namespace local_copy_propagation;
IRSymbol* LocalCopyPropagation::check_replace(IRSymbol* sym)
{
    if (sym->kind() == IRSymbolKind::Value)
        return sym;
    else if ((sym->kind() == IRSymbolKind::Global && m_idfa->global_symbol_optimization_is_valid())||
        sym->kind() == IRSymbolKind::Local|| sym->kind() == IRSymbolKind::Param || sym->kind() == IRSymbolKind::Temp) {
        CopyExpression expr(nullptr, sym);
        if (auto iter = m_expr_set.find(expr); iter != m_expr_set.end()) {
            if (iter->get_a()->kind() == IRSymbolKind::Value) {
                return m_ir_sym_table->create_value(iter->get_a()->basic_type(), iter->get_a()->value());
            }else 
            return iter->get_a();
        }else return sym;
    } else assert(false);
    return sym;
}
void LocalCopyPropagation::work_ir_list(IRInstrList& program,int unit_tag)
{
    m_expr_set.clear();
    for(auto& instr:program){
        switch (instr.type()){
        case IRType::Assign:
            instr.rebind_a(check_replace(instr.a()));
            if (instr.a() != nullptr && instr.a()->kind() == IRSymbolKind::Global) {
                break;
            } else if(instr.r() != nullptr && instr.r()->kind() == IRSymbolKind::Global){
                if(!m_idfa->global_symbol_optimization_is_valid())
                    break;
                //删除原先的赋值语句，插入新的赋值语句
                m_expr_set.erase(CopyExpression(nullptr, instr.r()));
                auto iter = m_expr_set.insert(CopyExpression(instr.a(), instr.r())).first;
                //维护用a替代了r的信息
                if (instr.a()->ssa_index() >= 0) {
                    m_replaced_vars[instr.a()->def_sym()->get_tag()].push_back(instr.r());
                }
            } else {
                //插入对应赋值语句
                auto iter = m_expr_set.insert(CopyExpression(instr.a(), instr.r())).first;
                //删除所有右值为本指令左值的CopyExpression
                if (instr.r()->ssa_index() >= 0) {
                    for (auto sym : m_replaced_vars[instr.r()->def_sym()->get_tag()])
                        m_expr_set.erase(CopyExpression(nullptr, sym));
                    m_replaced_vars[instr.r()->def_sym()->get_tag()].clear();
                }
                //维护用a替代了r的信息
                if (instr.a()->ssa_index() >= 0) {
                    m_replaced_vars[instr.a()->def_sym()->get_tag()].push_back(instr.r());
                }
            }
            break;
        case IRType::PhiFunc:
            break;
        case IRType::BinaryCalc:
            if (!is_relation_oper(instr.op())) {
                instr.rebind_a(check_replace(instr.a()));
                instr.rebind_b(check_replace(instr.b()));
            }
            else {
                if (instr.a()->kind() == IRSymbolKind::Temp)
                    instr.rebind_a(check_replace(instr.a()));
                if (instr.b()->kind() == IRSymbolKind::Temp)
                    instr.rebind_b(check_replace(instr.b()));
            }
            break;
        case IRType::UnaryCalc:
            instr.rebind_a(check_replace(instr.a()));
            break;
        case IRType::RParam:
            instr.rebind_a(check_replace(instr.a()));
            break;
        case IRType::ArrayStore:
            instr.rebind_r(check_replace(instr.r()));
            instr.rebind_a(check_replace(instr.a()));
            instr.rebind_b(check_replace(instr.b()));
            break;
        case IRType::ArrayLoad:
            instr.rebind_a(check_replace(instr.a()));
            instr.rebind_b(check_replace(instr.b()));
            break;
        case IRType::Call:
            [[fallthrough]];
        case IRType::CallWithRet:
            //删掉所有所调函数影响的变量的信息
            if (m_idfa->global_symbol_optimization_is_valid()) {
                if (!instr.a()->global_sym()->is_internal_function()) {
                    for (int i = 0; i < m_idfa->get_global_var_count(); ++i) {
                        IRSymbol* sym = m_idfa->get_global_var(i);
                        if (m_idfa->get_global_var_def(instr.a()->get_tag()).test(i))
                            m_expr_set.erase(CopyExpression(nullptr, sym));
                    }
                }
            }
            break;
        case IRType::Return:
            break;
        case IRType::ValReturn:
            instr.rebind_a(check_replace(instr.a()));
            break;
        case IRType::BlockGoto:
            break;
        case IRType::BlockCondGoto: {
            auto sym = check_replace(instr.a());
            //***可能发生内存泄漏***
            if (!sym->is_value())
                instr.rebind_a(sym);
            break;
        }
        default:
            break;
        }
    }
}
void LocalCopyPropagation::work_cfg(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while(!q.empty()){
        IRBlock* now=q.front();
        q.pop();
        work_ir_list(now->get_instr_list(),unit->get_tag());
        for(int k=1;k>=0;--k){
            if(now->get_succ(k)!=nullptr&& visited.find(now->get_succ(k))==visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void LocalCopyPropagation::run()
{
    std::cout << "Running pass: Local Copy Propagation " << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_idfa->global_symbol_optimization_is_valid()) {
        m_idfa->compute_side_effect_info();
    }
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef){
            m_sym_count = 0;
            for (auto& def_instr : unit.get_definations()) {
                if (def_instr.a()->ssa_index() >= 0) {
                    def_instr.a()->def_sym()->set_tag(m_sym_count++);
                }
            }
            m_replaced_vars.clear();
            m_replaced_vars.resize(m_sym_count);
            work_cfg(&unit);
        }
}
