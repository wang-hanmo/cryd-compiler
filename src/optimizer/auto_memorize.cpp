#include <auto_memorize.h>

const int cache_size = 256;

void AutoMemorize::run()
{
    std::cout << "Running pass: Auto Memorize" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    visited.clear();
    assert(m_cfg != nullptr);
    m_idfa->compute_side_effect_info();
    Cache.resize(m_idfa->get_func_count());
    CheckCache.resize(m_idfa->get_func_count());
    for (auto& unit : *m_cfg) {
        //new_local.clear();
        if (unit.get_type() == IRUnitType::FuncDef) {
            IRInstrList &list = unit.get_definations();
            std::vector<IRSymbol*> fparams;
            for (const auto& instr: list) {
                if (instr.type() == IRType::FParam) {
                    fparams.emplace_back(instr.a());
                }
            }
            if (check_memorize(list.front().a())) {
                create_func_cache(list.front().a());
                find_max_index(&unit);
                auto return_sym = m_ir_sym_table->create_local(list.front().a()->basic_type(), ++max_temp_index);
                list.emplace_back(IRInstr::create_local_decl(return_sym));
                rewrite_return(unit.get_entry(), return_sym);
                insert_return_statement(&unit, fparams[0],return_sym);
                insert_use_cache_statement(&unit, fparams[0], return_sym);
            } else if (check_memorize_2_param(list.front().a())) {
                create_func_cache(list.front().a());
                find_max_index(&unit);
                auto return_sym = m_ir_sym_table->create_local(list.front().a()->basic_type(), ++max_temp_index);
                list.emplace_back(IRInstr::create_local_decl(return_sym));
                rewrite_return(unit.get_entry(), return_sym);
                insert_return_statement_2_param(&unit, fparams[0], fparams[1], return_sym);
                insert_use_cache_statement_2_param(&unit, fparams[0], fparams[1], return_sym);
            }
            /*
            for (auto l: new_local) {
                list.emplace_back(IRInstr::create_local_decl(l));
            }*/
        }
    }
}
void AutoMemorize::insert_return_statement(IRUnit* unit,IRSymbol* param, IRSymbol* return_sym)
{
    const int func_id = unit->get_tag();
    IRSymbol* t = nullptr;
    /* 函数末尾的 return statement */
    auto check_idx_block = new IRBlock();
    //param >= 0
    check_idx_block->add_instr(IRInstr::create_binary_calc(
        IROper::GreaterEqualI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param,
        m_ir_sym_table->create_value_0(BasicType::Int)));
    check_idx_block->add_instr(IRInstr::create_block_cond_goto(t));

    //param < cache_size
    auto check_idx_block2 = new IRBlock();
    check_idx_block2->add_instr(IRInstr::create_binary_calc(
        IROper::LessI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    check_idx_block2->add_instr(IRInstr::create_block_cond_goto(t));

    //is_cached[param]=1
    //cache[param]=return value
    auto store_cache_block = new IRBlock();
    store_cache_block->add_instr(IRInstr::create_array_store(
        CheckCache[func_id],
        param,
        m_ir_sym_table->create_value_1(BasicType::Int)));
    store_cache_block->add_instr(IRInstr::create_array_store(
        Cache[func_id],
        param,
        return_sym));
    store_cache_block->add_instr(IRInstr::create_block_goto());

    //return cache;
    auto return_block = new IRBlock();
    return_block->add_instr(IRInstr::create_value_return(return_sym));

    //set edges
    //此处必须复制
    auto preds = unit->get_exit()->get_pred();
    for (auto pred : preds) {
        int index = -1;
        if (pred->get_succ(0) == unit->get_exit())
            index = 0;
        else if (pred->get_succ(0) == unit->get_exit())
            index = 1;
        assert(index != -1);
        //assert(pred->get_instr_list_const().back().type() == IRType::ValReturn);
        pred->add_instr(IRInstr::create_block_goto());
        pred->delete_edge(index);
        pred->set_edge(index, check_idx_block);
    }
    check_idx_block->set_edge(0, return_block);
    check_idx_block->set_edge(1, check_idx_block2);
    check_idx_block2->set_edge(0, return_block);
    check_idx_block2->set_edge(1, store_cache_block);
    store_cache_block->set_edge(0, return_block);
    return_block->set_edge(0, unit->get_exit());
    //return check_idx_block;
}
void AutoMemorize::insert_use_cache_statement(IRUnit* unit, IRSymbol* param, IRSymbol* return_sym)
{
    const int func_id = unit->get_tag();
    auto first_block = unit->get_entry()->get_succ(0);
    unit->get_entry()->delete_edge(0);

    /*函数开头的 return statement*/
    auto check_idx_block = new IRBlock();
    IRSymbol* t = nullptr;

    //param >= 0
    check_idx_block->add_instr(IRInstr::create_binary_calc(
        IROper::GreaterEqualI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param,
        m_ir_sym_table->create_value_0(BasicType::Int)));
    check_idx_block->add_instr(IRInstr::create_block_cond_goto(t));

    //param < cache_size
    auto check_idx_block2 = new IRBlock();
    check_idx_block2->add_instr(IRInstr::create_binary_calc(
        IROper::LessI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    check_idx_block2->add_instr(IRInstr::create_block_cond_goto(t));

    //is_cached[param]
    auto check_cached_block = new IRBlock();
    check_cached_block->add_instr(IRInstr::create_array_load(
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        CheckCache[func_id],
        param)
    );
    check_cached_block->add_instr(IRInstr::create_block_cond_goto(t));

    //return cache;
    auto read_cache_block = new IRBlock();
    read_cache_block->add_instr(IRInstr::create_array_load(
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        Cache[func_id],
        param)
    );
    read_cache_block->add_instr(IRInstr::create_value_return(t));
    //set edges
    unit->get_entry()->set_edge(0, check_idx_block);
    check_idx_block->set_edge(0, first_block);
    check_idx_block->set_edge(1, check_idx_block2);
    check_idx_block2->set_edge(0, first_block);
    check_idx_block2->set_edge(1, check_cached_block);
    check_cached_block->set_edge(0, first_block);
    check_cached_block->set_edge(1, read_cache_block);
    read_cache_block->set_edge(0, unit->get_exit());

}
void AutoMemorize::create_func_cache(IRSymbol* func)
{
    auto type = func->global_sym()->get_val_type();
    auto name = func->global_sym()->get_name();
    name = name + "__cache__";
    auto check_type = ValueType(BasicType::Int);
    auto check_name = func->global_sym()->get_name() + "__check_cache__";

    // 创建数组作为函数结果缓存，分别创建symbol和ir symbol
    std::vector<std::size_t> length;
    length.push_back(cache_size);
    if (func->global_sym()->get_param_type().size() == 2)
        length.push_back(cache_size);
    type.set_dimension(length);
    check_type.set_dimension(length);
    auto cache_sym = m_sym_table->add_sym(Symbol(type, VarKind::Global, name));
    auto check_cache_sym = m_sym_table->add_sym(Symbol(check_type, VarKind::Global, check_name));
    auto cache_ir_sym = m_ir_sym_table->create_global(cache_sym);
    auto check_cache_ir_sym = m_ir_sym_table->create_global(check_cache_sym);
    Cache[func->get_tag()] = cache_ir_sym;
    CheckCache[func->get_tag()] = check_cache_ir_sym;
    cache_sym->set_ir_sym(cache_ir_sym);
    check_cache_sym->set_ir_sym(check_cache_ir_sym);
    auto var_def1 = IRInstr::create_global_decl(cache_ir_sym);
    auto var_def2 = IRInstr::create_global_decl(check_cache_ir_sym);
    for (auto& unit: *m_cfg) {
        if (unit.get_type() == IRUnitType::VarDef) {
            auto& list = unit.get_definations();
            list.emplace_back(var_def1);
            list.emplace_back(var_def2);
            return;
        }
    }
    // 没有，则创建一个VarDef Unit
    auto var_def_unit = IRUnit::create_var_def();
    auto& list = var_def_unit.get_definations();
    list.emplace_back(var_def1);
    list.emplace_back(var_def2);
    m_cfg->emplace_front(var_def_unit);
}

bool AutoMemorize::check_memorize(IRSymbol* func)
{
    // 自动记忆化条件：
    // 1、函数是递归函数且无副作用
    // 2、函数不受外界影响
    // 3、函数只有一个参数且参数为整数
    // 4、自己调用自己时才用缓存结果
    // 5、函数有返回值
    auto sym = func->global_sym();
    if ((func->basic_type()==BasicType::Int|| func->basic_type() == BasicType::Float)&&
        sym->get_param_type().size() == 1&& 
        sym->get_param_type(0).basic() == BasicType::Int &&
        !m_idfa->has_side_effect(func->get_tag())&& 
        !m_idfa->affected_by_env(func->get_tag())&&
         m_idfa->is_direct_recursion_function(func->get_tag()))
        return true;
    return false;
}

void AutoMemorize::rewrite_return(IRBlock* block,IRSymbol* return_sym)
{
    assert(block != nullptr);
    visited.insert(block);
    for (auto& instr: block->get_instr_list()) {
        if (instr.type() == IRType::ValReturn) {
            instr.rebind_r(return_sym);
            instr.rewrite_type(IRType::Assign);
        }
    }
    for (int i = 0; i < 2; ++i) {
        if(block->get_succ(i) != nullptr && visited.find(block->get_succ(i)) == visited.end())
            rewrite_return(block->get_succ(i), return_sym);
    }
}

void AutoMemorize::insert_return_statement_2_param(IRUnit* unit,IRSymbol* param1, IRSymbol* param2, IRSymbol* return_sym)
{
    const int func_id = unit->get_tag();
    IRSymbol* t = nullptr;
    /* 函数末尾的 return statement */
    auto check_idx_block_dim1 = new IRBlock();
    //param1 >= 0
    check_idx_block_dim1->add_instr(IRInstr::create_binary_calc(
        IROper::GreaterEqualI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param1,
        m_ir_sym_table->create_value_0(BasicType::Int)));
    check_idx_block_dim1->add_instr(IRInstr::create_block_cond_goto(t));

    auto check_idx_block_dim2 = new IRBlock();
    //param2 >= 0
    check_idx_block_dim2->add_instr(IRInstr::create_binary_calc(
        IROper::GreaterEqualI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param2,
        m_ir_sym_table->create_value_0(BasicType::Int)));
    check_idx_block_dim2->add_instr(IRInstr::create_block_cond_goto(t));

    //param1 < cache_size
    auto check_idx_block2_dim1 = new IRBlock();
    check_idx_block2_dim1->add_instr(IRInstr::create_binary_calc(
        IROper::LessI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param1,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    check_idx_block2_dim1->add_instr(IRInstr::create_block_cond_goto(t));

    //param2 < cache_size
    auto check_idx_block2_dim2 = new IRBlock();
    check_idx_block2_dim2->add_instr(IRInstr::create_binary_calc(
        IROper::LessI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param2,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    check_idx_block2_dim2->add_instr(IRInstr::create_block_cond_goto(t));

    //is_cached[param]=1
    //cache[param]=return value
    auto store_cache_block = new IRBlock();
    // 计算索引
    store_cache_block->add_instr(IRInstr::create_binary_calc(
        IROper::MulI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param1,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    IRSymbol* index = nullptr;
    store_cache_block->add_instr(IRInstr::create_binary_calc(
        IROper::AddI,
        index = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        t,
        param2));
    store_cache_block->add_instr(IRInstr::create_array_store(
        CheckCache[func_id],
        index,
        m_ir_sym_table->create_value_1(BasicType::Int)));
    store_cache_block->add_instr(IRInstr::create_array_store(
        Cache[func_id],
        index,
        return_sym));
    store_cache_block->add_instr(IRInstr::create_block_goto());

    //return cache;
    auto return_block = new IRBlock();
    return_block->add_instr(IRInstr::create_value_return(return_sym));

    //set edges
    //此处必须复制
    auto preds = unit->get_exit()->get_pred();
    for (auto pred : preds) {
        int index = -1;
        if (pred->get_succ(0) == unit->get_exit())
            index = 0;
        else if (pred->get_succ(0) == unit->get_exit())
            index = 1;
        assert(index != -1);
        //assert(pred->get_instr_list_const().back().type() == IRType::ValReturn);
        pred->add_instr(IRInstr::create_block_goto());
        pred->delete_edge(index);
        pred->set_edge(index, check_idx_block_dim1);
    }
    check_idx_block_dim1->set_edge(0, return_block);
    check_idx_block_dim1->set_edge(1, check_idx_block_dim2);
    check_idx_block_dim2->set_edge(0, return_block);
    check_idx_block_dim2->set_edge(1, check_idx_block2_dim1);
    check_idx_block2_dim1->set_edge(0, return_block);
    check_idx_block2_dim1->set_edge(1, check_idx_block2_dim2);
    check_idx_block2_dim2->set_edge(0, return_block);
    check_idx_block2_dim2->set_edge(1, store_cache_block);
    store_cache_block->set_edge(0, return_block);
    return_block->set_edge(0, unit->get_exit());
    //return check_idx_block;
}

void AutoMemorize::insert_use_cache_statement_2_param(IRUnit* unit, IRSymbol* param1, IRSymbol* param2, IRSymbol* return_sym)
{
    const int func_id = unit->get_tag();
    auto first_block = unit->get_entry()->get_succ(0);
    unit->get_entry()->delete_edge(0);

    /*函数开头的 return statement*/
    IRSymbol* t = nullptr;
    auto check_idx_block_dim1 = new IRBlock();
    //param1 >= 0
    check_idx_block_dim1->add_instr(IRInstr::create_binary_calc(
        IROper::GreaterEqualI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param1,
        m_ir_sym_table->create_value_0(BasicType::Int)));
    check_idx_block_dim1->add_instr(IRInstr::create_block_cond_goto(t));

    auto check_idx_block_dim2 = new IRBlock();
    //param2 >= 0
    check_idx_block_dim2->add_instr(IRInstr::create_binary_calc(
        IROper::GreaterEqualI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param2,
        m_ir_sym_table->create_value_0(BasicType::Int)));
    check_idx_block_dim2->add_instr(IRInstr::create_block_cond_goto(t));

    //param1 < cache_size
    auto check_idx_block2_dim1 = new IRBlock();
    check_idx_block2_dim1->add_instr(IRInstr::create_binary_calc(
        IROper::LessI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param1,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    check_idx_block2_dim1->add_instr(IRInstr::create_block_cond_goto(t));

    //param2 < cache_size
    auto check_idx_block2_dim2 = new IRBlock();
    check_idx_block2_dim2->add_instr(IRInstr::create_binary_calc(
        IROper::LessI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param2,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    check_idx_block2_dim2->add_instr(IRInstr::create_block_cond_goto(t));

    //is_cached[param]
    auto check_cached_block = new IRBlock();
    // 计算索引
    check_cached_block->add_instr(IRInstr::create_binary_calc(
        IROper::MulI,
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        param1,
        m_ir_sym_table->create_value(BasicType::Int, cache_size)));
    IRSymbol* index = nullptr;
    check_cached_block->add_instr(IRInstr::create_binary_calc(
        IROper::AddI,
        index = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        t,
        param2));
    check_cached_block->add_instr(IRInstr::create_array_load(
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        CheckCache[func_id],
        index)
    );
    check_cached_block->add_instr(IRInstr::create_block_cond_goto(t));

    //return cache;
    auto read_cache_block = new IRBlock();
    read_cache_block->add_instr(IRInstr::create_array_load(
        t = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index),
        Cache[func_id],
        index)
    );
    read_cache_block->add_instr(IRInstr::create_value_return(t));
    //set edges
    unit->get_entry()->set_edge(0, check_idx_block_dim1);
    check_idx_block_dim1->set_edge(0, first_block);
    check_idx_block_dim1->set_edge(1, check_idx_block_dim2);
    check_idx_block_dim2->set_edge(0, first_block);
    check_idx_block_dim2->set_edge(1, check_idx_block2_dim1);
    check_idx_block2_dim1->set_edge(0, first_block);
    check_idx_block2_dim1->set_edge(1, check_idx_block2_dim2);
    check_idx_block2_dim2->set_edge(0, first_block);
    check_idx_block2_dim2->set_edge(1, check_cached_block);
    check_cached_block->set_edge(0, first_block);
    check_cached_block->set_edge(1, read_cache_block);
    read_cache_block->set_edge(0, unit->get_exit());
}

bool AutoMemorize::check_memorize_2_param(IRSymbol* func)
{
    auto sym = func->global_sym();
    if ((func->basic_type()==BasicType::Int|| func->basic_type() == BasicType::Float)&&
        sym->get_param_type().size() == 2&& 
        sym->get_param_type(0).basic() == BasicType::Int &&
        sym->get_param_type(1).basic() == BasicType::Int &&
        !m_idfa->has_side_effect(func->get_tag())&& 
        !m_idfa->affected_by_env(func->get_tag())&&
         m_idfa->is_direct_recursion_function(func->get_tag()))
        return true;
    return false;
}

void AutoMemorize::ir_list_find_index(IRInstrList& program)
{
   for(auto&instr:program)
   {
        if(instr.r()!=nullptr){
            if(instr.r()->kind()==IRSymbolKind::Temp || instr.r()->kind() == IRSymbolKind::Local || instr.r()->kind() == IRSymbolKind::Param)
                max_temp_index = std::max(max_temp_index,instr.r()->index()+1);
        }
        if(instr.a()!=nullptr){
            if(instr.a()->kind() == IRSymbolKind::Temp || instr.a()->kind() == IRSymbolKind::Local || instr.a()->kind() == IRSymbolKind::Param)
                max_temp_index = std::max(max_temp_index,instr.a()->index()+1);
        }
        if(instr.b()!=nullptr){
            if(instr.b()->kind() == IRSymbolKind::Temp || instr.b()->kind() == IRSymbolKind::Local || instr.b()->kind() == IRSymbolKind::Param)
                max_temp_index = std::max(max_temp_index,instr.b()->index()+1);
        }
        if (instr.c() != nullptr) {
            if (instr.c()->kind() == IRSymbolKind::Temp || instr.c()->kind() == IRSymbolKind::Local || instr.c()->kind() == IRSymbolKind::Param)
                max_temp_index = std::max(max_temp_index, instr.c()->index() + 1);
        }
   }
}

void AutoMemorize::find_max_index(IRUnit* unit)
{
    max_temp_index = 0;
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