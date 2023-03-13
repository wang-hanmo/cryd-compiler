#include <dead_code_elimination.h>
#include <string>
#include <iostream>
#include <cassert>
#include <map>
bool DeadCodeElimination::is_ssa_var(IRSymbol* sym)
{
    return ((sym->kind() == IRSymbolKind::Param) ||
            (sym->kind() == IRSymbolKind::Local) ||
            sym->kind() == IRSymbolKind::Temp);
}
//含有SSA形式左值（非全局变量、数组）
bool DeadCodeElimination::has_lvalue(const IRInstr& instr)
{
    if (instr.type() == IRType::Assign || instr.type() == IRType::ArrayLoad || instr.type() == IRType::PhiFunc
        || instr.type() == IRType::BinaryCalc || instr.type() == IRType::UnaryCalc || instr.type() == IRType::CallWithRet) {
        return is_ssa_var(instr.r());
    }
    return false;
}
std::vector<IRSymbol*> DeadCodeElimination::rvalue_list(const IRInstr& instr) {  //取出所有SSA形式的右值
    std::vector<IRSymbol*> res;
    bool is_rvalue_a = false;
    bool is_rvalue_b = false;
    bool is_rvalue_r = false;
    switch (instr.type()) {
    case IRType::BinaryCalc:
        is_rvalue_a = true;
        is_rvalue_b = true;
        break;
    case IRType::UnaryCalc:
        is_rvalue_a = true;
        break;
    case IRType::Assign:
        is_rvalue_a = true;
        break;
    case IRType::RParam:
        is_rvalue_a = true;
        break;
    case IRType::ArrayStore:
        is_rvalue_r = true;
        is_rvalue_a = true;
        is_rvalue_b = true;
        break;
    case IRType::ArrayLoad:
        is_rvalue_a = true;
        is_rvalue_b = true;
        break;
    case IRType::Call:
        break;
    case IRType::CallWithRet:
        break;
    case IRType::Return:
        break;
    case IRType::ValReturn:
        is_rvalue_a = true;
        break;
    case IRType::BlockGoto:
        break;
    case IRType::BlockCondGoto:
        is_rvalue_a = true;
        break;
    case IRType::PhiFunc:
        for (const auto& param : instr.a()->phi_params())
            res.push_back(param.sym);
        break;
    default:
        break;
    }
    if (is_rvalue_r && is_ssa_var(instr.r()))
        res.push_back(instr.r());
    if (is_rvalue_a && is_ssa_var(instr.a()))
        res.push_back(instr.a());
    if (is_rvalue_b && is_ssa_var(instr.b()))
        res.push_back(instr.b());
    return res;
}
bool DeadCodeElimination::is_initially_alive(const IRInstr& instr) {    //是否在最初就为活跃指令
    bool alive = false;
    switch (instr.type()) {
    case IRType::BinaryCalc:
        if (instr.r()->kind() == IRSymbolKind::Global)
            alive = true;
        break;
    case IRType::UnaryCalc:
        if (instr.r()->kind() == IRSymbolKind::Global)
            alive = true;
        break;
    case IRType::Assign:
        if (instr.r()->kind() == IRSymbolKind::Global)
            alive = true;
        break;
    case IRType::ArrayLoad:
        alive = false;
        break;
    case IRType::PhiFunc:
        alive = false;
        break;
    case IRType::CallWithRet:
        alive = false;
        break;
    default:
        alive = true;
        break;
    }
    return alive;
}
//1-1.bfs标记不可达基本块
void DeadCodeElimination::dead_block_eliminate_mark(IRBlock* entry)
{
    //bfs判断死边
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(entry);
    entry->set_tag(0b01);
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (int k = 1; k >= 0; --k) {
            auto next = now->get_succ(k);
            if (next == nullptr)continue;
            int alive = 1;//是否存在一条经过本block到达子block的活跃路径
            //如果本block到子block的边为死,则无法使子block活跃
            if (now->out_degree() == 2) {
                assert(!now->get_instr_list_const().empty());
                auto cond = now->get_instr_list_const().crbegin()->a();
                if (cond->kind() == IRSymbolKind::Value && ((cond->value().int_value == 0) != (k == 0)))
                    alive = 0;
            }
            //如果本block为死block，则无法使子block活跃
            if (now->get_tag() == 0)
                alive = 0;
            //没有遍历过子block，则加入队列
            if (visited.find(next) == visited.end()) {
                q.push(next);
                next->set_tag(alive);
                visited.insert(next);
            } else if(next->get_tag()==0 && alive==1) {//虽然遍历过子block，但本block能使原先不活跃的子block活跃起来，也加入队列
                q.push(next);
                next->set_tag(alive);
            }
        }
    }
}
//1-2.dfs删除不可达基本块
void DeadCodeElimination::dead_block_eliminate_delete(IRBlock* block)
{
    //保证复杂度为线性
    if ((block->get_tag() & 0b10) == 0) {
        block->set_tag(block->get_tag()|0b10);
    }else return;
    //递归删除子节点
    for (int k = 1; k >= 0; --k) {
        auto next = block->get_succ(k);
        if (next == nullptr)continue;
        dead_block_eliminate_delete(next);
        int alive = 1;
        if (block->out_degree() == 2) {//如果本block到子block的边为死，则删除边
            assert(!block->get_instr_list_const().empty());
            auto cond = block->get_instr_list_const().crbegin()->a();
            if (cond->kind() == IRSymbolKind::Value && ((cond->value().int_value == 0) != (k == 0)))
                alive = 0;
        }
        //如果本block为死block，则删除边
        if (block->get_tag()==0b10)
            alive = 0;
        //如果无法经过本block到达子block，就删除边
        if (alive == 0) {
            int out_degree = block->out_degree();
            block->delete_edge(k);
            if (out_degree == 2) {
                //最后一条语句一定是条件分支语句，顺带将最后的BlockCondGoto指令改为BlockGoto
                assert(!block->get_instr_list_const().empty());
                auto& cond_instr = *block->get_instr_list().rbegin();
                cond_instr.rewrite_type(IRType::BlockGoto);
                //将true条件改为uncond条件
                if (k == 0)
                    block->adjust_edge_to_uncond();
            }
            //如果子节点已经没有入度，说明死块连通子图已经全部删除，delete掉它，防止内存泄漏。
            if (next->in_degree() == 0)
                delete next;
        }
    }
}
//1.删除不可达基本块
void DeadCodeElimination::dead_block_eliminate(IRUnit& unit)
{
    auto entry = unit.get_entry();
    //第一遍遍历，将用block的tag表示其是否是活块
    //时间复杂度为O(BlogB)
    dead_block_eliminate_mark(entry);
    //第二遍遍历，dfs删除死块
    //tag的第二位为1表示已经遍历过，防止重复遍历导致复杂度退化。
    //时间复杂度为O(BlogB)，(删除一个节点时，维护正反边的复杂度为logB)
    dead_block_eliminate_delete(entry);
}
//2 初始化符号的编号，维护符号到定值指令的映射，向工作队列添加活跃指令。
void DeadCodeElimination::initialize_index(IRUnit& unit) {
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit.get_entry());
    //添加所有定义语句中的符号
    for (auto& instr : unit.get_definations()) {
        if (is_ssa_var(instr.a())) {
            instr.set_no(0);//0表示非活跃指令
            //维护SSA形式符号。
            instr.a()->set_tag(m_sym_count);            //维护符号编号
            instr.a()->def_sym()->set_tag(m_sym_count); //给原形式的符号也赋予同样的编号
            m_sym_to_def_instr.push_back(&instr);      //维护符号到定值指令的映射
            m_sym_count++;
        }else instr.set_no(1);
    }
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list()) {
            instr.set_no(0);//0表示非活跃指令
            if (has_lvalue(instr)) {
                instr.r()->set_tag(m_sym_count);        //维护符号编号
                m_sym_to_def_instr.push_back(&instr);   //维护符号到定值指令的映射
                //std::cout << instr.r() << "\tid=" << m_sym_count << " def no="<<instr.no() << std::endl;
                m_sym_count++;
            }
            if (is_initially_alive(instr)) {
                instr.set_no(1);//1表示活跃指令
                m_working_queue.push(&instr);
            }
        }
        for (int k = 1; k >= 0; --k)
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
    }
}
//4.死代码删除
void DeadCodeElimination::dead_instr_elimination(IRUnit& unit) {
    //删除定义区死代码
    for (auto iter = unit.get_definations().begin(); iter != unit.get_definations().end();) {
        //删除条件：是SSA形式变量（排除数组），且为局部变量，且非活跃
        if (iter->a()->ssa_index()>-1 && iter->no() == 0 && iter->type()==IRType::LocalDecl)
            iter = unit.get_definations().erase(iter);
        else ++iter;
    }
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto iter = now->get_instr_list().begin(); iter != now->get_instr_list().end();){
            if (iter->no() == 0) {
                //非活跃的call with ret指令退化为忽略返回值的call
                if (iter->type() == IRType::CallWithRet) {
                    iter->rewrite_type(IRType::Call);
                    ++iter;
                }else iter = now->get_instr_list().erase(iter);
            }
            else ++iter;
        }
        for (int k = 1; k >= 0; --k)
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
    }
}
/*
static std::ostream& operator<<(std::ostream& os, IRSymbol* ir_sym)
{
    os << ir_sym->basic_type() << " ";
    switch (ir_sym->kind()) {
    case IRSymbolKind::Global:
        return os << "@" << ir_sym->global_sym()->name();
    case IRSymbolKind::Param:
    case IRSymbolKind::Local:
        if (ir_sym->ssa_index() != -1)
            return os << "l" << ir_sym->index() << "_" << ir_sym->ssa_index();
        else return os << "l" << ir_sym->index();
    case IRSymbolKind::Temp:
        return os << "t" << ir_sym->index();
    case IRSymbolKind::Value: {
        if (ir_sym->basic_type() == BasicType::Float)
            return os << ir_sym->value().float_value;
        return os << ir_sym->value().int_value;
    }
    case IRSymbolKind::PhiFunc: {
        os << "phi(";
        bool first = true;
        for (auto param : ir_sym->phi_params()) {
            if (first)first = false;
            else os << ", ";
            os << param.sym;
        }
        return os << ")";
    }
    default:
        break;
    }
    return os << "[Error Oprand]";
}*/
//3.活跃指令分析
void DeadCodeElimination::active_instruction_analysis() {
    while (!m_working_queue.empty()) {
        IRInstr* now = m_working_queue.front();
        m_working_queue.pop();
        now->set_no(1);//1表示活跃指令
        //对原始定义指令维护其活跃性
        if (has_lvalue(*now)&&(now->r()->kind()==IRSymbolKind::Local|| now->r()->kind() == IRSymbolKind::Param)){
            m_sym_to_def_instr[now->r()->def_sym()->get_tag()]->set_no(1);
        }
        auto rv_list = rvalue_list(*now);
        for (auto sym : rv_list) {
            /*if (sym->get_tag() >= m_sym_to_def_instr.size()) {
               std:: cout << sym << std::endl;
            }*/
            auto tar_instr = m_sym_to_def_instr[sym->get_tag()];
            if (tar_instr->no() == 0)
                m_working_queue.push(tar_instr);
        }
    }
}
//4.全局变量相关活跃指令分析
void DeadCodeElimination::global_active_instruction_analysis(IRUnit& unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        int iteration_count = 0;
        global_active_instruction_analysis_block(now,unit.get_tag());
        for (int k = 1; k >= 0; --k)
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
    }
}
//4-1.基本块内的全局变量相关活跃指令分析。返回值为是否标注了新的死代码
void DeadCodeElimination::global_active_instruction_analysis_block(IRBlock* block,int unit_tag)
{
    //定值需求表
    BitMap global_var_def_demand(m_idfa->get_global_var_count());
    //基本块出口包含对所有全局变量值的需求
    global_var_def_demand.set_all();
    //逆序访问所有指令，遇到一个对全局变量的使用，就记录对其定值的需求。
    //遇到一个对全局变量的定值，如果其从未被需求过，则是一个无用的定值
    for (auto instr = block->get_instr_list().rbegin(); instr != block->get_instr_list().rend();++instr) {
        //全局变量定值
        if (instr->type()==IRType::Assign && instr->r()->kind() == IRSymbolKind::Global) {
            //如果存在一个定值，但又不会使用这个值，则标记为死代码
            if (!global_var_def_demand.test(instr->r()->get_tag())) {
                instr->set_no(0);
            } else {//否则记录需求不存在
                global_var_def_demand.reset(instr->r()->get_tag());
            }
        }
        //全局变量使用
        if (instr->a() != nullptr && instr->a()->kind() == IRSymbolKind::Global && instr->a()->is_non_array() &&
            instr->a()->global_sym()->get_kind() == VarKind::Global) {
            global_var_def_demand.set(instr->a()->get_tag());
        }
        if (instr->b() != nullptr && instr->b()->kind() == IRSymbolKind::Global && instr->b()->is_non_array() &&
            instr->b()->global_sym()->get_kind() == VarKind::Global) {
            global_var_def_demand.set(instr->b()->get_tag());
        }
        if (instr->type() == IRType::Call || instr->type() == IRType::CallWithRet) {
            for (int i = 0; i < m_idfa->get_global_var_count(); ++i) {
                //如果函数可能使用到某变量，那么记录对其的需求
                if (m_idfa->get_global_var_use(unit_tag).test(i))
                    global_var_def_demand.set(i);
            }
        }
    }
}
//顶层
void DeadCodeElimination::work_unit(IRUnit& unit)
{
    m_sym_count = 0;
    m_sym_to_def_instr.clear();
    while (!m_working_queue.empty())
        m_working_queue.pop();
    //1、删除不可达的基本块
    dead_block_eliminate(unit);
    //2、初始化符号的编号，维护符号到定值指令的映射，向工作队列添加活跃指令。
    initialize_index(unit);
    //3、活跃指令分析
    active_instruction_analysis();
    //4、基本块内的全局变量赋值活跃指令分析
    if (m_idfa->global_symbol_optimization_is_valid()) {
        global_active_instruction_analysis(unit);
    }
    //5、死代码删除
    dead_instr_elimination(unit);
}
void DeadCodeElimination::run()
{
    std::cout << "Running pass: Dead Code Elimination" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_ir_sym_table == nullptr) {
        Optimizer::optimizer_error("No IR symbol table specified");
    }
    if (m_idfa->global_symbol_optimization_is_valid())
        m_idfa->compute_side_effect_info();
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef) {
            //改变控制流图后支配树信息失效
            unit.set_dominator_tree_info_valid(false);
            work_unit(unit);
        }
}