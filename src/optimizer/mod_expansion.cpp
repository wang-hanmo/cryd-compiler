#include <mod_expansion.h>
#include <ir_define.h>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <ir_symbol_table.h>
#include <cmath>
#include <iterator>
#include <algorithm>
void ModExpansion::ir_list_find_index(IRInstrList& program)
{
   for(auto&instr:program)
   {
        if(instr.r()!=nullptr){
            if(instr.r()->kind()==IRSymbolKind::Temp || instr.r()->kind() == IRSymbolKind::Local || instr.r()->kind() == IRSymbolKind::Param)
                m_max_temp_index = std::max(m_max_temp_index,instr.r()->index()+1);
        }
        if(instr.a()!=nullptr){
            if(instr.a()->kind() == IRSymbolKind::Temp || instr.a()->kind() == IRSymbolKind::Local || instr.a()->kind() == IRSymbolKind::Param)
                m_max_temp_index = std::max(m_max_temp_index,instr.a()->index()+1);
        }
        if(instr.b()!=nullptr){
            if(instr.b()->kind() == IRSymbolKind::Temp || instr.b()->kind() == IRSymbolKind::Local || instr.b()->kind() == IRSymbolKind::Param)
                m_max_temp_index = std::max(m_max_temp_index,instr.b()->index()+1);
        }
        if (instr.c() != nullptr) {
            if (instr.c()->kind() == IRSymbolKind::Temp || instr.c()->kind() == IRSymbolKind::Local || instr.c()->kind() == IRSymbolKind::Param)
                m_max_temp_index = std::max(m_max_temp_index, instr.c()->index() + 1);
        }
   }
}
//判断一个正数是否为2的整数幂
bool ModExpansion::is_power_of_2(int num)
{
    return (num & (num - 1)) ? false : true;
}
IRInstrList::iterator ModExpansion::mod_expansion(IRInstrList& instr_list, IRInstrList::iterator instr)
{
    IRInstrList insertion_list;
    auto res_oprand = instr->r();
    auto divisor = instr->b();
    auto dividend = instr->a();
    if (divisor->kind() == IRSymbolKind::Value && is_power_of_2(divisor->int_value())) {
        return instr;
    }
    //模运算展开
    IRSymbol* temp_oprand = nullptr;
    insertion_list.push_back(IRInstr::create_binary_calc(
        IROper::DivI,
        temp_oprand = m_ir_sym_table->create_temp(BasicType::Int,++m_max_temp_index),
        dividend,
        divisor
    ));
    IRSymbol* temp_oprand2 = nullptr;
    insertion_list.push_back(IRInstr::create_binary_calc(
        IROper::MulI,
        temp_oprand2 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
        divisor,
        temp_oprand
    ));
    insertion_list.push_back(IRInstr::create_binary_calc(
        IROper::SubI,
        res_oprand,
        dividend,
        temp_oprand2
    ));
    instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
    instr = instr_list.erase(instr);
    return --instr;
}
void ModExpansion::work_ir_list(IRInstrList& program)
{
    for (auto instr = program.begin(); instr != program.end(); ++instr) {
        if (instr->type() != IRType::BinaryCalc)
            continue;
        switch (instr->op()){
        case IROper::ModI: {
            //对模运算做优化
            instr = mod_expansion(program, instr);
            break;
        }
        default:
            break;
        }
    }
}
void ModExpansion::work_cfg(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while(!q.empty()){
        IRBlock* now=q.front();
        q.pop();
        work_ir_list(now->get_instr_list());
        for(int k=1;k>=0;--k){
            if(now->get_succ(k)!=nullptr&& visited.find(now->get_succ(k))==visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void ModExpansion::find_max_index(IRUnit* unit)
{
    m_max_temp_index = 0;
    ir_list_find_index(unit->get_definations());
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        ir_list_find_index(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void ModExpansion::run()
{
    std::cout << "Running pass: Mod Expansion " << std::endl;
    if (m_cfg == nullptr || m_ir_sym_table==nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    for (auto& unit : *m_cfg){
         if (unit.get_type() == IRUnitType::FuncDef){
             find_max_index(&unit);
             work_cfg(&unit);
         }
    }
}
