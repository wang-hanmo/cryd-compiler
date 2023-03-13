#include <forward_substitution.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <cfg_manager.h>
using namespace forward_substitution;
//删除关联语句
void ForwardSubstitution::delete_related_instr(IRSymbol* sym)
{
    if (sym->kind() == IRSymbolKind::Local || sym->kind() == IRSymbolKind::Param) {
        for (auto sym_ : m_related_sym[sym->get_tag()])
            m_sym_def_set.erase(SymDefPair(sym_));
        m_related_sym[sym->get_tag()].clear();
    } else if (sym->kind() == IRSymbolKind::Global) {
        for (auto sym_ : m_related_sym_glb[sym->get_tag()])
            m_sym_def_set_glb.erase(SymDefPair(sym_));
        m_related_sym_glb[sym->get_tag()].clear();
    }
}
bool ForwardSubstitution::is_not_considered_type(IRSymbol* sym)
{
    if (sym == nullptr)
        return false;
    if (sym->kind() == IRSymbolKind::Register || sym->kind() == IRSymbolKind::Memory)
        return true;
    if (sym->array_length() != IRArrayLength::IR_NONE_ARRAY)
        return true;
    return false;
}
void ForwardSubstitution::work_assign(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //检查本语句是否能被替代
    if (auto iter = m_sym_def_set.find(SymDefPair(instr->a())); iter != m_sym_def_set.end()) {
        instr->rebind_a(iter->def->a());
        instr->rebind_b(iter->def->b());
        instr->rebind_c(iter->def->c());
        instr->reset_op(iter->def->op());
        instr->reset_sop(iter->def->sop());
        instr->rewrite_type(iter->def->type());
        iter->def->set_no(0);
        m_sym_def_set.erase(iter);
    }else if (auto iter = m_sym_def_set_glb.find(SymDefPair(instr->a())); iter != m_sym_def_set_glb.end()) {
        instr->rebind_a(iter->def->a());
        instr->rebind_b(iter->def->b());
        instr->rebind_c(iter->def->c());
        instr->reset_op(iter->def->op());
        instr->reset_sop(iter->def->sop());
        instr->rewrite_type(iter->def->type());
        iter->def->set_no(0);
        m_sym_def_set_glb.erase(iter);
    }
    //检查本语句是否能够作为替代其他语句的候选
    if (is_non_pointer_temp_var(instr->r()) && m_use_count[instr->r()->get_tag()] == 1) {
        //插入对应赋值语句
        m_sym_def_set.insert(SymDefPair(instr));
        //在本指令涉及的操作数上维护关联语句
        if (instr->a()->kind() == IRSymbolKind::Local || instr->a()->kind() == IRSymbolKind::Param)
            m_related_sym[instr->a()->get_tag()].push_back(instr->r());
        else if(instr->a()->kind()==IRSymbolKind::Global)
            m_related_sym_glb[instr->a()->get_tag()].push_back(instr->r());
    }
    //删除关联语句
    delete_related_instr(instr->r());
}
void ForwardSubstitution::work_unary_calc(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //检查本语句是否能够作为替代其他语句的候选
    if (is_non_pointer_temp_var(instr->r()) && m_use_count[instr->r()->get_tag()] == 1) {
        //插入对应赋值语句
        m_sym_def_set.insert(SymDefPair(instr));
        //在本指令涉及的操作数上维护关联语句
        if (instr->a()->kind() == IRSymbolKind::Local || instr->a()->kind() == IRSymbolKind::Param)
            m_related_sym[instr->a()->get_tag()].push_back(instr->r());
        else if (instr->a()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->a()->get_tag()].push_back(instr->r());
    }
    //删除关联语句
    delete_related_instr(instr->r());
}
void ForwardSubstitution::work_binary_calc(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->b()) ||
        is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //有两个r左值的语句没法向前替代
    if (instr->op() == IROper::SignedLargeMulI)
        return;
    //检查本语句是否能够作为替代其他语句的候选
    if (is_non_pointer_temp_var(instr->r()) && m_use_count[instr->r()->get_tag()] == 1) {
        //插入对应赋值语句
        m_sym_def_set.insert(SymDefPair(instr));
        //在本指令涉及的操作数上维护关联语句
        if (instr->a()->kind() == IRSymbolKind::Local || instr->a()->kind() == IRSymbolKind::Param)
            m_related_sym[instr->a()->get_tag()].push_back(instr->r());
        else if (instr->a()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->a()->get_tag()].push_back(instr->r());
        if (instr->b()->kind() == IRSymbolKind::Local || instr->b()->kind() == IRSymbolKind::Param)
            m_related_sym[instr->b()->get_tag()].push_back(instr->r());
        else if (instr->b()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->b()->get_tag()].push_back(instr->r());
    }
    //删除关联语句
    delete_related_instr(instr->r());
}
void ForwardSubstitution::work_ternary_calc(IRInstr* instr)
{
    //暂不考虑全局变量和数组
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a()) ||
        is_not_considered_type(instr->b()) ||
        is_not_considered_type(instr->c())) {
        return;
    }
    //检查本语句是否能够作为替代其他语句的候选
    if (is_non_pointer_temp_var(instr->r()) && m_use_count[instr->r()->get_tag()] == 1) {
        //插入对应赋值语句
        m_sym_def_set.insert(SymDefPair(instr));
        //在本指令涉及的操作数上维护关联语句
        if (instr->a()!=nullptr && (instr->a()->kind() == IRSymbolKind::Local || instr->a()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->a()->get_tag()].push_back(instr->r());
        else if (instr->a() != nullptr && instr->a()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->a()->get_tag()].push_back(instr->r());
        if (instr->b() != nullptr && (instr->b()->kind() == IRSymbolKind::Local || instr->b()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->b()->get_tag()].push_back(instr->r());
        else if (instr->b()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->b()->get_tag()].push_back(instr->r());
        if (instr->c() != nullptr && (instr->c()->kind() == IRSymbolKind::Local || instr->c()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->c()->get_tag()].push_back(instr->r());
        else if (instr->c()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->c()->get_tag()].push_back(instr->r());
    }
    //删除关联语句
    delete_related_instr(instr->r());
}
void ForwardSubstitution::work_block_cond_goto(IRInstr* instr)
{
    //检查本语句是否能被替代
    auto iter = m_sym_def_set.find(SymDefPair(instr->a()));
    auto iter_glb = m_sym_def_set_glb.find(SymDefPair(instr->a()));
    bool local_valid = iter != m_sym_def_set.end();
    bool global_valid = iter_glb != m_sym_def_set_glb.end();
    if (!local_valid && !global_valid)
        return;
    IRInstr* def_instr = nullptr;
    if (local_valid) def_instr = iter->def;
    else  def_instr = iter_glb->def;
    bool rewrited = false;
    switch (def_instr->type()) {
    case IRType::Assign:
        instr->rebind_a(def_instr->a());
        rewrited = true;
        break;
    case IRType::BinaryCalc:
        if (is_relation_oper(def_instr->op())) {
            instr->rebind_a(def_instr->a());
            instr->rebind_b(def_instr->b());
            instr->reset_op(def_instr->op());
            instr->rewrite_type(IRType::BlockBinaryGoto);
            rewrited = true;
        }
        break;
    case IRType::UnaryCalc:
        if (def_instr->op() == IROper::NotI || def_instr->op() == IROper::NotF) {
            instr->rebind_a(def_instr->a());
            instr->reset_op(def_instr->op());
            instr->rewrite_type(IRType::BlockUnaryGoto);
            rewrited = true;
        }
    default:
        break;
    }
    if (rewrited) {
        def_instr->set_no(0);
        if (local_valid) m_sym_def_set.erase(iter);
        else m_sym_def_set_glb.erase(iter);
    }
}
void ForwardSubstitution::work_ir_list(IRInstrList& program)
{
    m_sym_def_set.clear();
    for (auto& instr:program) {
        instr.set_no(1);//1表示活代码，0表示死代码
        switch (instr.type()) {
        case IRType::Assign:
            work_assign(&instr);
            break;
        case IRType::UnaryCalc:
            work_unary_calc(&instr);
            break;
        case IRType::BinaryCalc:
            work_binary_calc(&instr);
            break;
        case IRType::TernaryCalc:
            work_ternary_calc(&instr);
            break;
        case IRType::BlockCondGoto:
            work_block_cond_goto(&instr);
            break;
        case IRType::Call:
            [[fallthrough]];
        case IRType::CallWithRet:
            m_sym_def_set_glb.clear();
            break;
        default:
            break;
        }
    }
    //删除被替代的t变量
    for (auto instr = program.begin(); instr != program.end();) {
        if (instr->no()==0)
            instr = program.erase(instr);
        else instr++;
    }
}
void ForwardSubstitution::work_cfg(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        work_ir_list(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void ForwardSubstitution::init()
{
    m_global_count = 0;
    m_related_sym_glb.clear();
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::VarDef) {
            for (auto& instr : unit.get_definations()) {
                if (instr.a()->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                    instr.a()->set_tag(m_global_count++);
                    m_related_sym_glb.push_back({});
                }
            }
        }
}
bool ForwardSubstitution::is_non_pointer_temp_var(IRSymbol* sym)
{
    return sym != nullptr && sym->kind() == IRSymbolKind::Temp &&sym->array_length() == IRArrayLength::IR_NONE_ARRAY;
}
//按照bfs序遍历块，定值一定在使用之前出现
void ForwardSubstitution::get_use_info_for_temp_vars(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list()) {
            for (auto sym : instr.lvalues()) {
                if (is_non_pointer_temp_var(sym)) {
                    sym->set_tag(m_temp_count++);
                    m_use_count.push_back(0);
                }
            }
            for (auto sym : instr.rvalues()){
                if (is_non_pointer_temp_var(sym)) {
                    assert(sym->get_tag() >= 0);
                    m_use_count[sym->get_tag()]++;
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
}
void ForwardSubstitution::run()
{
    std::cout << "Running pass: Forward Substitution " << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    init();
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef) {
            m_temp_count = 0;
            m_use_count.clear();
            m_local_param_count = 0;
            m_related_sym.clear();
            //对l、p变量编号
            for (auto& def_instr : unit.get_definations()) {
                if (def_instr.a()->kind() == IRSymbolKind::Local || def_instr.a()->kind() == IRSymbolKind::Param)
                    if (def_instr.a()->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                        def_instr.a()->set_tag(m_local_param_count);
                        m_related_sym.push_back({});
                        m_local_param_count++;
                    }
            }
            get_use_info_for_temp_vars(&unit);
            work_cfg(&unit);
        }
}
