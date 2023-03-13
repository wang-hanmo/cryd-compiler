#include <live.h>

void LiveIntervals::print_li(std::ostream &os)
{
    os << "----IRUnit " << m_unit_no << "----" << std::endl;
    os << "live intervals:" << std::endl;
    int var_count = m_symbol.size();
    for(int i = 0; i < var_count; i++)
    {
        os << m_symbol[i]->get_string() << ":";
        int no = m_live_intervals[i].size();
        int prev = 0, curr = 0;
        for(int j = 0; j < no; j++)
        {
            curr = m_live_intervals[i].test(j);
            if(prev == 0 && curr != 0)
                os << "[" << j+m_start_no << ",";
            else if(prev != 0 && curr == 0)
                os << j-1+m_start_no << "] ";
            prev = curr;
        }
        if(prev != 0)
            os << no-1+m_start_no << "] ";

        os << " (" << m_first_no[i]+m_start_no << "," << m_last_no[i]+m_start_no << ")";

        os << std::endl;
    }

}

void LiveManager::compute_block(IRBlock* block)
{
    if(block == nullptr)
        live_error("IRBlock nullptr!", 0);
    std::queue<IRBlock*> bfn;
    bfn.push(block);
    while(!bfn.empty())
    {
        IRBlock* i_bfn = bfn.front();
        bfn.pop();
        if(!blk_visited.count(i_bfn))
        {
            blk_visited.emplace(i_bfn);
            i_bfn->set_tag(blk_idx++);
            m_ir_block.push_back(i_bfn);
            for(auto& instr : i_bfn->get_instr_list())
            {
                instr.set_no(no++);
                if(instr.a() != nullptr && check_symbol_in_instr(instr, instr.a()) && !sym_visited.count(instr.a()))
                {
                    sym_visited.emplace(instr.a());
                    instr.a()->set_tag(sym_idx++);//设置tag附加信息，用于索引IRSymbol在m_ir_symbol中的位置
                    m_ir_symbol.push_back(instr.a());
                }
                if(instr.b() != nullptr && check_symbol_in_instr(instr, instr.b()) && !sym_visited.count(instr.b()))
                {
                    sym_visited.emplace(instr.b());
                    instr.b()->set_tag(sym_idx++);//设置tag附加信息，用于索引IRSymbol在m_ir_symbol中的位置
                    m_ir_symbol.push_back(instr.b());
                }
                if(instr.c() != nullptr && check_symbol_in_instr(instr, instr.c()) && !sym_visited.count(instr.c()))
                {
                    sym_visited.emplace(instr.c());
                    instr.c()->set_tag(sym_idx++);//设置tag附加信息，用于索引IRSymbol在m_ir_symbol中的位置
                    m_ir_symbol.push_back(instr.c());
                }
                if(instr.r() != nullptr && check_symbol_in_instr(instr, instr.r()) && !sym_visited.count(instr.r()))
                {
                    sym_visited.emplace(instr.r());
                    instr.r()->set_tag(sym_idx++);//设置tag附加信息，用于索引IRSymbol在m_ir_symbol中的位置
                    m_ir_symbol.push_back(instr.r());
                }
            }
            for(int i = 0; i < i_bfn->out_degree(); i++)
            {
                IRBlock* child = i_bfn->get_succ(i);
                bfn.push(child);
            }
        }
    }
    
}

void LiveManager::initial(IRUnit unit)
{
    blk_idx = 0;
    sym_idx = 0;
    blk_visited.clear();
    sym_visited.clear();
    m_ir_block.clear();
    m_ir_symbol.clear();
    m_live_def.clear();
    m_live_use.clear();
    m_live_in.clear();
    m_live_out.clear();
    m_live_intervals.clear();
    first_no.clear();
    last_no.clear();
    //载入基本块
    start_no = no;
    compute_block(unit.get_entry());
    end_no = no - 1;
    
    // for (auto & instr : unit.get_definations())
    // {
    //     if(instr.type() == IRType::FParam || instr.type() == IRType::LocalDecl)
    //     {
    //         sym_visited.emplace(instr.a());
    //         instr.a()->set_tag(sym_idx++);//设置tag附加信息，用于索引IRSymbol在m_ir_symbol中的位置
    //         m_ir_symbol.push_back(instr.a());
    //     }
    // }
    for(int i = 0; i < ARGUMENT_REGISTER_COUNT + ARGUMENT_REGISTER_COUNT_S + 2; i++)
    {
        IRSymbol* sym = m_argreg[i];
        sym_visited.emplace(sym);
        sym->set_tag(sym_idx++);//设置tag附加信息，用于索引IRSymbol在m_ir_symbol中的位置
        m_ir_symbol.push_back(sym);
    }
    //初始化bitmap
    const int block_count = m_ir_block.size();
    const int var_count = m_ir_symbol.size();
    m_live_def.resize(block_count, BitMap(var_count));
    m_live_use.resize(block_count, BitMap(var_count));
    m_live_in.resize(block_count, BitMap(var_count));
    m_live_out.resize(block_count, BitMap(var_count));
    m_live_intervals.resize(var_count, Interval(end_no - start_no + 1));
    first_no.resize(var_count, -1);
    last_no.resize(var_count, -1);
    // if(var_count > 100000)
    //     live_error("many symbols", 1);
}

void LiveManager::compute_live_def_and_use()
{
    const int block_count = m_ir_block.size();
    //遍历每个IRBlock
    int idx = -1;
    for(int i = 0; i < block_count; i++)
    {
        //遍历每条IR指令
        for(auto& instr : m_ir_block[i]->get_instr_list_const())
        {
            if(instr.type() == IRType::Call || instr.type() == IRType::CallWithRet)
            {
                //Call指令
                IRSymbol* func = instr.a();
                assert(func != nullptr);
                for(int j = 0; j < ARGUMENT_REGISTER_COUNT + ARGUMENT_REGISTER_COUNT_S + 2; j++)
                {
                    IRSymbol* sym = m_argreg[j];
                    assert(sym != nullptr);
                    idx = sym->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == sym)
                    {
                        m_live_def[i].set(idx);
                        // m_live_use[i].set(idx);
                    }
                }
            }
            else
            {
                //普通IR指令
                if(instr.a() != nullptr)
                {
                    idx = instr.a()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.a())
                    {
                        //若操作数不在live_def中，则加入live_use
                        if(m_live_def[i].test(idx) == 0)
                            m_live_use[i].set(idx);
                    }
                }
                if(instr.b() != nullptr)
                {
                    idx = instr.b()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.b())
                    {
                        //若操作数不在live_def中，则加入live_use
                        if(m_live_def[i].test(idx) == 0)
                            m_live_use[i].set(idx);
                    }
                }
                if(instr.c() != nullptr)
                {
                    idx = instr.c()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.c())
                    {
                        if(instr.type() == IRType::TernaryCalc && instr.op() == IROper::SignedLargeMulI)
                        {
                            //加入live_def
                            m_live_def[i].set(idx);
                        }
                        else
                        {
                        //若操作数不在live_def中，则加入live_use
                        if(m_live_def[i].test(idx) == 0)
                            m_live_use[i].set(idx);
                        }
                    }
                }
                if(instr.r() != nullptr)
                {
                    idx = instr.r()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.r())
                    {
                        if(instr.type() == IRType::ArrayStore || instr.type() == IRType::Store)
                        {
                            //数组视为use
                            if(m_live_def[i].test(idx) == 0)
                                m_live_use[i].set(idx);
                        }
                        else
                        {
                            //加入live_def
                            m_live_def[i].set(idx);
                        }
                    }
                }
            }
            
        }
    }
}

void LiveManager::compute_live_in_and_out()
{
    const int block_count = m_ir_block.size();
    while(1)
    {
        bool modified = false;
        //逆序遍历每个IRBlock
        for(int i = block_count - 1; i >= 0; i--)
        {
            //live_out是所有后继者的并集
            BitMap tmp = m_live_out[i];
            for(int j = 0; j < m_ir_block[i]->out_degree(); j++)
            {
                IRBlock* child = m_ir_block[i]->get_succ(j);
                int idx = child->get_tag();
                if(idx == -1)
                {
                    std::cout << "block error" << std::endl;
                    live_error("block error", 1);
                }
                else
                {
                    m_live_out[i] |= m_live_in[idx];
                }
            }
            tmp ^= m_live_out[i];
            if(!tmp.empty())
                modified = true;
            //live_in为live_out和live_def的差集与live_use取并集
            m_live_in[i] = (m_live_out[i] - m_live_def[i]) | m_live_use[i];
        }
        //直到live_out不变
        if(!modified)
            break;
    }
}

void LiveManager::compute_live_intervals()
{
    const int block_count = m_ir_block.size();
    const int var_count = m_ir_symbol.size();
    //逆序遍历每个IRBlock
    for(int i = block_count - 1; i >= 0; i--)
    {
        if(m_ir_block[i]->get_index() == 0 || m_ir_block[i]->get_index() == 1)
            continue;
        IRInstrList list = m_ir_block[i]->get_instr_list();
        int start_index;
        int end_index;
        if(list.empty())
        {
            start_index = start_no;
            end_index = start_no;
            continue;
        }
        else
        {
            start_index = list.front().no();
            end_index = list.back().no();
        }
        //延长原有Range或添加新Range(block开始，block结束)
        for(int idx = 0; idx < var_count; idx++)
        {
            if(m_live_out[i].test(idx))
            {
                m_live_intervals[idx].set(start_index - start_no);
                if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                    first_no[idx] = start_index - start_no;
                if(last_no[idx] == -1 || last_no[idx] < (end_index - start_no))
                    last_no[idx] = end_index - start_no;
            }
        }
        int no = end_index;
        int idx = -1;
        IRInstrList::reverse_iterator iter;
        for(iter = list.rbegin(); iter != list.rend(); iter++)
        {
            IRInstr instr = *iter;
            if(instr.type() == IRType::Call || instr.type() == IRType::CallWithRet)
            {
                //Call指令
                IRSymbol* func = instr.a();
                assert(func != nullptr);
                //def return
                if(func->get_ret_reg() != nullptr)
                {
                    IRSymbol* sym = func->get_ret_reg();
                    idx = sym->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == sym)
                    {
                        //将被定值的变量的range在此处截断
                        if(m_live_intervals[idx].test(start_index - start_no))
                        {
                            m_live_intervals[idx].reset(start_index - start_no);
                            m_live_intervals[idx].set(no - start_no);
                        }
                        if(first_no[idx] == (start_index - start_no))
                            first_no[idx] = no - start_no;
                    }
                }
                //use r0~r3,r12,s0~s15
                for(int j = 0; j < ARGUMENT_REGISTER_COUNT + ARGUMENT_REGISTER_COUNT_S + 2; j++)
                {
                    IRSymbol* sym = m_argreg[j];
                    assert(sym != nullptr);
                    idx = sym->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == sym)
                    {
                        //延长原有Range或添加新Range(block开始，指令位置)
                        if(m_live_intervals[idx].test(no - start_no))
                        {
                            //同时def
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                m_live_intervals[idx].reset(no - start_no);
                            }
                        }
                        else
                        {
                            //常规use
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                if(no < end_index)
                                {
                                    if(m_live_intervals[idx].test(no - start_no + 1))
                                        m_live_intervals[idx].reset(no - start_no + 1);
                                    else
                                        m_live_intervals[idx].set(no - start_no + 1);
                                }
                            }
                        }
                        if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                            first_no[idx] = start_index - start_no;
                        if(last_no[idx] == -1 || last_no[idx] < (no - start_no))
                            last_no[idx] = no - start_no;
                    }
                }
                //def r0~r3,r12,s0~s15
                for(int j = 0; j < ARGUMENT_REGISTER_COUNT + ARGUMENT_REGISTER_COUNT_S + 2; j++)
                {
                    IRSymbol* sym = m_argreg[j];
                    assert(sym != nullptr);
                    idx = sym->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == sym)
                    {
                        //将被定值的变量的range在此处截断
                        if(m_live_intervals[idx].test(start_index - start_no))
                        {
                            m_live_intervals[idx].reset(start_index - start_no);
                            m_live_intervals[idx].set(no - start_no);
                        }
                        if(first_no[idx] == (start_index - start_no))
                            first_no[idx] = no - start_no;
                    }
                }
                //use param
                for(auto& sym: func->get_rparam_reg())
                {
                    assert(sym != nullptr);
                    idx = sym->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == sym)
                    {
                        //延长原有Range或添加新Range(block开始，指令位置)
                        
                        if(m_live_intervals[idx].test(no - start_no))
                        {
                            //同时def
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                m_live_intervals[idx].reset(no - start_no);
                            }
                        }
                        else
                        {
                            //常规use
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                if(no < end_index)
                                {
                                    if(m_live_intervals[idx].test(no - start_no + 1))
                                        m_live_intervals[idx].reset(no - start_no + 1);
                                    else
                                        m_live_intervals[idx].set(no - start_no + 1);
                                }
                            }
                        }
                        if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                            first_no[idx] = start_index - start_no;
                        if(last_no[idx] == -1 || last_no[idx] < (no - start_no))
                            last_no[idx] = no - start_no;
                    }
                }
            }
            else
            {
                //普通IR指令
                if(instr.r() != nullptr)
                {
                    idx = instr.r()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.r())
                    {
                        if(instr.type() == IRType::ArrayStore || instr.type() == IRType::Store)
                        {
                            //数组的定值和使用对于活跃区间作用等效
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                if(no < end_index)
                                {
                                    if(m_live_intervals[idx].test(no - start_no + 1))
                                        m_live_intervals[idx].reset(no - start_no + 1);
                                    else
                                        m_live_intervals[idx].set(no - start_no + 1);
                                }
                            }
                            if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                                first_no[idx] = start_index - start_no;
                            if(last_no[idx] == -1 || last_no[idx] < (no - start_no))
                                last_no[idx] = no - start_no;
                        }
                        else
                        {
                            //将被定值的变量的range在此处截断
                            if(m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].reset(start_index - start_no);
                                m_live_intervals[idx].set(no - start_no);
                            }
                            if(first_no[idx] == (start_index - start_no))
                                first_no[idx] = no - start_no;
                        }
                    }
                } 
                if(instr.c() != nullptr)
                {
                    idx = instr.c()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.c())
                    {
                        if(instr.type() == IRType::TernaryCalc && instr.op() == IROper::SignedLargeMulI)
                        {
                            //将被定值的变量的range在此处截断
                            if(m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].reset(start_index - start_no);
                                m_live_intervals[idx].set(no - start_no);
                            }
                            if(first_no[idx] == (start_index - start_no))
                                first_no[idx] = no - start_no;
                        }
                        else
                        {
                            //延长原有Range或添加新Range(block开始，指令位置)
                            if(m_live_intervals[idx].test(no - start_no))
                            {
                                //同时def
                                if(!m_live_intervals[idx].test(start_index - start_no))
                                {
                                    m_live_intervals[idx].set(start_index - start_no);
                                    m_live_intervals[idx].reset(no - start_no);
                                }
                            }
                            else
                            {
                                //常规use
                                if(!m_live_intervals[idx].test(start_index - start_no))
                                {
                                    m_live_intervals[idx].set(start_index - start_no);
                                    if(no < end_index)
                                    {
                                        if(m_live_intervals[idx].test(no - start_no + 1))
                                            m_live_intervals[idx].reset(no - start_no + 1);
                                        else
                                            m_live_intervals[idx].set(no - start_no + 1);
                                    }
                                }
                            }
                            if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                                first_no[idx] = start_index - start_no;
                            if(last_no[idx] == -1 || last_no[idx] < (no - start_no))
                                last_no[idx] = no - start_no;
                        }
                    }
                }
                if(instr.a() != nullptr)
                {
                    idx = instr.a()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.a())
                    {
                        //延长原有Range或添加新Range(block开始，指令位置)
                        if(m_live_intervals[idx].test(no - start_no))
                        {
                            //同时def
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                m_live_intervals[idx].reset(no - start_no);
                            }
                        }
                        else
                        {
                            //常规use
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                if(no < end_index)
                                {
                                    if(m_live_intervals[idx].test(no - start_no + 1))
                                        m_live_intervals[idx].reset(no - start_no + 1);
                                    else
                                        m_live_intervals[idx].set(no - start_no + 1);
                                }
                            }
                        }
                        if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                            first_no[idx] = start_index - start_no;
                        if(last_no[idx] == -1 || last_no[idx] < (no - start_no))
                            last_no[idx] = no - start_no;
                    }
                }
                if(instr.b() != nullptr)
                {
                    idx = instr.b()->get_tag();
                    if(idx >= 0 && idx < m_ir_symbol.size() && m_ir_symbol[idx] == instr.b())
                    {
                        //延长原有Range或添加新Range(block开始，指令位置)
                        if(m_live_intervals[idx].test(no - start_no))
                        {
                            //同时def
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                m_live_intervals[idx].reset(no - start_no);
                            }
                        }
                        else
                        {
                            //常规use
                            if(!m_live_intervals[idx].test(start_index - start_no))
                            {
                                m_live_intervals[idx].set(start_index - start_no);
                                if(no < end_index)
                                {
                                    if(m_live_intervals[idx].test(no - start_no + 1))
                                        m_live_intervals[idx].reset(no - start_no + 1);
                                    else
                                        m_live_intervals[idx].set(no - start_no + 1);
                                }
                            }
                        }
                        if(first_no[idx] == -1 || first_no[idx] > (start_index - start_no))
                            first_no[idx] = start_index - start_no;
                        if(last_no[idx] == -1 || last_no[idx] < (no - start_no))
                            last_no[idx] = no - start_no;
                    }
                }
            }
            
            no--;
        }
        //求异或和
        for(int idx = 0; idx < var_count; idx++)
        {
            // std::cout << m_ir_symbol[idx]->get_string() << ":" << m_live_intervals[idx].get_string()<< std::endl;
            m_live_intervals[idx].xor_sum(start_index - start_no, end_index - start_no);
            // std::cout << m_ir_symbol[idx]->get_string() << ":" << m_live_intervals[idx].get_string()<< std::endl;
        }
    }
    // for(int idx = 0; idx < var_count; idx++)
    // {
    //     if(m_live_intervals[idx].empty())
    //     {
    //         first_no[idx] = -1;
    //         last_no[idx] = -1;
    //     }
    //     std::cout << m_ir_symbol[idx]->get_string() << m_live_intervals[idx].get_string() << " " << first_no[idx] << "-" << last_no[idx] << std::endl;
    // }
}

void LiveManager::print_liveness()
{
    const int block_count = m_ir_block.size();
    const int var_count = m_ir_symbol.size();
    for(int i = 0; i < block_count; i++)
    {
        std::cout << "----block " << i << "----" << std::endl;
        LinearIRManager::print_ir_list(m_ir_block[i]->get_instr_list_const());
        std::cout << "live_def: ";
        for(int j = 0; j < var_count; j++)
            if(m_live_def[i].test(j))
                std::cout << m_ir_symbol[j]->get_string() << " ";
        std::cout << std::endl;
        std::cout << "live_use: ";
        for(int j = 0; j < var_count; j++)
            if(m_live_use[i].test(j))
                std::cout << m_ir_symbol[j]->get_string() << " ";
        std::cout << std::endl;
        std::cout << "live_in: ";
        for(int j = 0; j < var_count; j++)
            if(m_live_in[i].test(j))
                std::cout << m_ir_symbol[j]->get_string() << " ";
        std::cout << std::endl;
        std::cout << "live_out: ";
        for(int j = 0; j < var_count; j++)
            if(m_live_out[i].test(j))
                std::cout << m_ir_symbol[j]->get_string() << " ";
        std::cout << std::endl;
    }
    std::cout << "----live intervals----" << std::endl;
    for(int i = 0; i < var_count; i++)
    {
        std::cout << m_ir_symbol[i]->get_string() << " : ";
        int no = m_live_intervals[i].size();
        int prev = 0, curr = 0;
        for(int j = 0; j < no; j++)
        {
            curr = m_live_intervals[i].test(j);
            if(prev == 0 && curr != 0)
                std::cout << "[" << j+start_no << ",";
            else if(prev != 0 && curr == 0)
                std::cout << j-1+start_no << "] ";
            prev = curr;
        }
        if(prev != 0)
            std::cout << no-1+start_no << "] ";

        std::cout << std::endl;
    }
    std::cout << "-----------" << std::endl;
}

LiveIntervals* LiveManager::build()
{
    static int unit_no = 0;
    LiveIntervals* i_li = new LiveIntervals();
    i_li->set_unit_no(unit_no++);
    i_li->set_start_no(start_no);
    i_li->set_end_no(end_no);
    const int var_count = m_ir_symbol.size();
    i_li->set_symbol(m_ir_symbol);
    i_li->set_live_intervals(m_live_intervals);
    i_li->set_first_no(first_no);
    i_li->set_last_no(last_no);
    
    return i_li;
}

void LiveManager::print_li_list(std::list<LiveIntervals*> li_list, std::ostream &os)
{
    int i = 0;
    for(auto& li : li_list)
    {
        li->print_li(os);
    }
}

std::list<LiveIntervals*>& LiveManager::compute_liveness()
{
    no = 0;
    m_li.clear();
    for (auto& unit : *m_cfg)
    {
        if (unit.get_type()==IRUnitType::FuncDef) 
        {
            // 初始化静态成员
            initial(unit);
            // 计算LiveDef和LiveUse
            compute_live_def_and_use();
            // 计算LiveIn和LiveOut
            compute_live_in_and_out();
            // 计算Live Interval
            compute_live_intervals();

            // print_liveness();
            
            // 构造LiveIntervals
            LiveIntervals* i_li = build();
            
            m_li.push_back(i_li);
        }
    }
    
    return m_li;
}

bool LiveManager::check_symbol_in_instr(IRInstr instr, IRSymbol* symbol)
{
    if(symbol->kind() == IRSymbolKind::Temp)
        return true;
    if(symbol->kind() == IRSymbolKind::Param)
        return true;
    if(symbol->kind() == IRSymbolKind::Local && symbol->array_length() <= 0)
        return true;
    // if(symbol->kind() == IRSymbolKind::Memory)
    //     return true;
    // else if(symbol->kind() == IRSymbolKind::Value)
    // {
    //     if(instr.type() == IRType::BinaryCalc && \
    //     (instr.op() == IROper::Mul || instr.op() == IROper::Div || instr.op() == IROper::Mod))
    //         return true;
    //     else if(instr.type() == IRType::ArrayStore)
    //         return true;
    //     // else if(instr.type() == IRType::RParam)
    //     //     return true;
    //     else if (instr.type() == IRType::BlockCondGoto)
    //         return true;
    //     else if(!Operand2::checkImm8m(symbol->value().int_value))
    //         return true;
    //     else if(!ArmInstr::checkImm12(symbol->value().int_value))
    //         return true;
    // }
    return false;
}

bool LiveManager::check_move_edge_conflict(IRInstr instr)
{
    // if(instr.type() == IRType::BinaryCalc && instr.op() == IROper::Mod)
    //     return true;
    // if(instr.type() == IRType::ArrayLoad || instr.type() == IRType::ArrayStore)
    //     return true;
    // return false;
    return true;
}


void LiveManager::live_error(std::string msg, int error_code)
{
    std::cerr<<"[Live Error] " << msg << std::endl;
    exit(ErrorCode::LIVE_ERROR + error_code);
}