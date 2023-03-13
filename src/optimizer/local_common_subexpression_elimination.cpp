#include <local_common_subexpression_elimination.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <cfg_manager.h>
#include <linear_ir_manager.h>
using namespace lcse;
//#define DEBUG_INFO_OPTIMIZER
#ifdef DEBUG_INFO_OPTIMIZER
static std::ostream& operator <<(std::ostream& os, IRSymbol* sym) {
    LinearIRManager::print_ir_symbol(sym, os);
    return os;
}
static std::ostream& operator <<(std::ostream& os, const IRInstr& instr) {
    LinearIRManager::print_ir_instr(instr, os);
    return os;
}
#endif 
int LocalCommonSubexpressionElimination::find_or_new_vn(IRSymbol* sym) {
    if (auto iter = m_vn_of_sym.find(sym); iter == m_vn_of_sym.end()) {
#ifdef DEBUG_INFO_OPTIMIZER
        /*
        if(sym!=nullptr)
            std::cout << sym <<" vn = "<< m_vn << std::endl;
        else std::cout << "nullptr vn = " << m_vn << std::endl;*/
#endif
        return m_vn_of_sym[sym] = m_vn++;
    }
    else return iter->second;
}
void LocalCommonSubexpressionElimination::ir_list_eliminate(IRInstrList& program)
{
    m_vn = 0;
    m_vn_to_sym.clear();
    m_vn_of_sym.clear();
    m_vn_of_expr.clear();
    for(auto& instr:program){
        switch (instr.type()){
        case IRType::Assign:
            m_vn_of_sym[instr.r()] = find_or_new_vn(instr.a());
            break;
        case IRType::BinaryCalc:
            [[fallthrough]];
        case IRType::UnaryCalc:{
            int vn_a = find_or_new_vn(instr.a());
            int vn_b = find_or_new_vn(instr.b());
            Expression expr(vn_a, vn_b, instr.op());
#ifdef DEBUG_INFO_OPTIMIZER
            //std::cout << vn_a << " " << vn_b << std::endl;
#endif        
            if (auto iter = m_vn_of_expr.find(expr); iter == m_vn_of_expr.end()) {
                m_vn_of_expr[expr] = m_vn;
                m_vn_of_sym[instr.r()] = m_vn;
                m_vn_to_sym[m_vn] = instr.r();
#ifdef DEBUG_INFO_OPTIMIZER
                //std::cout << instr << " vn = " << m_vn << std::endl;       
#endif
                m_vn++;
            } else {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << instr << " vn = " << m_vn << std::endl;
#endif
                m_vn_of_sym[instr.r()] = iter->second;
                instr.rebind_a(m_vn_to_sym[iter->second]);
                instr.rewrite_type(IRType::Assign);
            }
            break;
        }
        case IRType::Call:
            [[fallthrough]];
        case IRType::CallWithRet:
            //删掉所有所调函数影响的变量的信息
            if (m_idfa->global_symbol_optimization_is_valid()) {
                if (!instr.a()->global_sym()->is_internal_function()) {
                    for (int i = 0; i < m_idfa->get_global_var_count(); ++i) {
                        IRSymbol* sym = m_idfa->get_global_var(i);
                        if (m_idfa->get_global_var_def(instr.a()->get_tag()).test(i))
                            m_vn_of_sym.erase(sym);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}
void LocalCommonSubexpressionElimination::control_flow_graph_eliminate(IRBlock* entry)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(entry);
    while(!q.empty()){
        IRBlock* now=q.front();
        q.pop();
        ir_list_eliminate(now->get_instr_list());
        for(int k=0;k<now->out_degree(); ++k) {
            if(visited.find(now->get_succ(k))==visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void LocalCommonSubexpressionElimination::run()
{
    std::cout << "Running pass: Local Common Subexpression Elimination " << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_idfa->global_symbol_optimization_is_valid())
        m_idfa->compute_side_effect_info();
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef)
             control_flow_graph_eliminate(unit.get_entry());
}
