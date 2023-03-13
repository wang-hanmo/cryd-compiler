#include <global_common_subexpression_elimination.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <cfg_manager.h>
#include <linear_ir_manager.h>
using namespace lcse;
using namespace gcse;
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
int GlobalCommonSubexpressionElimination::find_or_new_vn(IRSymbol* sym) {
    if (auto iter = m_vn_of_sym.find(sym); iter == m_vn_of_sym.end()) {
        assert(sym==nullptr||sym->is_value()||sym->is_global());
#ifdef DEBUG_INFO_OPTIMIZER
        if(sym!=nullptr)
            std::cout << sym <<" vn = "<< m_vn << std::endl;
        else std::cout << "nullptr vn = " << m_vn << std::endl;
#endif
        return m_vn_of_sym[sym] = m_vn++;
    }
    else return iter->second;
}
int GlobalCommonSubexpressionElimination::find_vn(IRSymbol* sym) {
    if (auto iter = m_vn_of_sym.find(sym); iter == m_vn_of_sym.end())
        assert(false);
    else return iter->second;
    return -1;
}
int GlobalCommonSubexpressionElimination::new_vn(IRSymbol* sym) {
#ifdef DEBUG_INFO_OPTIMIZER
        if(sym!=nullptr)
            std::cout << sym <<" vn = "<< m_vn << std::endl;
        else std::cout << "nullptr vn = " << m_vn << std::endl;
#endif
    assert(m_vn_of_sym.find(sym) == m_vn_of_sym.end());
    return m_vn_of_sym[sym] = m_vn++;
}
std::pair<bool, CallExpression> GlobalCommonSubexpressionElimination::get_call_expression(const std::vector<IRSymbol*>& params, IRSymbol* func)
{
    if(func->global_sym()->is_internal_function())
        return { false,CallExpression() };
    if (m_idfa->has_side_effect(func->get_tag())|| m_idfa->affected_by_env(func->get_tag())) {
        return { false,CallExpression() };
    }
    CallExpression res;
    res.vn_func = find_or_new_vn(func);
    for (auto param : params) {
        if (param->is_global()||param->is_array_or_pointer())
            return { false,CallExpression() };
        res.vn_rparam.push_back(find_or_new_vn(param));
    }
    return { true,res };
}
void GlobalCommonSubexpressionElimination::work_block(IRInstrList& program)
{
    for (auto instr = program.begin(); instr != program.end();++instr) {
        switch (instr->type()){
        case IRType::PhiFunc: {
            int vn = -1;
            for (auto& phi_param : instr->a()->phi_params()) {
                if (auto iter = m_vn_of_sym.find(phi_param.sym);iter == m_vn_of_sym.end()) {
                    vn = -2;//缺失值编号的phi参数
                    break;
                }
                else {
                    if (vn == -1) {
                        vn = iter->second;
                    }
                    else if (vn != iter->second)
                        vn = -3;//不同值编号的phi参数
                }
            }
            if (vn == -2) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "unnumbered phi param" << std::endl;   
#endif
                new_vn(instr->r());
            }else if (vn == -1) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "no valid path to phi param" << std::endl;
#endif
                new_vn(instr->r());
            }
            else if (vn == -3) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "diffirent numbered phi param"<< std::endl;
#endif
                new_vn(instr->r());
            } else {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "same vn for all phi params" << vn << std::endl;  
#endif
                m_vn_of_sym[instr->r()] = vn;
            }
            break;
        }
        case IRType::Assign: {
            int vn;
            if (instr->r() != nullptr && instr->r()->is_global())
                break;
            if (instr->a() != nullptr && instr->a()->is_global()) {
                vn = new_vn(instr->r());
            }else {
                vn = find_or_new_vn(instr->a());
                m_vn_of_sym[instr->r()] = vn;
            }
            
#ifdef DEBUG_INFO_OPTIMIZER
            std::cout << instr->r() << " vn = " << vn << std::endl;
#endif
            break;
        }
        case IRType::BinaryCalc:
            [[fallthrough]];
        case IRType::UnaryCalc:{
            if (instr->a() != nullptr && instr->a()->is_global() ||
                instr->b() != nullptr && instr->b()->is_global()) {
                new_vn(instr->r());
                break;
            }
            int vn_a = find_or_new_vn(instr->a());
            int vn_b = find_or_new_vn(instr->b());
            Expression expr(vn_a, vn_b, instr->op());
#ifdef DEBUG_INFO_OPTIMIZER
            //std::cout << vn_a << " " << vn_b << std::endl;
#endif        
            if (auto iter = m_vn_of_expr.find(expr); iter == m_vn_of_expr.end()) {
                int vn = new_vn(instr->r());
                if (!is_relation_oper(instr->op())) {
                    m_vn_of_expr[expr] = vn;
                    m_vn_to_sym[vn] = instr->r();
                }

#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "expr " << vn_a << " " << (int)instr->op() << " " << vn_b << " vn = " << vn << std::endl;
#endif
            } else {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "find expr " << vn_a << " " << (int)instr->op() << " " << vn_b<<" vn = "<<iter->second << std::endl;
#endif
                m_vn_of_sym[instr->r()] = iter->second;
                instr->rebind_a(m_vn_to_sym[iter->second]);
                instr->rewrite_type(IRType::Assign);
            }
            break;
        }
        case IRType::ArrayLoad:
            new_vn(instr->r());
            break;
        case IRType::CallWithRet: {
            //删除冗余函数调用
            std::vector<IRSymbol*> rparams;
            int rparam_count = instr->b()->int_value();
            auto iter = instr;
            std::advance(iter, -rparam_count);
            for (int i = 0; i < rparam_count; ++i, ++iter) {
                assert(iter->type() == IRType::RParam);
                rparams.push_back(iter->a());
            }
            auto [succ, call_expr] = get_call_expression(rparams,instr->a());
            if (succ) {
                if (auto iter = m_vn_of_call.find(call_expr); iter == m_vn_of_call.end()) {
                    int vn = new_vn(instr->r());
                    m_vn_of_call[call_expr] = vn;
                    m_vn_to_sym[vn] = instr->r();
                } else { 
                    std::advance(instr, -rparam_count);
                    for (int i = 0; i < rparam_count; ++i)
                        instr = program.erase(instr);
                    m_vn_of_sym[instr->r()] = iter->second;
                    instr->rebind_a(m_vn_to_sym[iter->second]);
                    instr->rewrite_type(IRType::Assign);
                }
            }else new_vn(instr->r());
            break;
        }
        default:
            break;
        }
    }
}

void GlobalCommonSubexpressionElimination::roll_back(IRInstrList& program)
{
    for (auto instr = program.rbegin(); instr != program.rend();++instr) {
        switch (instr->type()) {
        case IRType::Assign: {
#ifdef DEBUG_INFO_OPTIMIZER
            auto iter = m_vn_of_sym.find(instr->r());
            assert(instr->r()->is_global()||iter != m_vn_of_sym.end());
            std::cout << "erase vn " << iter->second << " sym=" << instr->r() << std::endl;
#endif
            m_vn_of_sym.erase(instr->r());
            break;
        }
        case IRType::BinaryCalc:
            [[fallthrough]];
        case IRType::UnaryCalc: {
            if (instr->a() != nullptr && instr->a()->is_global() ||
                instr->b() != nullptr && instr->b()->is_global()) {
                m_vn_of_sym.erase(instr->r());
                break;
            }
            int vn_a = find_or_new_vn(instr->a());
            int vn_b = find_or_new_vn(instr->b());
            Expression expr(vn_a, vn_b, instr->op());  
            if (auto iter = m_vn_of_expr.find(expr); iter != m_vn_of_expr.end()) {
#ifdef DEBUG_INFO_OPTIMIZER
                auto it = m_vn_of_sym.find(instr->r());
                assert(it != m_vn_of_sym.end());
                std::cout << "erase vn " << iter->second << " sym=" << instr->r() << std::endl;
                std::cout << "erase vn " << iter->second << " expr=" << vn_a<<" " << (int)instr->op() <<" " << vn_b << std::endl;
#endif
                m_vn_of_sym.erase(instr->r());
                m_vn_to_sym.erase(iter->second);
                m_vn_of_expr.erase(iter);
            }
            break;
        }
        case IRType::PhiFunc:
            [[fallthrough]];
        case IRType::ArrayLoad:
            m_vn_of_sym.erase(instr->r());
            break;
        case IRType::CallWithRet: {
            m_vn_of_sym.erase(instr->r());
            //删除冗余函数调用
            std::vector<IRSymbol*> rparams;
            int rparam_count = instr->b()->int_value();
            auto iter = instr;
            std::advance(iter, rparam_count);
            for (int i = 0; i < rparam_count; ++i, --iter) {
                assert(iter->type() == IRType::RParam);
                rparams.push_back(iter->a());
            }
            auto [succ, call_expr] = get_call_expression(rparams, instr->a());
            if (succ) {
                m_vn_of_call.erase(call_expr);
            }
            break;
        }
        default:
            break;
        }
    }
}
void GlobalCommonSubexpressionElimination::work(IRBlock* block)
{
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "Work B" << block->get_index()<<std::endl;
#endif
    work_block(block->get_instr_list());
    for (auto to : block->get_idom_child())
        work(to);
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "Roll back B" << block->get_index() << std::endl;
#endif
    roll_back(block->get_instr_list());
}
void GlobalCommonSubexpressionElimination::run()
{
    std::cout << "Running pass: Global Common Subexpression Elimination " << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_idfa->global_symbol_optimization_is_valid()) {
        m_idfa->compute_side_effect_info();
    }
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef) {
            CFGManager::build_dominator_tree(&unit);
            m_vn = 0;
            m_vn_to_sym.clear();
            m_vn_of_sym.clear();
            m_vn_of_expr.clear();
            m_vn_of_call.clear();
            for (auto& instr : unit.get_definations_const()) {
                if (!instr.a()->is_global())
                    new_vn(instr.a());
            }
            work(unit.get_entry());
        }
}
