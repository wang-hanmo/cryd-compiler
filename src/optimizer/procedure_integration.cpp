#include <procedure_integration.h>


bool ProcedureIntegration::is_inline(IRSymbol* func)
{
    assert(m_procedures.count(func));
    int idx = m_procedures.at(func);
    //递归函数不内联
    if(m_recursive[idx])
        return false;
    //参数含数组的函数不内联
    // if(m_array[idx])
    //     return false;
    //调用次数少的函数内联
    if(m_call_num[idx] <= 3)
        return true;
    //过长的函数不内联
    if(m_instr_num[idx] > 120)
        return false;
    return true;
}

void ProcedureIntegration::integrate_procedure(IRBlock* block, int unit_no)
{
    // std::cout << "blk" << block->get_index() << std::endl;
    std::vector<IRSymbol*> rparams;
    std::unordered_map<IRSymbol*, IRSymbol*> param_map;
    int instr_no = 0;
    int key_instr_no = -1;
    for(auto& instr: block->get_instr_list())
    {
        if(instr.type() == IRType::RParam)
        {
            //记录实参
            if(key_instr_no == -1)
                key_instr_no = instr_no;
            rparams.push_back(instr.a());
        }
        if(instr.type() == IRType::Call || instr.type() == IRType::CallWithRet)
        {
            if(key_instr_no == -1)
                key_instr_no = instr_no;
            IRSymbol* func = instr.a();
            // std::cout << "call " << func->get_string() << std::endl;
            if(m_procedures.count(func) && is_inline(func))
            {
                // CFGManager::print_ir_block(block, std::cout);
                // std::cout << "no " << key_instr_no << "-" << instr_no << ": integrating procedure " << func->get_string() << std::endl;
                // std::cout << "integrating procedure " << func->get_string() << std::endl;
                //硬拷贝调用过程
                IRUnit* new_unit = IRUnit::clone(*(m_units[m_procedures.at(func)]));
                //变量重命名
                // CFGManager::print_ir_unit(*new_unit, std::cout);
                rename(new_unit, unit_no, rparams, param_map);
                // CFGManager::print_ir_unit(*new_unit, std::cout);
                //插入子过程
                insert(block, key_instr_no, new_unit, unit_no, param_map);
                delete new_unit;
                break;
            }
            //清除已用实参
            rparams.clear();
            key_instr_no = -1;
            param_map.clear();
        }
        instr_no++;
    }
}

void ProcedureIntegration::search()
{
    for(auto& unit: *m_cfg)
    {
        if(unit.get_type() == IRUnitType::FuncDef)
        {
            std::set<IRBlock *> is_printed;
            std::queue<IRBlock *> q;
            q.push(unit.get_entry());
            while (!q.empty()) {
                IRBlock* now = q.front();
                q.pop();
                if (!now->is_exit()&&!now->is_entry())
                    integrate_procedure(now, unit.get_tag());
                for (int k = 0; k <= 1; ++k)
                    if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
                    {
                        q.push(now->get_succ(k));
                        is_printed.insert(now->get_succ(k));
                    }
            }
        }
    }
}


void ProcedureIntegration::insert(IRBlock* block, int instr_no, IRUnit* src_unit, int unit_no, std::unordered_map<IRSymbol*, IRSymbol*> param_map)
{
    //插入子过程
    //将原block以rparam和call/callwithret为界分为两个block
    //将两个block分别与子过程的entry和exit相连接
    //return和callwithret对应
    IRUnit* dest_unit = m_units[unit_no];
    
    //合并符号定义
    for(auto& def: src_unit->get_definations())
    {
        if(def.type() == IRType::FParam || def.type() == IRType::LocalDecl)
        {
            if(def.type() == IRType::FParam && param_map.at(def.a()) == def.a())
                continue;
            dest_unit->add_def_instr(IRInstr::create_local_decl(def.a()));
        }
    }

    IRBlock* new_block = new IRBlock();
    IRInstrList instr_list = block->get_instr_list();
    block->get_instr_list().clear();
    // LinearIRManager::print_ir_list(instr_list);

    //定位call位置
    int no = 0;
    int flag = (no == instr_no);
    IRSymbol* ret = nullptr;
    for(auto& instr: instr_list)
    {
        if(flag)
        {
            switch(instr.type())
            {
                case IRType::RParam:
                    break;
                case IRType::Call:
                    flag = false;
                    break;
                case IRType::CallWithRet:
                    ret = instr.r();
                    flag = false;
                    break;
                default:assert(0);break;
            }
            no++;
            continue;
        }
        if(no > instr_no)
        {
            new_block->add_instr(instr);
        }
        else
        {
            block->add_instr(instr);
        }
        no++;
        if(no == instr_no)
            flag = true;
    }
    //形参和实参对应,assign指令
    for(auto& param: param_map)
    {
        if(param.first == param.second)
            continue;
        block->add_instr(IRInstr::create_assign(param.first, param.second));
    }

    //返回值对应
    if(ret != nullptr)
    {
        IRSymbol* ret_local = m_ir_sym_table->create_local(ret->basic_type(), m_symbol_num[unit_no]++, ret->array_length());
        dest_unit->add_def_instr(IRInstr::create_local_decl(ret_local));
        new_block->add_instr_to_front(IRInstr::create_assign(ret, ret_local));
        std::set<IRBlock *> is_printed;
        std::queue<IRBlock *> q;
        q.push(src_unit->get_entry());
        while (!q.empty()) {
            IRBlock* now = q.front();
            q.pop();
            bool found = false;
            if (!now->is_exit()&&!now->is_entry())
            {
                for(auto& instr: now->get_instr_list())
                {
                    if(instr.type() == IRType::ValReturn)
                    {
                        instr = IRInstr::create_assign(ret_local, instr.a());
                        found = true;
                    }
                }
            }
            if(found && now->get_instr_list().back().type() != IRType::BlockGoto &&\
                 now->get_instr_list().back().type() != IRType::BlockCondGoto &&\
                 now->get_instr_list().back().type() != IRType::Return &&\
                 now->get_instr_list().back().type() != IRType::ValReturn)
                now->add_instr(IRInstr::create_block_goto());
            for (int k = 0; k <= 1; ++k)
                if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
                {
                    q.push(now->get_succ(k));
                    is_printed.insert(now->get_succ(k));
                }
        }
    }
    else
    {
        std::set<IRBlock *> is_printed;
        std::queue<IRBlock *> q;
        q.push(src_unit->get_entry());
        while (!q.empty()) {
            IRBlock* now = q.front();
            q.pop();
            bool found = false;
            if (!now->is_exit()&&!now->is_entry())
            {
                for(auto iter= now->get_instr_list().begin();iter!= now->get_instr_list().end();)
                {
                    if(iter->type() == IRType::Return || iter->type() == IRType::ValReturn)
                    {
                        iter= now->get_instr_list().erase(iter);
                        found = true;
                    }
                    else iter++;
                }
                if(now->get_instr_list().empty())
                    now->add_instr(IRInstr::create_block_goto());
                else if(found && now->get_instr_list().back().type() != IRType::BlockGoto &&\
                    now->get_instr_list().back().type() != IRType::BlockCondGoto &&\
                    now->get_instr_list().back().type() != IRType::Return &&\
                    now->get_instr_list().back().type() != IRType::ValReturn)
                    now->add_instr(IRInstr::create_block_goto());
                // for(auto& instr: now->get_instr_list())
                // {
                //     if(instr.type() == IRType::Return)
                //     {
                //         instr = IRInstr::create_assign(ret_local, instr.a());
                //     }
                // }
            }
            for (int k = 0; k <= 1; ++k)
                if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
                {
                    q.push(now->get_succ(k));
                    is_printed.insert(now->get_succ(k));
                }
        }

    }

    //连接block
    new_block->set_edge(0, block->get_succ(0));
    new_block->set_edge(1, block->get_succ(1));
    block->delete_edge(0);
    block->delete_edge(1);

    IRBlock* entry = src_unit->get_entry();
    IRBlock* exit = src_unit->get_exit();

    block->set_edge(0, entry);
    block->set_edge(1, nullptr);
    exit->set_edge(0, new_block);
    exit->set_edge(1, nullptr);

    m_instr_num[unit_no] += m_instr_num[src_unit->get_tag()];

    entry->add_instr(IRInstr::create_block_goto());
    exit->add_instr(IRInstr::create_block_goto());
    if(block->get_instr_list().empty())
        block->add_instr(IRInstr::create_block_goto());
    else if(block->get_instr_list().back().type() != IRType::BlockGoto &&\
                block->get_instr_list().back().type() != IRType::BlockCondGoto &&\
                block->get_instr_list().back().type() != IRType::Return &&\
                block->get_instr_list().back().type() != IRType::ValReturn)
        block->add_instr(IRInstr::create_block_goto());
    if(new_block->get_instr_list().empty())
        new_block->add_instr(IRInstr::create_block_goto());

}

void ProcedureIntegration::rename(IRUnit* new_unit, int unit_no, std::vector<IRSymbol*> rparams, std::unordered_map<IRSymbol*, IRSymbol*>& param_map)
{
    //变量重命名
    //形参重命名为实参
    //局部变量和临时变量重命名为新的不冲突的序号
    //返回值重命名需对应
    std::unordered_map<IRSymbol*, IRSymbol*> def_map;
    int param_count = 0;
    int local_count = m_symbol_num[unit_no];
    for(auto& def: new_unit->get_definations())
    {
        if(def.type() == IRType::FParam)
        {
            if(def.a()->array_length() == IRArrayLength::IR_NONE_ARRAY)
            {
                IRSymbol* new_symbol = m_ir_sym_table->create_local(def.a()->basic_type(), local_count, def.a()->array_length());
                def_map.emplace(def.a(), new_symbol);
                param_map.emplace(new_symbol, rparams[param_count]);
                def.rebind_a(new_symbol);
            }
            else if(rparams[param_count]->kind() == IRSymbolKind::Temp)
            {
                IRSymbol* new_symbol = m_ir_sym_table->create_local(rparams[param_count]->basic_type(), local_count, 0);
                def_map.emplace(def.a(), new_symbol);
                param_map.emplace(new_symbol, rparams[param_count]);
                def.rebind_a(new_symbol);
            }
            else
            {
                def_map.emplace(def.a(), rparams[param_count]);
                param_map.emplace(rparams[param_count], rparams[param_count]);
                def.rebind_a(rparams[param_count]);
            }
            local_count++;
            param_count++;
        }
        else if(def.type() == IRType::LocalDecl)
        {
            IRSymbol* new_symbol = m_ir_sym_table->create_local(def.a()->basic_type(), local_count, def.a()->array_length());
            def_map.emplace(def.a(), new_symbol);
            def.rebind_a(new_symbol);
            local_count++;
        }
    }
    
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(new_unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        if (!now->is_exit()&&!now->is_entry())
        {
            for(auto& instr: now->get_instr_list())
            {
                if(instr.a() != nullptr)
                {
                    if(def_map.count(instr.a()))
                    {
                        //前面instr已重命名的param,local,部分temp
                        // std::cout << instr.a()->get_string() << " --> ";
                        instr.rebind_a(def_map.at(instr.a()));
                        // std::cout << instr.a()->get_string() << std::endl;
                    }
                    else if(instr.a()->kind() == IRSymbolKind::Temp)
                    {
                        //新建一个temp重命名
                        IRSymbol* new_symbol = m_ir_sym_table->create_temp(instr.a()->basic_type(), local_count, instr.a()->array_length());
                        def_map.emplace(instr.a(), new_symbol);
                        instr.rebind_a(new_symbol);
                        local_count++;
                    }
                    else if(instr.a()->kind() == IRSymbolKind::Value)
                    {
                        //新建一个value重命名
                        IRSymbol* new_symbol = m_ir_sym_table->create_value(instr.a()->basic_type(), instr.a()->value());
                        def_map.emplace(instr.a(), new_symbol);
                        instr.rebind_a(new_symbol);
                    }
                }
                if(instr.b() != nullptr)
                {
                    if(def_map.count(instr.b()))
                    {
                        //前面instr已重命名的param,local,部分temp
                        instr.rebind_b(def_map.at(instr.b()));
                    }
                    else if(instr.b()->kind() == IRSymbolKind::Temp)
                    {
                        //新建一个temp重命名
                        IRSymbol* new_symbol = m_ir_sym_table->create_temp(instr.b()->basic_type(), local_count, instr.b()->array_length());
                        def_map.emplace(instr.b(), new_symbol);
                        instr.rebind_b(new_symbol);
                        local_count++;
                    }
                    else if(instr.b()->kind() == IRSymbolKind::Value)
                    {
                        //新建一个value重命名
                        IRSymbol* new_symbol = m_ir_sym_table->create_value(instr.b()->basic_type(), instr.b()->value());
                        def_map.emplace(instr.b(), new_symbol);
                        instr.rebind_b(new_symbol);
                    }
                }
                if(instr.r() != nullptr)
                {
                    if(def_map.count(instr.r()))
                    {
                        //前面instr已重命名的param,local,部分temp
                        instr.rebind_r(def_map.at(instr.r()));
                    }
                    else if(instr.r()->kind() == IRSymbolKind::Temp)
                    {
                        //新建一个temp重命名
                        IRSymbol* new_symbol = m_ir_sym_table->create_temp(instr.r()->basic_type(), local_count, instr.r()->array_length());
                        def_map.emplace(instr.r(), new_symbol);
                        instr.rebind_r(new_symbol);
                        local_count++;
                    }
                    else if(instr.r()->kind() == IRSymbolKind::Value)
                    {
                        //新建一个value重命名
                        IRSymbol* new_symbol = m_ir_sym_table->create_value(instr.r()->basic_type(), instr.r()->value());
                        def_map.emplace(instr.r(), new_symbol);
                        instr.rebind_r(new_symbol);
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
    m_symbol_num[unit_no] = local_count;
}

void ProcedureIntegration::initial()
{
    m_procedures.clear();
    m_units.clear();
    m_instr_num.clear();
    m_symbol_num.clear();
    m_recursive.clear();
    m_array.clear();
    m_call_num.clear();
    int unit_no = 0;
    for(auto& unit: *m_cfg)
    {
        if(unit.get_type() == IRUnitType::FuncDef)
        {
            IRSymbol* func = nullptr;
            bool is_array = false;
            for(auto& def: unit.get_definations())
            {
                if(def.type() == IRType::FuncDef)
                {
                    unit.set_tag(unit_no);//设置tag
                    func = def.a();
                    m_procedures.emplace(func, unit_no++);
                    m_units.push_back(&unit);
                    m_call_num.push_back(0);
                }
                else if(def.type() == IRType::FParam && def.a()->array_length() == IRArrayLength::IR_ARRAY_POINTER)
                {
                    is_array = true;
                }
            }
            assert(func != nullptr);
            assert(m_procedures.size() == unit_no);
            m_array.push_back(is_array);

            int instr_no = 0;
            int max_symbol_no = 0;
            bool recursive = false;
            std::set<IRBlock *> is_printed;
            std::queue<IRBlock *> q;
            q.push(unit.get_entry());
            while (!q.empty()) {
                IRBlock* now = q.front();
                q.pop();
                if (!now->is_exit()&&!now->is_entry())
                {
                    for(auto& instr: now->get_instr_list())
                    {
                        instr_no++;
                        if(instr.type() == IRType::Call || instr.type() == IRType::CallWithRet)
                        {
                            if(instr.a() == func)
                                recursive = true;
                            else if(m_procedures.count(instr.a()))
                            {
                                m_call_num[m_procedures.at(instr.a())]++;
                            }
                        }
                        if((instr.a() != nullptr) && (instr.a()->index() > max_symbol_no) &&\
                            (instr.a()->kind() == IRSymbolKind::Local || instr.a()->kind() == IRSymbolKind::Temp || instr.a()->kind() == IRSymbolKind::Param))
                        {
                            max_symbol_no = instr.a()->index();
                        }
                        if((instr.b() != nullptr) && (instr.b()->index() > max_symbol_no) &&\
                            (instr.b()->kind() == IRSymbolKind::Local || instr.b()->kind() == IRSymbolKind::Temp || instr.b()->kind() == IRSymbolKind::Param))
                        {
                            max_symbol_no = instr.b()->index();
                        }
                        if((instr.r() != nullptr) && (instr.r()->index() > max_symbol_no) &&\
                            (instr.r()->kind() == IRSymbolKind::Local || instr.r()->kind() == IRSymbolKind::Temp || instr.r()->kind() == IRSymbolKind::Param))
                        {
                            max_symbol_no = instr.r()->index();
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
            m_instr_num.push_back(instr_no);
            m_symbol_num.push_back(max_symbol_no + 1);
            m_recursive.push_back(recursive);
        }
    }

    // for(auto& m: m_procedures)
    // {
    //     int idx = m.second;
    //     // std::cout << m.first->get_string() << " " << (int*)m_units[idx] << " " << m_instr_num[idx] << " " << m_symbol_num[idx] <<std::endl;
    //     std::cout << m.first->get_string() << " " << m_call_num[idx] << std::endl;
    // }

}

void ProcedureIntegration::delete_dead_func()
{
    for(auto iter = (*m_cfg).begin(); iter != (*m_cfg).end();)
    {
        auto& unit = *iter;
        if(unit.get_type() == IRUnitType::FuncDef)
        {
            IRSymbol* func = nullptr;
            for(auto& def: unit.get_definations())
            {
                if(def.type() == IRType::FuncDef)
                {
                    func = def.a();
                    break;
                }
            }
            if(func->global_sym()->get_name() != "main" && is_inline(func))
            {
                //删除死函数
                iter= (*m_cfg).erase(iter);
            }
            else iter++;
        }
        else iter++;
    }
}

void ProcedureIntegration::run()
{
    std::cout << "Running pass: Procedure Integration" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_ir_sym_table == nullptr) {
        Optimizer::optimizer_error("No IR symbol table specified");
    }

    initial();
    search();
    delete_dead_func();
}