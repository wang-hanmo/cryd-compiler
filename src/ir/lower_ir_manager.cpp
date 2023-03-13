#include <lower_ir_manager.h>
#include <set>
#include <linear_ir_manager.h>
#include <arm.h>

using namespace std;

void LowerIRManager::ir_list_find_index(IRInstrList& program)
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

void LowerIRManager::find_max_index(IRUnit* unit)
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

void LowerIRManager::bind_runtime_functions()
{
    // memset
    if(m_runtime_functions[0]->get_ir_sym() != nullptr)
    {
        // std::cout << "getint" << std::endl;
        m_runtime_functions[0]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[0]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
        m_runtime_functions[0]->get_ir_sym()->add_fparam_reg(m_argreg[1]);
        m_runtime_functions[0]->get_ir_sym()->add_rparam_reg(m_argreg[1]);
        m_runtime_functions[0]->get_ir_sym()->add_fparam_reg(m_argreg[2]);
        m_runtime_functions[0]->get_ir_sym()->add_rparam_reg(m_argreg[2]);
    }
    // getint
    if(m_runtime_functions[1]->get_ir_sym() != nullptr)
    {
        // std::cout << "getint" << std::endl;
        m_runtime_functions[1]->get_ir_sym()->set_ret_reg(m_argreg[0]);
    }
    // getch
    if(m_runtime_functions[2]->get_ir_sym() != nullptr)
    {
        // std::cout << "getch" << std::endl;
        m_runtime_functions[2]->get_ir_sym()->set_ret_reg(m_argreg[0]);
    }
    // getfloat
    if(m_runtime_functions[3]->get_ir_sym() != nullptr)
    {
        // std::cout << "getfloat" << std::endl;
        m_runtime_functions[3]->get_ir_sym()->set_ret_reg(m_argreg[ARGUMENT_REGISTER_COUNT]);
    }
    // getarray
    if(m_runtime_functions[4]->get_ir_sym() != nullptr)
    {
        // std::cout << "getarray" << std::endl;
        m_runtime_functions[4]->get_ir_sym()->set_ret_reg(m_argreg[0]);
        m_runtime_functions[4]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[4]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
    }
    // getfarray
    if(m_runtime_functions[5]->get_ir_sym() != nullptr)
    {
        // std::cout << "getfarray" << std::endl;
        m_runtime_functions[5]->get_ir_sym()->set_ret_reg(m_argreg[0]);
        m_runtime_functions[5]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[5]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
    }
    // putint
    if(m_runtime_functions[6]->get_ir_sym() != nullptr)
    {
        // std::cout << "putint" << std::endl;
        m_runtime_functions[6]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[6]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
    }
    // putch
    if(m_runtime_functions[7]->get_ir_sym() != nullptr)
    {
        // std::cout << "putch" << std::endl;
        m_runtime_functions[7]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[7]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
    }
    // putfloat
    if(m_runtime_functions[8]->get_ir_sym() != nullptr)
    {
        // std::cout << "putfloat" << std::endl;
        m_runtime_functions[8]->get_ir_sym()->add_fparam_reg(m_argreg[ARGUMENT_REGISTER_COUNT]);
        m_runtime_functions[8]->get_ir_sym()->add_rparam_reg(m_argreg[ARGUMENT_REGISTER_COUNT]);
    }
    // putarray
    if(m_runtime_functions[9]->get_ir_sym() != nullptr)
    {
        // std::cout << "putarray" << std::endl;
        m_runtime_functions[9]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[9]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
        m_runtime_functions[9]->get_ir_sym()->add_fparam_reg(m_argreg[1]);
        m_runtime_functions[9]->get_ir_sym()->add_rparam_reg(m_argreg[1]);
    }
    // putfarray
    if(m_runtime_functions[10]->get_ir_sym() != nullptr)
    {
        // std::cout << "putfarray" << std::endl;
        m_runtime_functions[10]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[10]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
        m_runtime_functions[10]->get_ir_sym()->add_fparam_reg(m_argreg[1]);
        m_runtime_functions[10]->get_ir_sym()->add_rparam_reg(m_argreg[1]);
    }
    // _sysy_starttime
    if(m_runtime_functions[11]->get_ir_sym() != nullptr)
    {
        // std::cout << "_sysy_starttime" << std::endl;
        m_runtime_functions[11]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[11]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
    }
    // _sysy_stoptime
    if(m_runtime_functions[12]->get_ir_sym() != nullptr)
    {
        // std::cout << "_sysy_stoptime" << std::endl;
        m_runtime_functions[12]->get_ir_sym()->add_fparam_reg(m_argreg[0]);
        m_runtime_functions[12]->get_ir_sym()->add_rparam_reg(m_argreg[0]);
    }
}

void LowerIRManager::bind_argreg()
{
    IRSymbol* func = nullptr;
    for(int i = 0; i < ARGUMENT_REGISTER_COUNT; i++)
    {
        IRSymbol* reg = m_ir_sym_table->create_register(i);
        m_argreg.push_back(reg);
    }
    for(int i = 0; i < ARGUMENT_REGISTER_COUNT_S; i++)
    {
        IRSymbol* reg = m_ir_sym_table->create_register(i, true);
        m_argreg.push_back(reg);
    }
    // r12 caller_save
    IRSymbol* reg12 = m_ir_sym_table->create_register(12);
    m_argreg.push_back(reg12);
    // r14 callee_save
    IRSymbol* reg14 = m_ir_sym_table->create_register(14);
    m_argreg.push_back(reg14);
    assert(m_argreg.size() == ARGUMENT_REGISTER_COUNT + ARGUMENT_REGISTER_COUNT_S + 2);
    for(auto& unit: *m_cfg)
    {
        if(unit.get_type() == IRUnitType::FuncDef)
        {
            int r_count = 0;//通用寄存器数
            int s_count = 0;//浮点寄存器数
            int m_count = 0;//栈传参个数
            for(auto& def: unit.get_definations())
            {
                if(def.type() == IRType::FuncDef)
                {
                    func = def.a();
                    if(def.a()->basic_type() == BasicType::Int)
                    {
                        //设置r0为返回寄存器
                        IRSymbol* reg = m_argreg[0];
                        func->set_ret_reg(reg);
                    }
                    else if(def.a()->basic_type() == BasicType::Float)
                    {
                        //设置s0为返回寄存器
                        IRSymbol* reg = m_argreg[ARGUMENT_REGISTER_COUNT];
                        func->set_ret_reg(reg);
                    }
                }
                if(def.type() == IRType::FParam)
                {
                    assert(func != nullptr);
                    if(def.a()->basic_type() == BasicType::Int || def.a()->array_length() != IRArrayLength::IR_NONE_ARRAY)
                    {
                        if(r_count < ARGUMENT_REGISTER_COUNT)
                        {
                            //新增一个通用寄存器作为参数
                            IRSymbol* reg = m_argreg[r_count];
                            func->add_fparam_reg(reg);
                            func->add_rparam_reg(reg);
                            r_count++;
                        }
                        else
                        {
                            //新增一个栈空间作为参数
                            if(m_argreg.size() > ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 3 + 2 * m_count)
                            {
                                IRSymbol* reg_f = m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 2 + 2 * m_count];
                                func->add_fparam_reg(reg_f);
                                IRSymbol* reg_r = m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 3 + 2 * m_count];
                                func->add_rparam_reg(reg_r);
                            }
                            else
                            {
                                IRSymbol* reg_f = m_ir_sym_table->create_memory(- 1 - m_count);
                                m_argreg.push_back(reg_f);
                                func->add_fparam_reg(reg_f);
                                IRSymbol* reg_r = m_ir_sym_table->create_memory(- 1 - m_count);
                                m_argreg.push_back(reg_r);
                                func->add_rparam_reg(reg_r);
                                assert(m_argreg.size() == ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 2 * m_count + 4);
                            }
                            m_count++;
                        }
                    }
                    else
                    {
                        if(s_count < ARGUMENT_REGISTER_COUNT_S)
                        {
                            //新增一个浮点寄存器作为参数
                            IRSymbol* reg = m_argreg[ARGUMENT_REGISTER_COUNT + s_count];
                            func->add_fparam_reg(reg);
                            func->add_rparam_reg(reg);
                            s_count++;
                        }
                        else
                        {
                            //新增一个栈空间作为参数
                            if(m_argreg.size() > ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 3 + 2 * m_count)
                            {
                                IRSymbol* reg_f = m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 2 + 2 * m_count];
                                func->add_fparam_reg(reg_f);
                                IRSymbol* reg_r = m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 3 + 2 * m_count];
                                func->add_rparam_reg(reg_r);
                            }
                            else
                            {
                                IRSymbol* reg_f = m_ir_sym_table->create_memory(- 1 - m_count);
                                m_argreg.push_back(reg_f);
                                func->add_fparam_reg(reg_f);
                                IRSymbol* reg_r = m_ir_sym_table->create_memory(- 1 - m_count);
                                m_argreg.push_back(reg_r);
                                func->add_rparam_reg(reg_r);
                                assert(m_argreg.size() == ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S + 2 * m_count + 4);
                            }
                            m_count++;
                        }

                    }
                }
            }

            // std::cout << func->get_string() << std::endl;
            // for(auto& param: func->get_fparam_reg())
            //     std::cout << param->get_string();
            // for(auto& param: func->get_rparam_reg())
            //     std::cout << param->get_string();
            // std::cout << std::endl;
            // if(func->get_ret_reg() != nullptr)
            //     std::cout << func->get_ret_reg()->get_string() << std::endl;
        }
    }
}

IRSymbol* LowerIRManager::load_global(IRSymbol* s, std::unordered_map<IRSymbol*, IRSymbol*> global_addr, IRInstrList& instr_list, IRInstrList::iterator& it)
{
    if (global_addr.find(s) == global_addr.end())
        assert(0);
    // temp = [addr]
    auto addr = global_addr.at(s);
    if (s->array_length() == -1) {
        auto temp = m_ir_sym_table->create_temp(s->basic_type(), ++max_temp_index);
        IRInstr inst = IRInstr::create_load(temp, addr);
        it = instr_list.insert(it, inst);
        ++it;
        return temp;
    } else {
        return addr;
    }
}

IRSymbol* LowerIRManager::store_global(IRSymbol* s, std::unordered_map<IRSymbol*, IRSymbol*> global_addr, IRInstrList& instr_list, IRInstrList::iterator& it)
{
    if (global_addr.find(s) == global_addr.end())
        assert(0);
    // [addr] = temp
    auto addr = global_addr.at(s);
    auto temp = m_ir_sym_table->create_temp(s->basic_type(), ++max_temp_index);
    IRInstr inst = IRInstr::create_store(addr, temp);
    ++it;
    it = instr_list.insert(it, inst);
    return temp;
}

IRSymbol* LowerIRManager::move_imm(IRSymbol* s, IRInstrList& instr_list, IRInstrList::iterator& it)
{
    auto temp = m_ir_sym_table->create_temp(s->basic_type(), ++max_temp_index);
    IRInstr inst = IRInstr::create_assign(temp, s);
    it = instr_list.insert(it, inst);
    ++it;
    return temp;
}

void LowerIRManager::scan_global(IRUnit& unit, unordered_set<IRSymbol*>& global_in_func)
{
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        if (!now->is_exit()&&!now->is_entry()) {
            for(auto iter = now->get_instr_list().begin(); iter != now->get_instr_list().end(); iter++) {
                auto a = iter->a();
                auto b = iter->b();
                auto c = iter->c();
                auto r = iter->r();
                if (a != nullptr && (a->kind() == IRSymbolKind::Global && a->global_sym()->get_kind() == VarKind::Global ||
                    a->kind() == IRSymbolKind::Local && a->array_length() > 0))
                    global_in_func.emplace(a);
                if (b != nullptr && (b->kind() == IRSymbolKind::Global && b->global_sym()->get_kind() == VarKind::Global ||
                    b->kind() == IRSymbolKind::Local && b->array_length() > 0))
                    global_in_func.emplace(b);
                if (c != nullptr && (c->kind() == IRSymbolKind::Global && c->global_sym()->get_kind() == VarKind::Global ||
                    c->kind() == IRSymbolKind::Local && c->array_length() > 0))
                    global_in_func.emplace(c);
                if (r != nullptr && (r->kind() == IRSymbolKind::Global && r->global_sym()->get_kind() == VarKind::Global ||
                    r->kind() == IRSymbolKind::Local && r->array_length() > 0))
                    global_in_func.emplace(r);
            }
        }
        for (int k = 0; k <= 1; ++k)
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
    }
}

void LowerIRManager::gen_block(IRUnit& unit, IRSymbol* func, unordered_map<IRSymbol*, IRSymbol*> global_addr)
{
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        if (!now->is_exit()&&!now->is_entry()) {
            int r_count = 0;//通用寄存器数
            int s_count = 0;//浮点寄存器数
            int m_count = 0;//栈传参个数
            auto& instr_list = now->get_instr_list();
            for(auto iter = instr_list.begin(); iter != instr_list.end();) {
                auto& instr = *iter;
                auto a = instr.a();
                auto b = instr.b();
                auto c = instr.c();
                auto r = instr.r();
                auto op = instr.op();
                switch(instr.type()) {
                    case IRType::BlockBinaryGoto:
                        [[fallthrough]];
                    case IRType::BinaryCalc: {
                        // 这些操作可以交换源操作数
                        if (op == IROper::AddI || op == IROper::SubI || op == IROper::BitAnd || op == IROper::BitOr || op == IROper::BitXor || op == IROper::BitClear) {
                            if (a->kind() == IRSymbolKind::Value) {
                                // exchange order: a->op2 b->rn res->rd
                                instr.rebind_a(b);
                                instr.rebind_b(a);
                                a = instr.a();
                                b = instr.b();
                                if (op == IROper::SubI)
                                    instr.reset_op(IROper::RsbI);
                            }
                        }
                        if (op == IROper::LessI && a->kind() == IRSymbolKind::Value && a->value().int_value == 0) {
                            instr.rebind_a(b);
                            instr.rebind_b(a);
                            a = instr.a();
                            b = instr.b();
                            instr.reset_op(IROper::GreaterI);
                        }
                        // 操作数a必定需要分配寄存器
                        if (a->kind() == IRSymbolKind::Value) {
                            auto temp = move_imm(a, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        // 操作数b在一定条件下需要分配寄存器
                        if (b->kind() == IRSymbolKind::Value) {
                            // 浮点数比较有cmpwith0指令，不需要为0分配寄存器
                            if (op != IROper::EqualF && op != IROper::NotEqualF && op != IROper::GreaterEqualF && op != IROper::LessEqualF && op != IROper::GreaterF && 
                            op != IROper::LessF || b->value().float_value != 0) {
                                if (op == IROper::MulI || op == IROper::DivI || op == IROper::ModI || op == IROper::SmmulI || b->basic_type() == BasicType::Float ||
                                !Operand2::checkImm8m(b->value().int_value)) {
                                    auto temp = move_imm(b, instr_list, iter);
                                    instr.rebind_b(temp);
                                }
                            }
                        }
                        if (a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        if (b->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(b, global_addr, instr_list, iter);
                            instr.rebind_b(temp);
                        }
                        if (r != nullptr && r->kind() == IRSymbolKind::Global) {
                            auto temp = store_global(r, global_addr, instr_list, iter);
                            instr.rebind_r(temp);
                        }
                        break;
                    }
                    case IRType::UnaryCalc: {
                        // 一元运算不会有立即数（优化后，不用考虑）
                        if (a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        if (r->kind() == IRSymbolKind::Global) {
                            auto temp = store_global(r, global_addr, instr_list, iter);
                            instr.rebind_r(temp);
                        }
                        break;
                    }
                    case IRType::TernaryCalc: {
                        if (a != nullptr && a->kind() == IRSymbolKind::Value) {
                            auto temp = move_imm(a, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        if (b->kind() == IRSymbolKind::Value) {
                            auto temp = move_imm(b, instr_list, iter);
                            instr.rebind_b(temp);
                        }
                        if ((op == IROper::MulAddI  || op == IROper::SignedLargeMulAddI) && c->kind() == IRSymbolKind::Value) {
                            auto temp = move_imm(c, instr_list, iter);
                            instr.rebind_c(temp);
                        }
                        if (a != nullptr && a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        if (b->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(b, global_addr, instr_list, iter);
                            instr.rebind_b(temp);
                        }
                        if (c->kind() == IRSymbolKind::Global) {
                            if (op == IROper::SignedLargeMulI) {
                                // 这个比较特殊，c才是目的操作数
                                auto temp = store_global(c, global_addr, instr_list, iter);
                                instr.rebind_c(temp);
                            } else {
                                auto temp = load_global(c, global_addr, instr_list, iter);
                                instr.rebind_c(temp);
                            }
                        }
                        if (r->kind() == IRSymbolKind::Global) {
                            auto temp = store_global(r, global_addr, instr_list, iter);
                            instr.rebind_r(temp);
                        }
                        break;
                    }
                    case IRType::BlockCondGoto: {
                        if (a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        break;
                    }
                    case IRType::BlockUnaryGoto: {
                        if (a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        break;
                    }
                    case IRType::ArrayLoad: {
                        if (b->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(b, global_addr, instr_list, iter);
                            instr.rebind_b(temp);
                        }
                        if (a->kind() == IRSymbolKind::Global) {
                            if (global_addr.find(a) == global_addr.end())
                                assert(0);
                            auto base = global_addr.at(a);
                            instr.rebind_a(base);
                        } else if (a->kind() == IRSymbolKind::Local && a->array_length() > 0 && b->kind() != IRSymbolKind::Value) {
                            // t = a
                            // r = t[b]
                            auto temp = global_addr.at(a);
                            instr.rebind_a(temp);
                            // 如果b是常数
                            // r = sp[b+offset]  不需要分配寄存器
                        }
                        if (r->kind() == IRSymbolKind::Global) {
                            auto temp = store_global(r, global_addr, instr_list, iter);
                            instr.rebind_r(temp);
                        }
                        break;
                    }
                    case IRType::ArrayStore: {
                        // r[a] = b;
                        if (a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        if (b->kind() == IRSymbolKind::Value) {
                            auto temp = move_imm(b, instr_list, iter);
                            instr.rebind_b(temp);
                        }
                        if (r->kind() == IRSymbolKind::Global) {
                            if (global_addr.find(r) == global_addr.end())
                                assert(0);
                            auto base = global_addr.at(r);
                            instr.rebind_r(base);
                        } else if (r->kind() == IRSymbolKind::Local && r->array_length() > 0 && a->kind() != IRSymbolKind::Value) {
                            // t = r
                            // t[a] = b
                            auto temp = global_addr.at(r);
                            instr.rebind_r(temp);
                            // 如果a是常数
                            // sp[b+offset] = b  不需要分配寄存器
                        }
                        break;
                    }
                    case IRType::RParam: {
                        if (a->kind() == IRSymbolKind::Global) {
                            auto temp = load_global(a, global_addr, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        //rparam l0 -> r0 = l0
                        if(instr.a()->basic_type() == BasicType::Int || instr.a()->array_length() != IRArrayLength::IR_NONE_ARRAY) {
                            if(r_count < ARGUMENT_REGISTER_COUNT) {
                                instr = IRInstr::create_assign(m_argreg[r_count], instr.a());
                                r_count++;
                            }
                            else {
                                instr = IRInstr::create_store(m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S+3+2*m_count], instr.a());
                                m_count++;
                            }
                        }
                        else {
                            if(s_count < ARGUMENT_REGISTER_COUNT_S) {
                                instr = IRInstr::create_assign(m_argreg[ARGUMENT_REGISTER_COUNT+s_count], instr.a());
                                s_count++;
                            }
                            else {
                                instr = IRInstr::create_store(m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S+3+2*m_count], instr.a());
                                m_count++;
                            }
                        }
                        break;
                    }
                    case IRType::CallWithRet: {
                        IRSymbol* ret_sym = instr.r();
                        IRSymbol* ret_reg = instr.a()->get_ret_reg();
                        instr.rebind_r(ret_reg);
                        if (ret_sym->kind() == IRSymbolKind::Global) {
                            if (global_addr.find(ret_sym) == global_addr.end())
                                assert(0);
                            auto addr = global_addr.at(ret_sym);
                            iter = now->get_instr_list().insert(++iter, IRInstr::create_store(addr, ret_reg));
                        } else
                            iter = now->get_instr_list().insert(++iter, IRInstr::create_assign(ret_sym, ret_reg));
                        r_count = 0;
                        s_count = 0;
                        m_count = 0;
                        break;
                    }
                    case IRType::Call: {
                        r_count = 0;
                        s_count = 0;
                        m_count = 0;
                        break;
                    }
                    case IRType::ValReturn: {
                        assert(func != nullptr);
                        IRSymbol* ret_sym = instr.a();
                        IRSymbol* ret_reg = func->get_ret_reg();
                        // instr = IRInstr::create_assign(ret_reg, ret_sym);
                        // now->get_instr_list().insert(++iter, IRInstr::create_value_return(ret_reg));
                        instr.rebind_a(ret_reg);
                        if (ret_sym->kind() == IRSymbolKind::Global) {
                            if (global_addr.find(ret_sym) == global_addr.end())
                                assert(0);
                            auto addr = global_addr.at(ret_sym);
                            iter = now->get_instr_list().insert(iter, IRInstr::create_load(ret_reg, addr));
                            iter++;
                        } else {
                            iter = now->get_instr_list().insert(iter, IRInstr::create_assign(ret_reg, ret_sym));
                            iter++;
                        }
                        break;
                    }
                    case IRType::Assign: {
                        if (a->kind() == IRSymbolKind::Global) {
                            if (global_addr.find(a) == global_addr.end())
                                assert(0);
                            auto addr = global_addr.at(a);
                            instr.rebind_a(addr);
                            if (a->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                                instr.rewrite_type(IRType::Load);
                                //此处可能存在bug
                                // assert(iter != instr_list.begin());
                                continue;
                            }
                        }
                        if (r->kind() == IRSymbolKind::Global) {
                            if (global_addr.find(r) == global_addr.end())
                                assert(0);
                            auto addr = global_addr.at(r);
                            instr.rebind_r(addr);
                            if (r->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                                instr.rewrite_type(IRType::Store);
                                //此处可能存在bug
                                // assert(iter != instr_list.begin());
                                continue;
                            }
                        }
                        break;
                    }
                    case IRType::Load: {
                        if (r->kind() == IRSymbolKind::Global) {
                            auto temp = store_global(r, global_addr, instr_list, iter);
                            instr.rebind_r(temp);
                        }
                        break;
                    }
                    case IRType::Store: {
                        if (a->kind() == IRSymbolKind::Value) {
                            auto temp = move_imm(a, instr_list, iter);
                            instr.rebind_a(temp);
                        }
                        break;
                    }
                    default:break;
                }
                ++iter;
            }
        }
        for (int k = 0; k <= 1; ++k) {
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) 
            {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
        }
    }
}

void LowerIRManager::revise_instr()
{
    for(auto& unit: *m_cfg)
    {
        if(unit.get_type() == IRUnitType::FuncDef)
        {
            find_max_index(&unit);

            // 记录一个函数内用到的全局变量
            unordered_set<IRSymbol*> global_in_func;
            // 记录全局变量的地址和临时变量的映射关系
            unordered_map<IRSymbol*, IRSymbol*> global_addr;
            IRSymbol* func = nullptr;
            IRInstrList fparam_list;
            IRBlock* entry = unit.get_entry();
            IRBlock* first = unit.get_entry()->get_succ(0);
            IRBlock* new_block = new IRBlock();
            int r_count = 0;//通用寄存器数
            int s_count = 0;//浮点寄存器数
            int m_count = 0;//栈传参个数
            for(auto& def: unit.get_definations()) {
                if(def.type() == IRType::FuncDef)
                    func = def.a();
                else if(def.type() == IRType::FParam) {
                    // fparam l0 -> mov l0, r0
                    if(def.a()->basic_type() == BasicType::Int || def.a()->array_length() != IRArrayLength::IR_NONE_ARRAY) {
                        if(r_count < ARGUMENT_REGISTER_COUNT) {
                            IRInstr&& instr = IRInstr::create_assign(def.a(), m_argreg[r_count]);
                            fparam_list.push_front(instr);
                            r_count++;
                        }
                        else {
                            IRInstr&& instr = IRInstr::create_load(def.a(), m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S+2+2*m_count]);
                            fparam_list.push_front(instr);
                            m_count++;
                        }
                    }
                    else {
                        if(s_count < ARGUMENT_REGISTER_COUNT_S) {
                            IRInstr&& instr = IRInstr::create_assign(def.a(), m_argreg[ARGUMENT_REGISTER_COUNT+s_count]);
                            fparam_list.push_front(instr);
                            s_count++;
                        }
                        else {
                            IRInstr&& instr = IRInstr::create_load(def.a(), m_argreg[ARGUMENT_REGISTER_COUNT+ARGUMENT_REGISTER_COUNT_S+2+2*m_count]);
                            fparam_list.push_front(instr);
                            m_count++;
                        }
                    }

                }
            }
            for(auto& instr: fparam_list)
                new_block->add_instr_to_front(instr);

            // 第一次遍历，确定函数用到的全局变量，并在开头将全局变量的地址load进来
            scan_global(unit, global_in_func);
            for (auto iter = global_in_func.begin(); iter != global_in_func.end(); ++iter) {
                auto temp = m_ir_sym_table->create_temp(BasicType::Int, ++max_temp_index, IRArrayLength::IR_ARRAY_POINTER);
                IRInstr&& instr = IRInstr::create_assign(temp, *iter);
                global_addr.emplace(*iter, temp);
                new_block->add_instr(instr);
            }

            gen_block(unit, func, global_addr);
            entry->delete_edge(0);
            entry->set_edge(0, new_block);
            new_block->set_edge(0, first);
            IRInstr&& instr = IRInstr::create_block_goto();
            new_block->add_instr(instr);
        }
    }
}

IRProgram* LowerIRManager::gen_lower_ir()
{
    bind_argreg();
    bind_runtime_functions();
    revise_instr();
    return m_cfg;
}