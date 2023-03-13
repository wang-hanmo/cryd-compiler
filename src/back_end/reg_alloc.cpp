#include <reg_alloc.h>
#include <cfg_manager.h>
int RegisterAllocator::mem_idx = 0;
int RegisterAllocator::tmp_idx = -1;
int RegisterAllocator::ra_time;

RIG::RIG(int i_unit_no, std::vector<IRSymbol*> i_symbol): 
    m_unit_no(i_unit_no), m_symbol(i_symbol)
{
    m_var_count = m_symbol.size();
    m_interfere.resize(m_var_count, BitMap(m_var_count));
    m_move.resize(m_var_count, BitMap(m_var_count));
    m_exist_flag = BitMap(m_var_count);
    m_search.clear();
    m_move_related.clear();
    m_move_unrelated.clear();
    m_interfere_degree.resize(m_var_count, 0);
    m_move_degree.resize(m_var_count, 0);
    m_mov_instr.clear();
    m_coalesce.clear();
    m_spill_time.resize(m_var_count, 0);
    m_fparam_memory.clear();
    for(int i = 0; i < m_var_count; i++)
    {
        m_exist_flag.set(i);
        m_search.emplace(m_symbol[i], i);
        m_move_unrelated.emplace(i);
    }
}

void RIG::set_interfere(int i, int j)
{
    if(m_interfere[i].test(j))
        return;
    m_interfere[i].set(j);
    m_interfere[j].set(i);
    if(m_exist_flag.test(i))
        m_interfere_degree[i]++;
    if(m_exist_flag.test(j))
        m_interfere_degree[j]++;
}
void RIG::reset_interfere(int i, int j)
{
    if(!m_interfere[i].test(j))
        return;
    m_interfere[i].reset(j);
    m_interfere[j].reset(i);
    if(m_exist_flag.test(i))
        m_interfere_degree[i]--;
    if(m_exist_flag.test(j))
        m_interfere_degree[j]--;
}


bool RIG::test_interfere(int i, int j)
{
    if(!m_exist_flag.test(i) || !m_exist_flag.test(j))
        return false;
    return m_interfere[i].test(j);
}

void RIG::set_move(int i, int j)
{
    if(m_move[i].test(j))
        return;
    m_move[i].set(j);
    m_move[j].set(i);
    if(m_exist_flag.test(i))
        m_move_degree[i]++;
    if(m_exist_flag.test(j))
        m_move_degree[j]++;
}
void RIG::reset_move(int i, int j)
{
    if(!m_move[i].test(j))
        return;
    m_move[i].reset(j);
    m_move[j].reset(i);
    if(m_exist_flag.test(i))
        m_move_degree[i]--;
    if(m_exist_flag.test(j))
        m_move_degree[j]--;
}

bool RIG::test_move(int i, int j)
{
    if(!m_exist_flag.test(i) || !m_exist_flag.test(j))
        return false;
    return m_move[i].test(j);
}

void RIG::delete_node(int i)
{
    m_exist_flag.reset(i);
    for(int j = 0; j < m_var_count; j++)
    {
        if(i == j)
        {
            m_interfere_degree[j] = 0;
            m_move_degree[j] = 0;
        }
        else if(m_exist_flag.test(j))
        {        
            if(m_interfere[i].test(j))
                m_interfere_degree[j]--;
            if(m_move[i].test(j))
                m_move_degree[j]--;
        }

    }
}
void RIG::insert_node(int i)
{
    m_exist_flag.set(i);
    for(int j = 0; j < m_var_count; j++)
    {
        if(i == j)
            continue;
        if(m_exist_flag.test(j))
        {
            if(m_interfere[i].test(j))
            {
                m_interfere_degree[i]++;
                m_interfere_degree[j]++;
            }
            if(m_move[i].test(j))
            {
                m_move_degree[i]++;
                m_move_degree[j]++;
            }
        }
    }
}

int RIG::coalesce_idx(int symid)
{
    for(auto& coalesce_pair: m_coalesce)
    {
        if(symid == coalesce_pair.first)
            return coalesce_pair.second;
    }
    return -1;
    // if(m_coalesce.count(symid))
    //     return m_coalesce.at(symid);
    // else
    //     return -1;
}

int RIG::symbol_idx(IRSymbol* symbol)
{
    if(symbol == nullptr)
        return -1;
    else if(m_search.count(symbol))
        return m_search.at(symbol);
    else
        return -1;
}

void RIG::print_rig()
{

    std::cout << "----IRUnit " << m_unit_no << "----" << std::endl;
    int var_count = m_symbol.size();
    std::cout << "interfere graph:" << std::endl;
    for(int i = 0; i < var_count; i++)
    {
        std::cout << "\t" << m_symbol[i]->get_string();
    }
    std::cout << std::endl;
    for(int i = 0; i < var_count; i++)
    {
        std::cout << m_symbol[i]->get_string() << "\t";
        for(int j = 0; j < var_count; j++)
        {
            std::cout << (int)m_interfere[i].test(j) << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "move graph:" << std::endl;
    for(int i = 0; i < var_count; i++)
    {
        std::cout << "\t" << m_symbol[i]->get_string();
    }
    std::cout << std::endl;
    for(int i = 0; i < var_count; i++)
    {
        std::cout << m_symbol[i]->get_string() << "\t";
        for(int j = 0; j < var_count; j++)
        {
            std::cout << (int)m_move[i].test(j) << "\t";
        }
        std::cout << std::endl;
    }
    // for(int i = 0; i < var_count; i++)
    //     std::cout << m_symbol[i]->get_string() << ":" << m_interfere_degree[i] << std::endl;
}

void RAT::print_rat(std::ostream &os)
{
    os << "----IRUnit " << m_unit_no << "----" << std::endl;
    int no = m_ir_symbol.empty() ? 0 : m_sym_live_intervals[0].size();
    // os << "sym_live_intervals:" << std::endl;
    // int sym_count = m_ir_symbol.size();
    // for(int i = 0; i < sym_count; i++)
    // {
    //     os << m_ir_symbol[i]->get_string() << ":";
    //     int prev = 0, curr = 0;
    //     for(int j = 0; j < no; j++)
    //     {
    //         curr = m_sym_live_intervals[i].test(j);
    //         if(prev == 0 && curr != 0)
    //             os << "[" << j+m_start_no << ",";
    //         else if(prev != 0 && curr == 0)
    //             os << j-1+m_start_no << "] ";
    //         prev = curr;
    //     }
    //     if(prev != 0)
    //         os << no-1+m_start_no << "] ";

    //     os << std::endl;
    // }
    os << "mode_r: " << (m_mode_r ? "linear" : "coloring") << " mode_s: " << (m_mode_s ? "linear" : "coloring") << std::endl;
    os << "reg_live_intervals:" << std::endl;
    for(int i = 0; i < RegCount + RegCount_S; i++)
    {
        if(i < RegCount)
        {
            if(REGISTER_USAGE[i] == special)
                continue;
            os << "r" << i << ":";
        }
        else
        {
            if(REGISTER_USAGE_S[i - RegCount] == special)
                continue;
            os << "s" << (i - RegCount) << ":";
        }
        int prev = 0, curr = 0;
        for(int j = 0; j < no; j++)
        {
            curr = m_reg_live_intervals[i].test(j);
            if(prev == 0 && curr != 0)
                os << "[" << j+m_start_no << ",";
            else if(prev != 0 && curr == 0)
                os << j-1+m_start_no << "] ";
            prev = curr;
        }
        if(prev != 0)
            os << no-1+m_start_no << "] ";

        os << std::endl;
    }
    os << "register allocation:" << std::endl;
    os << "IRSymbol\tRegister" << std::endl;
    int var_count = m_ir_symbol.size();
    for(int i = 0; i < var_count; i++)
    {
        os << m_ir_symbol[i]->get_string() << "  \t";
        if(m_spill[i])
            os << "Spill" << std::endl;
        else if((int)m_reg_id[i] < RegCount)
            os << "r" << (int)m_reg_id[i] << std::endl;
        else
            os << "s" << ((int)m_reg_id[i] - RegCount) << std::endl;

    }   
}

int RAT::at_map(IRSymbol* symbol)
{
    if(!m_search_map.count(symbol)) 
        return -1;
    else
        return m_search_map.at(symbol);
}

bool RAT::is_spill(IRSymbol* symbol)
{
    if(symbol == nullptr)
        RegisterAllocator::register_error("IRSymbol nullptr!", 0);
    int idx = at_map(symbol);
    if(idx == -1)
        return true;
    else
        return m_spill[idx];
    // std::cout << "[RAT error] no matching ir_symbol " << symbol->get_string() << std::endl;
}

RegID RAT::reg_alloc(IRSymbol* symbol)
{
    if(symbol == nullptr)
        RegisterAllocator::register_error("IRSymbol nullptr!", 1);
    int idx = at_map(symbol);
    if(idx == -1)
        RegisterAllocator::register_error("[RAT error] no matching ir_symbol", 3);
    else
    {
        if(m_reg_id[idx] >= RegCount + RegCount_S)
            RegisterAllocator::register_error("wrong regid", 2);
        return m_reg_id[idx];
    }
    // std::cout << "[RAT error] no matching ir_symbol " << symbol->get_string() << std::endl;
    return RegID::r0;
}

bool RAT::test_sym_live(IRSymbol* symbol,int no)
{
    if(symbol == nullptr)
        RegisterAllocator::register_error("IRSymbol nullptr!", 4);
    int idx = at_map(symbol);
    if(idx == -1)
        return false;
    else
        return m_sym_live_intervals[idx].test(no);
    // std::cout << "[RAT error] no matching ir_symbol " << symbol->get_string() << std::endl;
}

bool RAT::test_reg_live(RegID reg,int no)
{
    if((int)reg >= m_reg_live_intervals.size())
        RegisterAllocator::register_error("wrong regid", 5);
    return m_reg_live_intervals[(int)reg].test(no);
}

bool RAT::test_reg_used(RegID reg)
{
    if((int)reg >= m_reg_live_intervals.size())
        RegisterAllocator::register_error("wrong regid", 6);
    return !m_reg_live_intervals[(int)reg].empty();
}

RIG* RegisterAllocator::build(LiveIntervals* li, IRUnit unit)
{
    RIG* i_rig = new RIG(li->get_unit_no(), li->get_symbol());
    const int var_count = li->size();
    const int start_no = li->get_start_no();
    const int end_no = li->get_end_no();
    CFGManager::loop_depth_analysis_for_blocks(&unit);
    IRInstrList tmp_mov_instr;
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop(); 
        if (!now->is_exit()&&!now->is_entry())
        {
            int spillcost = spill_cost(now->get_tag());
            for(auto& instr: now->get_instr_list())
            {
                
                int symid = i_rig->symbol_idx(instr.r());
                if(symid != -1 && instr.type() == IRType::Load && instr.r()->kind() == IRSymbolKind::Param && instr.a()->kind() == IRSymbolKind::Memory)
                    i_rig->add_fparam_memory(instr.r(), instr.a());
                int instr_no_0 = -1;
                int instr_no_1 = -1;
                if(now->get_succ(0) != nullptr && !now->get_succ(0)->is_exit())
                    instr_no_0 = now->get_succ(0)->get_instr_list().front().no();
                if(now->get_succ(1) != nullptr && !now->get_succ(1)->is_exit())
                    instr_no_1 = now->get_succ(1)->get_instr_list().front().no();
                if(i_rig->symbol_idx(instr.a()) != -1)
                    i_rig->increase_spill_time(i_rig->symbol_idx(instr.a()), spillcost);
                if(i_rig->symbol_idx(instr.b()) != -1)
                    i_rig->increase_spill_time(i_rig->symbol_idx(instr.b()), spillcost);
                if(i_rig->symbol_idx(instr.c()) != -1)
                    i_rig->increase_spill_time(i_rig->symbol_idx(instr.c()), spillcost);
                if(i_rig->symbol_idx(instr.r()) != -1)
                    i_rig->increase_spill_time(i_rig->symbol_idx(instr.r()), spillcost);
                if(instr.type() == IRType::Assign)
                {
                    //mov指令
                    int symid_r = i_rig->symbol_idx(instr.r());
                    int symid_a = i_rig->symbol_idx(instr.a());
                    if(symid_r != -1)
                    {
                        if(instr.no() == now->get_instr_list().back().no())
                        {
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_r && i != symid_a && li->test_live_intervals(i, instr.no()-start_no) &&\
                                ((instr_no_0 != -1 && li->test_live_intervals(i, instr_no_0-start_no)) || (instr_no_1 != -1 && li->test_live_intervals(i, instr_no_1-start_no))))
                                    i_rig->set_interfere(symid_r, i);
                            }
                        }
                        else
                        {
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_r && i != symid_a && li->test_live_intervals(i, instr.no()-start_no) && li->test_live_intervals(i, instr.no()-start_no+1))
                                    i_rig->set_interfere(symid_r, i);
                            }
                        }
                    }
                    if(symid_r != -1 && symid_a != -1)
                        tmp_mov_instr.push_back(instr);
                }
                else if( (instr.type() == IRType::BinaryCalc && instr.op() != IROper::ModI) ||\
                         instr.type() == IRType::ArrayLoad || instr.type() == IRType::TernaryCalc ||\
                         instr.type() == IRType::Load || instr.type() == IRType::UnaryCalc)
                {
                    //除了模运算的双目运算指令, Arrayload指令, 三目运算指令, load指令, 单目运算指令
                    int symid_r = i_rig->symbol_idx(instr.r());
                    if(symid_r != -1)
                    {
                        if(instr.no() == now->get_instr_list().back().no())
                        {
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_r && li->test_live_intervals(i, instr.no()-start_no) &&\
                                ((instr_no_0 != -1 && li->test_live_intervals(i, instr_no_0-start_no)) || (instr_no_1 != -1 && li->test_live_intervals(i, instr_no_1-start_no))))
                                    i_rig->set_interfere(symid_r, i);
                            }
                        }
                        else
                        {
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_r && li->test_live_intervals(i, instr.no()-start_no) && li->test_live_intervals(i, instr.no()-start_no+1))
                                    i_rig->set_interfere(symid_r, i);
                            }
                        }
                    }
                    //SignedLargeMulI特俗处理, instr.c()为定值
                    int symid_c = i_rig->symbol_idx(instr.c());
                    if(symid_c != -1 && instr.type() == IRType::TernaryCalc && instr.op() == IROper::SignedLargeMulI)
                    {
                        if(instr.no() == now->get_instr_list().back().no())
                        {
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_c && li->test_live_intervals(i, instr.no()-start_no) &&\
                                ((instr_no_0 != -1 && li->test_live_intervals(i, instr_no_0-start_no)) || (instr_no_1 != -1 && li->test_live_intervals(i, instr_no_1-start_no))))
                                    i_rig->set_interfere(symid_c, i);
                            }
                        }
                        else
                        {
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_c && li->test_live_intervals(i, instr.no()-start_no) && li->test_live_intervals(i, instr.no()-start_no+1))
                                    i_rig->set_interfere(symid_c, i);
                            }
                        }
                    }
                }
                else if(instr.type() == IRType::Call || instr.type() == IRType::CallWithRet)
                {
                    //call指令
                    for(int j = 0; j < ARGUMENT_REGISTER_COUNT + ARGUMENT_REGISTER_COUNT_S + 2; j++)
                    {
                        assert(m_argreg[j]->kind() == IRSymbolKind::Register);
                        IRSymbol* symbol = m_argreg[j];
                        if((!m_type && symbol->index() < RegCount) || (m_type && symbol->index() >= RegCount))
                        {
                            int symid_r = i_rig->symbol_idx(symbol);
                            for(int i = 0; i < m_var_count; i++)
                            {
                                if(i != symid_r && li->test_live_intervals(i, instr.no()-start_no))
                                    i_rig->set_interfere(symid_r, i);
                            }
                        }
                    }
                }
                else
                {
                    //其他指令
                    if(instr.r() == nullptr || instr.type() == IRType::ArrayStore || instr.type() == IRType::Store)
                        continue;
                    int symid_r = i_rig->symbol_idx(instr.r());
                    if(symid_r != -1)
                    {
                        for(int i = 0; i < m_var_count; i++)
                        {
                            if(i != symid_r && li->test_live_intervals(i, instr.no()-start_no))
                                i_rig->set_interfere(symid_r, i);
                        }
                    }
                }
            }
        }
        for (int k = 0; k <= 1; ++k)
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
            {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
    }
    for(auto& instr: tmp_mov_instr)
    {
        if(instr.r()->kind() == IRSymbolKind::Register || instr.a()->kind() == IRSymbolKind::Register)
            continue;
        int no = instr.no() - li->get_start_no();
        int i = i_rig->symbol_idx(instr.r());
        int j = i_rig->symbol_idx(instr.a());
        if(i != -1 && j != -1)
        {
            if(!i_rig->test_interfere(i, j) && !i_rig->test_move(i, j))
            {
                i_rig->set_move(i, j);
                ///*DEBUG*/std::cout << "move_edge: " << instr.no() << " " << i_rig->get_symbol(i)->get_string() << " --- " << i_rig->get_symbol(j)->get_string() << std::endl;
                if(i_rig->is_move_unrelated(i))
                    i_rig->delete_move_unrelated(i);
                if(i_rig->is_move_unrelated(j))
                    i_rig->delete_move_unrelated(j);
                if(!i_rig->is_move_related(i))
                    i_rig->insert_move_related(i);
                if(!i_rig->is_move_related(j))
                    i_rig->insert_move_related(j);
                i_rig->add_mov_instr(instr);
            }
        }
    }
    // std::cout << "move unrelated: ";
    // for(auto symid: i_rig->get_move_unrelated())
    //     std::cout << i_rig->get_symbol(symid)->get_string() << ",";
    // std::cout << std::endl;
    // std::cout << "move related: ";
    // for(auto symid: i_rig->get_move_related())
    //     std::cout << i_rig->get_symbol(symid)->get_string() << ",";
    // std::cout << std::endl;

    return i_rig;
}

void RegisterAllocator::print_rig_list(std::list<RIG*> rig_list)
{
    for(auto& rig : rig_list)
    {
        rig->print_rig();
    }
}

void RegisterAllocator::simplify(RIG* rig)
{
    if(rig == nullptr)
        RegisterAllocator::register_error("Interfere Graph nullptr!", 7);
    auto iter = rig->get_move_unrelated().begin();
    while(iter != rig->get_move_unrelated().end())
    {
        int symid = *iter;
        iter++;
        if(!rig->test_node(symid))
            continue;//该结点不在图中
        if(rig->get_symbol(symid)->kind() == IRSymbolKind::Register)
            continue;
        int k = rig->get_interfere_degree(symid);
        if(k < (m_type ? ALLOCATABLE_REGISTER_COUNT_S : ALLOCATABLE_REGISTER_COUNT))
        {
            //简化
            ///*DEBUG*/std::cout << "simplify " << rig->get_symbol(symid)->get_string() << std::endl;
            rig->delete_node(symid);
            rig->push_select_stack(symid);
        }
    }
}

bool RegisterAllocator::coalesce(RIG* rig, LiveIntervals* li)
{
    auto iter = rig->get_mov_instr().begin();
    bool done = false;
    while(iter != rig->get_mov_instr().end())
    {
        auto& instr = *iter;
        if (rig->symbol_idx(instr.r()) == -1 || rig->symbol_idx(instr.a()) == -1)
        {
            iter++;
            continue;
        }
        int i = rig->symbol_idx(instr.r());
        int j = rig->symbol_idx(instr.a());
        if (!rig->is_move_related(i) || !rig->is_move_related(i))
        {
            iter++;
            continue;
        }
        if (!rig->test_node(i) || !rig->test_node(j))
        {
            iter++;
            continue;
        }
        // if (rig->get_move_degree(i) > 1 || rig->get_move_degree(j) > 1)
        // {
        //     iter++;
        //     continue;
        // }
        bool coalesced = true;
        //George合并策略
        for(int t = 0; t < rig->size(); t++)
        {
            if(rig->test_interfere(i, t))
            {
                //i的邻居结点
                if(!rig->test_interfere(j, t) && !(rig->get_interfere_degree(t) < (m_type ? ALLOCATABLE_REGISTER_COUNT_S : ALLOCATABLE_REGISTER_COUNT)))
                {
                    //不可以合并
                    coalesced = false;
                    break;
                }
            }
        }
        if (!coalesced)
        {
            iter++;
            continue;
        }

        //合并处理
        done = true;
        if(rig->get_symbol(i)->kind() == IRSymbolKind::Register)
        {
            int tmp = i;
            i = j;
            j = tmp;
        }
        ///*DEBUG*/std::cout << "coalesce " << rig->get_symbol(i)->get_string() << " " << rig->get_symbol(j)->get_string() << std::endl;
        rig->add_coalesce(i, j);
        for(int k = 0; k < rig->size(); k++)
        {
            if(rig->test_interfere_abs(i, k))
            {
                rig->set_interfere(j, k);
                rig->reset_interfere(i, k);
            }
            if(rig->test_move_abs(i, k))
            {
                rig->set_move(j, k);
                rig->reset_move(i, k);
            }
        }
        rig->delete_node(i);
        rig->reset_interfere(i, j);
        rig->reset_move(i, j);
        iter = rig->get_mov_instr().erase(iter);
        rig->delete_move_related(i);
        for(int k = 0; k < rig->size(); k++)
        {
            if(rig->test_interfere_abs(j, k) && rig->test_move_abs(j, k))
            {
                rig->reset_move(j, k);
                if(rig->get_move_degree(k) == 0)
                {
                    rig->delete_move_related(k);
                    rig->insert_move_unrelated(k);
                }
            }
        }
        if(rig->get_move_degree(j) == 0)
        {
            rig->delete_move_related(j);
            rig->insert_move_unrelated(j);
        }
        for(int no = 0; no < li->get_live_intervals(i).size(); no++)
        {
            if(li->get_live_intervals(i).test(no))
                li->get_live_intervals(j).set(no);
        }
    }
    return done;
}

bool RegisterAllocator::freeze(RIG* rig)
{
    int min_degree = rig->size();
    int symid = -1;
    if(rig->get_move_related().empty())
        return false;
    for(int idx: rig->get_move_related())
    {
        if(rig->get_interfere_degree(idx) < min_degree)
        {
            min_degree = rig->get_interfere_degree(idx);
            symid = idx;
        }
    }
    assert(symid != -1);
    ///*DEBUG*/std::cout << "freeze " << rig->get_symbol(symid)->get_string() << std::endl;
    for(int i = 0; i < rig->size(); i++)
    {
        if(rig->test_move(symid, i))
        {
            rig->reset_move(symid, i);
            if(rig->get_move_degree(i) == 0)
            {
                rig->delete_move_related(i);
                rig->insert_move_unrelated(i);
            }
        }
    }
    rig->delete_move_related(symid);
    rig->insert_move_unrelated(symid);
    return true;
}

bool RegisterAllocator::spill(RIG* rig)
{
    if(rig == nullptr)
        RegisterAllocator::register_error("RIG nullptr!", 8);
    if(rig->is_rig_empty())
        return false;
    int symid = -1;
    int max_value = -65536;
    for(int i = 0; i < rig->size(); i++)
    {
        if(!rig->test_node(i))
            continue;
        if(rig->get_symbol(i)->kind() == IRSymbolKind::Register)
            continue;
        int value = 2 * rig->get_interfere_degree(i) - rig->get_spill_time(i);
        // int value = rig->get_interfere_degree(i);
        // int value = - rig->get_spill_time(i);
        if(rig->get_symbol(i)->kind() == IRSymbolKind::Temp && rig->get_symbol(i)->index() < 0)
            value -= m_var_count;
        if(value > max_value)
        {
            max_value = value;
            symid = i;
        }
    }
    if(symid == -1)
        return false;
    ///*DEBUG*/std::cout << "potential spill " << rig->get_symbol(symid)->get_string() << std::endl;
    rig->delete_node(symid);
    rig->push_spill_stack(symid);
    return true;
}

RAT* RegisterAllocator::select(RIG* rig, LiveIntervals* li)
{
    if(rig == nullptr)
        RegisterAllocator::register_error("RIG nullptr!", 9);
    RAT* rat = new RAT();
    rat->set_mode(m_type, m_mode);
    rat->set_unit_no(rig->get_unit_no());
    rat->set_start_no(li->get_start_no());
    rat->set_end_no(li->get_end_no());

    rat->set_ir_symbol(li->get_symbol());
    rat->set_sym_live_intervals(li->get_live_intervals());
    rat->init_reg_id(m_type, m_var_count);
    rat->init_spill(m_var_count);
    rat->init_reg_live_intervals(m_type ? RegCount_S : RegCount, li->get_end_no()-li->get_start_no()+1);

    rat->init_map();
    rat->init_spill_symbol();
    rat->init_spill_memory();
    for(int i = 0; i < m_var_count; i++)
        rat->emplace_map(rat->get_ir_symbol(i), i);

    for(auto& sym: li->get_symbol())
    {
        if(sym->kind() == IRSymbolKind::Register)
        {
            int symid = sym->get_tag();
            rig->insert_node(symid);
            int regid = sym->index();
            rat->set_reg_id(symid, (RegID)regid);
            rat->set_spill(symid, false);
            ///*DEBUG*/std::cout << "preliminary select " << rig->get_symbol(symid)->get_string() << std::endl;
            regid = m_type ? (regid - RegCount) : regid;
            for(int i = 0; i < li->get_live_intervals(symid).size(); i++)
            {
                if(rat->get_sym_live_intervals(symid).test(i))
                    rat->get_reg_live_intervals(regid).set(i);
            }
        }
    }

    while(!rig->empty_select_stack())
    {
        int symid = rig->top_select_stack();
        rig->pop_select_stack();
        // std::cout << rig->get_symbol(symid)->get_string() << std::endl;
        if(rig->test_node(symid))
        {
            // std::cout << "already allocate " << rig->get_symbol(symid)->get_string() << std::endl;
            continue;//已经预设好寄存器
        }
        rig->insert_node(symid);
        int regid = 0;
        for(regid = 0; regid < (m_type ? ALLOCATABLE_REGISTER_COUNT_S : ALLOCATABLE_REGISTER_COUNT); regid++)
        {
            bool allocatable = true;
            // 检查寄存器冲突
            for(int i = 0; i < m_var_count; i++)
            {
                if(rig->test_interfere(symid, i) && rat->get_reg_id(i) == (m_type ? ALLOCATABLE_REGISTERS_S[regid] : ALLOCATABLE_REGISTERS[regid]))
                    allocatable = false;
            }
            if(allocatable)
            {
                ///*DEBUG*/std::cout << "select " << rig->get_symbol(symid)->get_string() << std::endl;
                // std::cout << "allocate r" << regid << " for " << rig->get_symbol(symid)->get_string() << std::endl;
                rat->set_reg_id(symid, (m_type ? ALLOCATABLE_REGISTERS_S[regid] : ALLOCATABLE_REGISTERS[regid]));
                rat->set_spill(symid, false);
                int regid_ = m_type ? (ALLOCATABLE_REGISTERS_S[regid] - RegCount) : ALLOCATABLE_REGISTERS[regid];
                for(int i = 0; i < li->get_live_intervals(symid).size(); i++)
                {
                    if(rat->get_sym_live_intervals(symid).test(i))
                        rat->get_reg_live_intervals(regid_).set(i);
                }
                break;
            }
        }
        if(regid == (m_type ? ALLOCATABLE_REGISTER_COUNT_S : ALLOCATABLE_REGISTER_COUNT))
        {
            ///*DEBUG*/std::cout << "actual spill* " << rig->get_symbol(symid)->get_string() << std::endl;
            // std::cout << "spill " << rig->get_symbol(symid)->get_string() << std::endl;
            rat->set_spill(symid, true);
            rat->add_spill_symbol(rat->get_ir_symbol(symid));
            rig->delete_node(symid);
        }
    }
    while(!rig->empty_spill_stack())
    {
        bool spill = true;
        int symid = rig->top_spill_stack();
        rig->pop_spill_stack();
        // std::cout << rig->get_symbol(symid)->get_string() << std::endl;
        if(rig->test_node(symid))
        {
            // std::cout << "already allocate " << rig->get_symbol(symid)->get_string() << std::endl;
            continue;//形参已经预设好寄存器
        }
        rig->insert_node(symid);
        for(int regid = 0; regid < (m_type ? ALLOCATABLE_REGISTER_COUNT_S : ALLOCATABLE_REGISTER_COUNT); regid++)
        {
            bool allocatable = true;
            // 检查寄存器冲突
            for(int i = 0; i < m_var_count; i++)
            {
                if(rig->test_interfere(symid, i) && rat->get_reg_id(i) == (m_type ? ALLOCATABLE_REGISTERS_S[regid] : ALLOCATABLE_REGISTERS[regid]))
                    allocatable = false;
            }
            if(allocatable)
            {
                ///*DEBUG*/std::cout << "select " << rig->get_symbol(symid)->get_string() << std::endl;
                // std::cout << "allocate r" << regid << " for " << rig->get_symbol(symid)->get_string() << std::endl;
                rat->set_reg_id(symid, (m_type ? ALLOCATABLE_REGISTERS_S[regid] : ALLOCATABLE_REGISTERS[regid]));
                rat->set_spill(symid, false);
                int regid_ = m_type ? (ALLOCATABLE_REGISTERS_S[regid] - RegCount) : ALLOCATABLE_REGISTERS[regid];
                for(int i = 0; i < li->get_live_intervals(symid).size(); i++)
                {
                    if(rat->get_sym_live_intervals(symid).test(i))
                        rat->get_reg_live_intervals(regid_).set(i);
                }
                spill = false;
                break;
            }
        }
        if(spill)
        {
            ///*DEBUG*/std::cout << "actual spill " << rig->get_symbol(symid)->get_string() << std::endl;
            // std::cout << "spill " << rig->get_symbol(symid)->get_string() << std::endl;
            rat->set_spill(symid, true);
            rat->add_spill_symbol(rat->get_ir_symbol(symid));
            rig->delete_node(symid);
        }
    }
    //设置溢出的内存
    for(auto& symbol : rat->get_spill_symbol())
    {
        IRSymbol* memory = nullptr;
        if(rig->fparam_memory(symbol) != nullptr)
            memory = rig->fparam_memory(symbol);
        else
            memory = m_ir_sym_table->create_memory(mem_idx++);
        rat->add_spill_memory(symbol, memory);
    }
    auto iter = rig->get_coalesce().rbegin();
    while(iter != rig->get_coalesce().rend())
    {
        auto& coalesce_pair = *iter;
        rat->set_reg_id(coalesce_pair.first, rat->get_reg_id(coalesce_pair.second));
        rat->set_spill(coalesce_pair.first, rat->get_spill(coalesce_pair.second));
        int symid = coalesce_pair.first;
        int regid = m_type ? (rat->get_reg_id(symid) - RegCount) : rat->get_reg_id(symid);
        if(!rat->get_spill(symid))
        {
            for(int i = 0; i < li->get_live_intervals(symid).size(); i++)
            {
                if(rat->get_sym_live_intervals(symid).test(i))
                    rat->get_reg_live_intervals(regid).set(i);
            }
        }
        else
        {
            assert(rat->spill_memory(rat->get_ir_symbol(coalesce_pair.second)) != nullptr);
            IRSymbol* memory = rat->spill_memory(rat->get_ir_symbol(coalesce_pair.second));
            IRSymbol* symbol = rat->get_ir_symbol(symid);
            rat->add_spill_memory(symbol, memory);
        }
        iter++;
    }
    return rat;
}

void RegisterAllocator::rewrite(RIG* rig, RAT* rat, IRUnit unit)
{
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        if (!now->is_exit()&&!now->is_entry())
        {
            
            for(auto iter = now->get_instr_list().begin();iter != now->get_instr_list().end();iter++)
            {
                auto& instr = *iter;
                //溢出处理
                IRSymbol* mem = nullptr;
                mem = rat->spill_memory(instr.a());
                if(instr.a() != nullptr && rig->symbol_idx(instr.a()) != -1 && mem != nullptr)
                {
                    // std::cout << "rewrite a spill " << instr.no() << std::endl;
                    IRSymbol* sym = m_ir_sym_table->create_temp(instr.a()->basic_type(), tmp_idx--, instr.a()->array_length());
                    iter = now->get_instr_list().insert(iter, IRInstr::create_load(sym, mem));
                    iter++;
                    instr.rebind_a(sym);
                }
                mem = rat->spill_memory(instr.b());
                if(instr.b() != nullptr && rig->symbol_idx(instr.b()) != -1 && mem != nullptr)
                {
                    // std::cout << "rewrite b spill " << instr.no() << std::endl;
                    IRSymbol* sym = m_ir_sym_table->create_temp(instr.b()->basic_type(), tmp_idx--, instr.b()->array_length());
                    iter = now->get_instr_list().insert(iter, IRInstr::create_load(sym, mem));
                    iter++;
                    instr.rebind_b(sym);
                }
                mem = rat->spill_memory(instr.c());
                if(instr.c() != nullptr && rig->symbol_idx(instr.c()) != -1 && mem != nullptr)
                {
                    // std::cout << "rewrite c spill " << instr.no() << std::endl;
                    IRSymbol* sym = m_ir_sym_table->create_temp(instr.c()->basic_type(), tmp_idx--, instr.c()->array_length());
                    if(instr.type() == IRType::TernaryCalc && instr.op() == IROper::SignedLargeMulI)
                    {
                        iter = now->get_instr_list().insert(++iter, IRInstr::create_store(mem, sym));
                        iter--;
                    }
                    else
                    {
                        iter = now->get_instr_list().insert(iter, IRInstr::create_load(sym, mem));
                        iter++;
                    }
                    instr.rebind_c(sym);
                }
                mem = rat->spill_memory(instr.r());
                if(instr.r() != nullptr && rig->symbol_idx(instr.r()) != -1 && mem != nullptr)
                {
                    // std::cout << "rewrite r spill " << instr.no() << std::endl;
                    IRSymbol* sym = m_ir_sym_table->create_temp(instr.r()->basic_type(), tmp_idx--, instr.r()->array_length());
                    if(instr.type() == IRType::ArrayStore || instr.type() == IRType::Store)//use
                    {
                        iter = now->get_instr_list().insert(iter, IRInstr::create_load(sym, mem));
                        iter++;
                    }
                    else//def
                    {
                        if(instr.type() == IRType::Load && instr.a() == mem)
                            iter = now->get_instr_list().erase(iter);
                        else
                            iter = now->get_instr_list().insert(++iter, IRInstr::create_store(mem, sym));
                        iter--;
                    }
                    instr.rebind_r(sym);
                }
            }
        }
        for (int k = 0; k <= 1; ++k)
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
            {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
    }
}

void RegisterAllocator::swap(LiveIntervals* li, int i, int j)
{
	IRSymbol* tmp_symbol = li->get_symbol(i);
    Interval tmp_Interval = li->get_live_intervals(i);
    int tmp_first_no = li->get_first_no(i);
    int tmp_last_no = li->get_last_no(i);
    
    li->set_symbol(i, li->get_symbol(j));
    li->set_live_intervals(i, li->get_live_intervals(j));
    li->set_first_no(i, li->get_first_no(j));
    li->set_last_no(i, li->get_last_no(j));

    li->set_symbol(j, tmp_symbol);
    li->set_live_intervals(j, tmp_Interval);
    li->set_first_no(j, tmp_first_no);
    li->set_last_no(j, tmp_last_no);
}
 
void RegisterAllocator::heapify(LiveIntervals* li, int n, int i)
{
	if (i > n)
	{
		return;
	}
	int c1 = 2 * i + 1;
	int c2 = 2 * i + 2;
	int max = i;
    
	if (c1 < n && li->get_first_no(c1) >= li->get_first_no(max))
	{
		max = c1;
	}
	if (c2 < n && li->get_first_no(c2) >= li->get_first_no(max))
	{
		max = c2;
	}
	if (max != i)
	{
		swap(li, max, i);
		heapify(li, n, max);
	}
}
 
void RegisterAllocator::build_heap(LiveIntervals* li)
{
	int n = li->size();
	int last_node = n - 1;
	int parent = (last_node - 1) / 2;
	int i;
	for (i = parent; i >= 0; i--)
	{
		heapify(li, n, i);
	}
}
 
void RegisterAllocator::heap_sort(LiveIntervals* li)
{
	int n = li->size();
	build_heap(li);
	int i;
	for (i = n - 1; i >= 0; i--)
	{
		swap(li, i, 0);
		heapify(li, i, 0);
	}
}

RAT* RegisterAllocator::scan(LiveIntervals* li)
{
    if(li == nullptr)
        RegisterAllocator::register_error("Live Intervals nullptr!", 11);
    RAT* rat = new RAT();
    rat->set_mode(m_type, m_mode);
    rat->set_unit_no(li->get_unit_no());
    rat->set_start_no(li->get_start_no());
    rat->set_end_no(li->get_end_no());
    
    rat->set_ir_symbol(li->get_symbol());
    rat->set_sym_live_intervals(li->get_live_intervals());
    rat->init_reg_id(m_type, m_var_count);
    rat->init_spill(m_var_count);
    rat->init_reg_live_intervals(m_type ? RegCount_S : RegCount, li->get_end_no()-li->get_start_no()+1);

    rat->init_map();
    rat->init_spill_symbol();
    rat->init_spill_memory();
    for(int i = 0; i < m_var_count; i++)
        rat->emplace_map(rat->get_ir_symbol(i), i);

    std::vector<int> active;
    for(int symid = 0; symid < m_var_count; symid++)
    {
        IRSymbol* sym = li->get_symbol(symid);
        if(sym->kind() == IRSymbolKind::Register)
        {
            int regid = sym->index();
            rat->set_reg_id(symid, (RegID)regid);
            rat->set_spill(symid, false);
            regid = m_type ? (regid - RegCount) : regid;
            for(int i = 0; i < li->get_live_intervals(symid).size(); i++)
            {
                if(rat->get_sym_live_intervals(symid).test(i))
                    rat->get_reg_live_intervals(regid).set(i);
            }
        }
        
        bool instead = false;
        for(int i = 0; i < active.size(); i++)
        {
            int i_symid = active[i];
            if(li->get_last_no(i_symid) < li->get_first_no(symid))
            {
                active[i] = symid;
                instead = true;
                // symid取代i_symid, 将i_symid的regid分配给symid
                RegID regid = rat->get_reg_id(i_symid);
                // std::cout << "get " << symid << " " << (int)regid << " " << i_symid << std::endl;
                rat->set_reg_id(symid, regid);
                rat->set_spill(symid, false);
                int regid_ = m_type ? (regid - RegCount) : regid;
                for(int j = 0; j < li->get_live_intervals(symid).size(); j++)
                {
                    if(rat->get_sym_live_intervals(symid).test(j))
                        rat->get_reg_live_intervals(regid_).set(j);
                }
                break;
            }
        }
        if(!instead)
        {
            if(active.size() < (m_type ? ALLOCATABLE_REGISTER_COUNT_S : ALLOCATABLE_REGISTER_COUNT) \
                - (m_type ? ARGUMENT_REGISTER_COUNT_S : ARGUMENT_REGISTER_COUNT))
            {
                int regid = active.size() + (m_type ? ARGUMENT_REGISTER_COUNT_S : ARGUMENT_REGISTER_COUNT);
                active.push_back(symid);
                rat->set_reg_id(symid, (m_type ? ALLOCATABLE_REGISTERS_S[regid] : ALLOCATABLE_REGISTERS[regid]));
                rat->set_spill(symid, false);
                // std::cout << "set " << symid << " " << (int)ALLOCATABLE_REGISTERS[regid] << std::endl;
                int regid_ = m_type ? (ALLOCATABLE_REGISTERS_S[regid] - RegCount) : ALLOCATABLE_REGISTERS[regid];
                for(int j = 0; j < li->get_live_intervals(symid).size(); j++)
                {
                    if(rat->get_sym_live_intervals(symid).test(j))
                        rat->get_reg_live_intervals(regid_).set(j);
                }
            }
            else
            {
                //spill
                rat->set_spill(symid, true);
            }
        }
    }

    return rat;
}

std::list<RAT*> RegisterAllocator::allocate_register()
{
    m_allocation.clear();
    auto iter1 = m_cfg->begin();
    auto iter2 = m_li.begin();
    if((*iter1).get_type()!=IRUnitType::FuncDef)
        iter1++;
    while(iter1 != m_cfg->end() && iter2 != m_li.end())
    {
        auto& unit = *iter1;
        auto& li = *iter2;
        LiveIntervals *r_li, *s_li;
        RAT *rat, *r_rat, *s_rat;
        //分解li
        spilt_li(li, &r_li, &s_li);

        //整型寄存器分配
        m_type = 0;
        m_var_count = r_li->size();
        m_mode = (m_var_count > 15000);   //超过n个变量时，使用简单的线性扫描
        switch(m_mode)
        {
            case 0:r_rat = coloring_allocate(r_li, unit);break;
            case 1:r_rat = linear_allocate(r_li);break;
            default:std::cout<<"[RegisterAllocation Error] Wrong mode"<<std::endl;break;
        }

        //浮点型寄存器分配
        m_type = 1;
        m_var_count = s_li->size();
        m_mode = (m_var_count > 15000);   //超过n个变量时，使用简单的线性扫描
        switch(m_mode)
        {
            case 0:s_rat = coloring_allocate(s_li, unit);break;
            case 1:s_rat = linear_allocate(s_li);break;
            default:std::cout<<"[RegisterAllocation Error] Wrong mode"<<std::endl;break;
        }
        delete r_li;
        delete s_li;
        //合并rat
        rat = merge_rat(r_rat, s_rat);
        delete r_rat;
        delete s_rat;
        m_allocation.push_back(rat);
        iter1++;
        iter2++;
    }
    ra_time++;
    return m_allocation;
}

RAT* RegisterAllocator::coloring_allocate(LiveIntervals* li, IRUnit unit)
{
    RIG* rig = build(li, unit);
    // rig->print_rig();
    bool coalesce_done = true;
    bool freeze_done = true;
    bool spill_done = true;
    while(spill_done)
    {
        while(freeze_done)
        {
            while(coalesce_done)
            {
                simplify(rig);
                coalesce_done = coalesce(rig, li);
            }
            freeze_done = freeze(rig);
            coalesce_done = true;
        }
        spill_done = spill(rig);
        coalesce_done = true;
        freeze_done = true;
    }
    RAT* rat = select(rig, li);
    int spill_num = rat->get_spill_memory().size();
    // std::cout << "round " << ra_time << " spill: " << spill_num << "/" << m_var_count << std::endl;
    if(!((ra_time >= 4) || (spill_num == 0)))
    {
        rewrite(rig, rat, unit);
        m_done = false;
    }
    delete rig;
    return rat;
}

RAT* RegisterAllocator::linear_allocate(LiveIntervals* li)
{
    heap_sort(li);
    // li->print_li();
    RAT* rat = scan(li);
    return rat;

}

void RegisterAllocator::print_allocation(std::list<RAT*> allocation, std::ostream &os)
{
    for(auto& rat : allocation)
    {
        rat->print_rat(os);
    }
}

void RegisterAllocator::spilt_li(LiveIntervals* li, LiveIntervals** r_li, LiveIntervals** s_li)
{
    *r_li = new LiveIntervals();//int型符号，对应r0~r15
    *s_li = new LiveIntervals();//float型符号，对应s0~s31
    const int var_count = li->size();
    int var_count_r = 0, var_count_s = 0;
    for(int i = 0; i < var_count; i++)
    {
        IRSymbol* sym = li->get_symbol(i);
        if((sym->basic_type() == BasicType::Float && sym->array_length() == IRArrayLength::IR_NONE_ARRAY)\
             || (sym->kind() == IRSymbolKind::Register && sym->index() >= RegCount))
        {
            //浮点寄存器分配
            sym->set_tag(var_count_s++);
            (*s_li)->add_symbol(sym);
            (*s_li)->add_live_intervals(li->get_live_intervals(i));
            (*s_li)->add_first_no(li->get_first_no(i));
            (*s_li)->add_last_no(li->get_last_no(i));
        }
        else
        {
            //整型寄存器分配
            sym->set_tag(var_count_r++);
            (*r_li)->add_symbol(sym);
            (*r_li)->add_live_intervals(li->get_live_intervals(i));
            (*r_li)->add_first_no(li->get_first_no(i));
            (*r_li)->add_last_no(li->get_last_no(i));
        }
    }
    
    (*s_li)->set_unit_no(li->get_unit_no());
    (*s_li)->set_start_no(li->get_start_no());
    (*s_li)->set_end_no(li->get_end_no());
    (*r_li)->set_unit_no(li->get_unit_no());
    (*r_li)->set_start_no(li->get_start_no());
    (*r_li)->set_end_no(li->get_end_no());
    
}
RAT* RegisterAllocator::merge_rat(RAT* r_rat, RAT* s_rat)
{
    RAT* rat = new RAT();
    rat->set_mode(0, r_rat->get_mode(0));
    rat->set_mode(1, s_rat->get_mode(1));
    rat->set_unit_no(r_rat->get_unit_no());
    rat->set_start_no(r_rat->get_start_no());
    rat->set_end_no(r_rat->get_end_no());
    
    const int var_count_r = r_rat->size();
    const int var_count_s = s_rat->size();
    if(var_count_r > var_count_s)
    {
        rat->set_ir_symbol(r_rat->get_ir_symbol());
        rat->set_reg_id(r_rat->get_reg_id());
        rat->set_spill(r_rat->get_spill());
        rat->set_sym_live_intervals(r_rat->get_sym_live_intervals());
        rat->set_map(r_rat->get_map());
        rat->set_spill_symbol(r_rat->get_spill_symbol());
        rat->set_spill_memory(r_rat->get_spill_memory());
        for(int i = 0; i < var_count_r; i++)
            rat->get_ir_symbol(i)->set_tag(i);
        for(int i = 0; i < var_count_s; i++)
        {
            s_rat->get_ir_symbol(i)->set_tag(i + var_count_r);
            rat->add_ir_symbol(s_rat->get_ir_symbol(i));
            rat->add_reg_id(s_rat->get_reg_id(i));
            rat->add_spill(s_rat->get_spill(i));
            rat->add_sym_live_intervals(s_rat->get_sym_live_intervals(i));
        }
        for(auto& m: s_rat->get_map())
            rat->emplace_map(m.first, m.second + var_count_r);
        for(auto& m: s_rat->get_spill_symbol())
            rat->add_spill_symbol(m);
        for(auto& m: s_rat->get_spill_memory())
            rat->add_spill_memory(m.first, m.second);
    }
    else
    {
        rat->set_ir_symbol(s_rat->get_ir_symbol());
        rat->set_reg_id(s_rat->get_reg_id());
        rat->set_spill(s_rat->get_spill());
        rat->set_sym_live_intervals(s_rat->get_sym_live_intervals());
        rat->set_map(s_rat->get_map());
        rat->set_spill_symbol(s_rat->get_spill_symbol());
        rat->set_spill_memory(s_rat->get_spill_memory());
        for(int i = 0; i < var_count_s; i++)
            rat->get_ir_symbol(i)->set_tag(i);
        for(int i = 0; i < var_count_r; i++)
        {
            r_rat->get_ir_symbol(i)->set_tag(i + var_count_s);
            rat->add_ir_symbol(r_rat->get_ir_symbol(i));
            rat->add_reg_id(r_rat->get_reg_id(i));
            rat->add_spill(r_rat->get_spill(i));
            rat->add_sym_live_intervals(r_rat->get_sym_live_intervals(i));
        }
        for(auto& m: r_rat->get_map())
            rat->emplace_map(m.first, m.second + var_count_s);
        for(auto& m: r_rat->get_spill_symbol())
            rat->add_spill_symbol(m);
        for(auto& m: r_rat->get_spill_memory())
            rat->add_spill_memory(m.first, m.second);
    }
    
    rat->set_reg_live_intervals(r_rat->get_reg_live_intervals());
    for(int i = 0; i < RegCount_S; i++)
        rat->add_reg_live_intervals(s_rat->get_reg_live_intervals(i));
    if(rat->get_reg_live_intervals().size() != RegCount + RegCount_S)
        register_error("reg live intervals wrong size", 12);
    
    assert(rat->size() == var_count_r + var_count_s);

    return rat;
}

void RegisterAllocator::register_error(std::string msg, int error_code)
{
    std::cerr<<"[Register Error] " << msg << std::endl;
    exit(ErrorCode::REGISTER_ERROR + error_code);
}

int RegisterAllocator::spill_cost(int t)
{
    assert(t >= 0);
    int cost = 0;
    if(t <= 5)
        cost = 1 << (2 * t);
    else
        cost = 1024 + 1000 * (t - 5);
    return cost;    
}