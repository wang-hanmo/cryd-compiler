#include<cfg.h>
#include<linear_ir_manager.h>
#include<unordered_map>
#include<queue>
#include<set>
#include<cassert>
#include<sstream>
#include<algorithm>
int IRBlock::s_index = 0;
void IRBlock::adjust_edge_to_uncond()
{
    assert(m_edge[1] != nullptr && m_edge[0]==nullptr);
    m_edge[0] = m_edge[1];
    m_edge[1] = nullptr;
}
void IRBlock::set_edge(int index,IRBlock* to)
{
    m_edge[index]=to;
    if(to != nullptr)
        to->m_anti_edge.insert(this);
}
void IRBlock::delete_edge_oneway(int index)
{
    m_edge[index] = nullptr;
}
void IRBlock::delete_edge(int index)
{
    IRBlock* target = m_edge[index];
    if(target != nullptr)
        target->erase_from_anti_edge(this);
    m_edge[index] = nullptr;
}
void IRBlock::erase_from_anti_edge(IRBlock* target)
{
    /*
    auto iter = std::find(m_anti_edge.begin(), m_anti_edge.end(), target);
    if (iter != m_anti_edge.end())
        m_anti_edge.erase(iter);*/
    if (auto iter = m_anti_edge.find(target);iter != m_anti_edge.end()) {
        m_anti_edge.erase(iter);
    }
}
void IRBlock::add_instr(const IRInstr& instr)
{
    m_instr_list.push_back(instr);
}
void IRBlock::add_instr_to_front(const IRInstr& instr)
{
    m_instr_list.push_front(instr);
}
void IRBlock::delete_graph(IRBlock* target)
{
    for(int i=0;i<1;++i)
        if(target->m_edge[i]!=nullptr){
            delete_graph(target->m_edge[i]);
            target->m_edge[i]=nullptr;
        }
    delete target;
}
IRUnit IRUnit::create_func_def()
{
    //IRBlock::reset_index();
    auto entry = new IRBlock();
    auto exit = new IRBlock();
    return IRUnit(IRUnitType::FuncDef, entry, exit);
}
IRUnit IRUnit::create_var_def()
{
    return IRUnit(IRUnitType::VarDef);
}
void IRUnit::add_def_instr(const IRInstr& instr)
{
    m_definations.push_back(instr);
}


IRUnit* IRUnit::clone(IRUnit unit)
{
    //此clone函数只适用于ProcedureIntegration中的IRUnit复制
    //因之后会对所有变量重命名,故此处没有创建IRSymbol!!!
    std::vector<IRBlock*> old_ibfn;
    std::vector<IRBlock*> new_ibfn;

    int n = 0;   
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        now->set_tag(n);
        old_ibfn.push_back(now);
        IRBlock* new_block = new IRBlock();
        for(const auto& instr: now->get_instr_list())
            new_block->add_instr(instr);
        new_block->set_tag(n);
        new_ibfn.push_back(new_block);
        n++;
        for (int k = 0; k <= 1; ++k)
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
            {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
    }

    for(int i = 0; i < n ; i++)
    {
        IRBlock* old_blk = old_ibfn[i];
        IRBlock* new_blk = new_ibfn[i];
        for(int k = 0; k <= 1; k++)
        {
            IRBlock* old_succ = old_blk->get_succ(k);//old
            if(old_succ != nullptr)
            {
                IRBlock* new_succ = new_ibfn[old_succ->get_tag()];
                new_blk->set_edge(k, new_succ); 
            }
        }
    }

    IRUnit* new_unit = new IRUnit(unit.get_type());
    for(const auto& def: unit.get_definations())
        new_unit->add_def_instr(def);
    IRBlock* old_entry = unit.get_entry();
    IRBlock* new_entry = new_ibfn[old_entry->get_tag()];
    new_unit->set_entry(new_entry);
    IRBlock* old_exit = unit.get_exit();
    IRBlock* new_exit = new_ibfn[old_exit->get_tag()];
    new_unit->set_exit(new_exit);

    new_unit->set_tag(unit.get_tag());

    return new_unit;
}

int IRUnit::find_max_index_for_tlp_var()
{
    int max_index = -1;
    for (auto& instr : this->get_definations_const())
        for (auto sym : { instr.r() ,instr.a(),instr.b(),instr.c() })
            if (instr.a()->kind() == IRSymbolKind::Temp || instr.a()->kind() == IRSymbolKind::Local || instr.a()->kind() == IRSymbolKind::Param)
                max_index = std::max(max_index, instr.a()->index());
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(this->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list_const())
            for (auto sym : { instr.r() ,instr.a(),instr.b(),instr.c() })
                if (sym != nullptr)
                    if (sym->kind() == IRSymbolKind::Temp || sym->kind() == IRSymbolKind::Local || sym->kind() == IRSymbolKind::Param)
                        max_index = std::max(max_index,sym->index());
        for (int k = 0; k < now->out_degree(); ++k) {
            if (visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
    return max_index;
}