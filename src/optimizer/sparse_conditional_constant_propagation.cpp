#include <sparse_conditional_constant_propagation.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
using namespace sccp;
//格的二元运算
ConstLat ConstLat::lat_eval_binaryop(const IROper op, const ConstLat& lhs, const ConstLat& rhs)
{
    ConstLat res;
    if (lhs.lat_value == ConstLatValue::Var || rhs.lat_value == ConstLatValue::Var) {
        res.lat_value = ConstLatValue::Var;
        return res;
    }
    if (lhs.lat_value == ConstLatValue::Const && rhs.lat_value == ConstLatValue::Const) {
        res.lat_value = ConstLatValue::Const;
        switch (op) {
        case IROper::AddI:          res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   + rhs.int_value;     break;
        case IROper::AddF:          res.basic_type = BasicType::Float; res.float_value = lhs.float_value + rhs.float_value;   break;
        case IROper::SubI:          res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   - rhs.int_value;     break;
        case IROper::SubF:          res.basic_type = BasicType::Float; res.float_value = lhs.float_value - rhs.float_value;   break;
        case IROper::MulI:          res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   * rhs.int_value;     break;
        case IROper::MulF:          res.basic_type = BasicType::Float; res.float_value = lhs.float_value * rhs.float_value;   break;
        case IROper::DivI:          res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   / rhs.int_value;     break;
        case IROper::DivF:          res.basic_type = BasicType::Float; res.float_value = lhs.float_value / rhs.float_value;   break;
        case IROper::ModI:          res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   % rhs.int_value;     break;
        case IROper::EqualI:        res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   == rhs.int_value ? 1 : 0;     break;
        case IROper::EqualF:        res.basic_type = BasicType::Int;   res.int_value   = lhs.float_value == rhs.float_value ? 1 : 0;   break;
        case IROper::NotEqualI:     res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   != rhs.int_value ? 1 : 0;     break;
        case IROper::NotEqualF:     res.basic_type = BasicType::Int;   res.int_value   = lhs.float_value != rhs.float_value ? 1 : 0;   break;
        case IROper::GreaterI:      res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   >  rhs.int_value ? 1 : 0;     break;
        case IROper::GreaterF:      res.basic_type = BasicType::Int;   res.int_value   = lhs.float_value >  rhs.float_value ? 1 : 0;   break;
        case IROper::GreaterEqualI: res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   >= rhs.int_value ? 1 : 0;     break;
        case IROper::GreaterEqualF: res.basic_type = BasicType::Int;   res.int_value   = lhs.float_value >= rhs.float_value ? 1 : 0;   break;
        case IROper::LessI:         res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   <  rhs.int_value ? 1 : 0;     break;
        case IROper::LessF:         res.basic_type = BasicType::Int;   res.int_value   = lhs.float_value <  rhs.float_value ? 1 : 0;   break;
        case IROper::LessEqualI:    res.basic_type = BasicType::Int;   res.int_value   = lhs.int_value   <= rhs.int_value ? 1 : 0;     break;
        case IROper::LessEqualF:    res.basic_type = BasicType::Int;   res.int_value   = lhs.float_value <= rhs.float_value ? 1 : 0;   break;
        default:
            assert(0);
            Optimizer::optimizer_error("Unexpected type of instruction");
        }
    }
    return res;
}
//格的一元运算
ConstLat ConstLat::lat_eval_unaryop(const IROper op, const ConstLat& oprand)
{
    ConstLat res;
    if (oprand.lat_value == ConstLatValue::Var) {
        res.lat_value = ConstLatValue::Var;
        return res;
    }
    if (oprand.lat_value == ConstLatValue::Const) {
        res.lat_value = ConstLatValue::Const;
        switch (op) {
        case IROper::FToI:res.basic_type = BasicType::Int;    res.int_value = (int)oprand.float_value;   break;
        case IROper::IToF:res.basic_type = BasicType::Float;  res.float_value = (float)oprand.int_value; break;
        case IROper::NotI:res.basic_type = BasicType::Int;    res.int_value = (oprand.int_value == 0) ? 1 : 0;   break;
        case IROper::NotF:res.basic_type = BasicType::Int;    res.int_value = (oprand.float_value == 0.0) ? 1 : 0;   break;
        case IROper::NegI:res.basic_type = BasicType::Int;    res.int_value = -oprand.int_value;   break;
        case IROper::NegF:res.basic_type = BasicType::Float;  res.float_value = -oprand.float_value;   break;
        default:
            assert(0);
            Optimizer::optimizer_error("Unexpected type of instruction");
        }
    }
    return res;
}
//格运算，交
void ConstLat::intersect_with(const ConstLat& rhs)
{
    //自身的级别比rhs更靠近顶部就降格
    if ((int)lat_value < (int)rhs.lat_value) {
        lat_value = rhs.lat_value;
        if (basic_type == BasicType::Int)
            int_value = rhs.int_value;
        else if (basic_type == BasicType::Float)
            float_value = rhs.float_value;
    }
    else if ((int)lat_value == (int)rhs.lat_value) {//级别相同，则判断常数是否不同,如果不同则降格到变量
        if (lat_value == ConstLatValue::Const) {
            if (basic_type == BasicType::Int && int_value != rhs.int_value) {
                lat_value = ConstLatValue::Var;
            }
            else if (basic_type == BasicType::Float && float_value != rhs.float_value) {
                lat_value = ConstLatValue::Var;
            }
        }
    }
}
bool SparseConditionalConstantPropagation::is_ssa_var(IRSymbol* sym)
{
    return (sym->kind() == IRSymbolKind::Param||
            sym->kind() == IRSymbolKind::Local||
             sym->kind() == IRSymbolKind::Temp);
}
//含有SSA形式左值（非全局变量、数组）
bool SparseConditionalConstantPropagation::has_lvalue(const IRInstr& instr)
{
    if (instr.type() == IRType::Assign || instr.type() == IRType::ArrayLoad || instr.type() == IRType::PhiFunc
        || instr.type() == IRType::BinaryCalc || instr.type() == IRType::UnaryCalc || instr.type() == IRType::CallWithRet) {
        return is_ssa_var(instr.r());
    }
    return false;
}
//获取SSA形式的右值列表
std::vector<IRSymbol*> SparseConditionalConstantPropagation::rvalue_list(const IRInstr& instr)
{
    std::vector<IRSymbol*> res;
    bool is_rvalue_a = false;
    bool is_rvalue_b = false;
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
        is_rvalue_a = true;
        is_rvalue_b = true;
        break;
    case IRType::ArrayLoad:
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
        for (const auto& param : instr.a()->phi_params()) {
            res.push_back(param.sym);
        }
        break;
    default:
        assert(0);
        Optimizer::optimizer_error("Unexpected instruction!");
        break;
    }
    if (is_rvalue_a && is_ssa_var(instr.a()))
        res.push_back(instr.a());
    if (is_rvalue_b && is_ssa_var(instr.b()))
        res.push_back(instr.b());
    return res;
}
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
        for (const auto& param : ir_sym->phi_params()) {
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
}
//1 初始化指令、符号的编号，并维护符号到定值指令的映射。
void SparseConditionalConstantPropagation::initialize_instr_index(IRUnit& unit) {
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit.get_entry());
    //添加虚拟entry bb
    m_instr_count++;
    m_i_instr.push_back(nullptr);
    m_instr_to_block.push_back(nullptr);//维护指令编号到基本块的映射
    //添加所有定义语句中的符号
    for (auto& instr : unit.get_definations()) {
        if (is_ssa_var(instr.a())) {
            instr.a()->set_tag(m_sym_count);            //维护符号编号
            m_sym_to_def_instr.push_back(instr.no());   //维护指令编号，用于占位，该值不会被使用
            //std::cout << instr.r() << "\tid=" << m_sym_count << " def no="<<instr.no() << std::endl;
            m_sym_count++;
        }
    }
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list()) {
            instr.set_no(m_instr_count++);//初始化指令编号
            m_i_instr.push_back(&instr);  //维护编号到指令的映射
            m_instr_to_block.push_back(now);//维护指令编号到基本块的映射
            if(has_lvalue(instr)){
                instr.r()->set_tag(m_sym_count);            //维护符号编号
                m_sym_to_def_instr.push_back(instr.no());   //维护指令编号
               // std::cout << instr.r() << "\tid=" << m_sym_count << " def no="<<instr.no() << std::endl;
                m_sym_count++;
            }
        }
        for (int k = 1; k >= 0; --k)
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
    }
    //添加虚拟exit bb
    m_instr_count++;
    m_i_instr.push_back(nullptr);
    m_instr_to_block.push_back(nullptr);//维护指令编号到基本块的映射
}
//2 构建最小流图结构（每条指令一个BB)，CFG、SSA工作边表，lat cell的初始值。
void SparseConditionalConstantPropagation::initialize_working_environment(IRUnit& unit)
{
    //初始化SSA边表、(虚拟)CFG边表数组
    m_ssa_edge_list.resize(m_instr_count);
    m_cfg_edge_list.resize(m_instr_count);
    m_anti_cfg_edge_list.resize(m_instr_count);
    //初始化格值数组
    m_lat_cell.resize(m_sym_count);
    //添加entry到第一条指令的边
    m_cfg_edge_list[0].push_back(InstrEdge(0,1));
    m_anti_cfg_edge_list[1].push_back(&(m_cfg_edge_list[0].back()));
    //初始化定义语句的格值为变量，因为这些变量还未在当前函数内赋过值，可能是外部传递的参数
    for (auto& instr : unit.get_definations()) {
        if(is_ssa_var(instr.a()))
            m_lat_cell[instr.a()->get_tag()] = ConstLat(ConstLatValue::Var, instr.a()->basic_type());
    }
    std::set<IRBlock*> visited;
    std::queue<IRBlock*> q;
    q.push(unit.get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        initialize_block(now);
        for (int k = 1; k >= 0; --k)
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
    }
    //初始化CFG工作边表,加入所有以entry为起点的流图边（只有一条）
    for(auto& edge_from_entry:m_cfg_edge_list[0])
        m_cfg_work_list.push(&edge_from_entry);
    //exsc flag已经在InstrEdge的构造函数中隐式初始化了
}
//2-1-1 获取指定block中第一条指令的编号（如果为空，继续从后续block里找）
int SparseConditionalConstantPropagation::get_next_instr_index(IRBlock* block){        
    int tar_index = -1;
    if (block->is_exit())
        tar_index = m_instr_count - 1;
    else {
        //找到下一条指令，如果节点为空就一直顺着找，直到找到exit block
        while (!block->is_exit() && block->get_instr_list_const().empty())
            block = block->get_succ(0);
        if (block->get_instr_list_const().empty())
            tar_index = m_instr_count - 1;
        else
            tar_index = block->get_instr_list_const().cbegin()->no();
    }
    return tar_index;
}
//2-1 basic block级的初始化,index表示指令编号
void SparseConditionalConstantPropagation::initialize_block(IRBlock* block)
{
    auto& instr_list = block->get_instr_list();
    for (auto iter = instr_list.begin(); iter != instr_list.end();++iter) {
        auto next_instr = iter;
        next_instr++;
        //构建最小流图边及其反向边
        if (iter->type() == IRType::BlockCondGoto) {
            int true_index = get_next_instr_index(block->get_succ(1));
            int false_index = get_next_instr_index(block->get_succ(0));
            m_cfg_edge_list[iter->no()].push_back(InstrEdge(iter->no(), false_index, false));
            m_anti_cfg_edge_list[false_index].push_back(&(m_cfg_edge_list[iter->no()].back()));
            m_cfg_edge_list[iter->no()].push_back(InstrEdge(iter->no(), true_index, true));
            m_anti_cfg_edge_list[true_index].push_back(&(m_cfg_edge_list[iter->no()].back()));
        }else if (iter->type() == IRType::BlockGoto||next_instr==instr_list.end()) {
            int uncond_tar_index = get_next_instr_index(block->get_succ(0));
            m_cfg_edge_list[iter->no()].push_back(InstrEdge(iter->no(), uncond_tar_index, false));
            m_anti_cfg_edge_list[uncond_tar_index].push_back(&(m_cfg_edge_list[iter->no()].back()));
        }else {
            m_cfg_edge_list[iter->no()].push_back(InstrEdge(iter->no(),next_instr->no()));
            m_anti_cfg_edge_list[next_instr->no()].push_back(&(m_cfg_edge_list[iter->no()].back()));
        }
        //构建SSA边
        for (auto rvalue : rvalue_list(*iter)) {
            //对于index=0的初值符号，不加入SSA边，否则加入ssa边
            if (rvalue->kind() != IRSymbolKind::Temp && rvalue->ssa_index() == 0)
                continue;
            m_ssa_edge_list[m_sym_to_def_instr[rvalue->get_tag()]].push_back
                (InstrEdge(m_sym_to_def_instr[rvalue->get_tag()], iter->no()));
        }
        //初始化格值为未确定
        if (has_lvalue(*iter)) {
            m_lat_cell[iter->r()->get_tag()] = ConstLat(ConstLatValue::Uncertain,iter->r()->basic_type());
        }
    }
}
void SparseConditionalConstantPropagation::visit_phi(IRInstr* instr)
{
    assert(instr->type() == IRType::PhiFunc);
    int sym_index= instr->r()->get_tag();
    auto val = m_lat_cell[sym_index];
    auto& first_instr = m_instr_to_block[instr->no()]->get_instr_list_const().front();
    //对每个活参数执行交运算
    for (auto& param : instr->a()->phi_params()) {
        bool is_executed_edge = false;
        if (param.from->is_entry()) {
            is_executed_edge = true;
        } else {
            for (const auto& edge : m_cfg_edge_list[param.from->get_instr_list_const().back().no()]) {
                if (edge.to == first_instr.no() && edge.exsc_flag) {
                    is_executed_edge = true;
                }
            }
        }
        if(is_executed_edge)
            m_lat_cell[sym_index].intersect_with(m_lat_cell[param.sym->get_tag()]);
    }
    //如果格值变化，加入SSA边
    if (m_lat_cell[sym_index] != val) {
        for (auto& edge : m_ssa_edge_list[instr->no()]) {
            m_ssa_work_list.push(&edge);
            //std::cout << "add edge " << edge.from << " " << edge.to << std::endl;
        }
    }
}
void SparseConditionalConstantPropagation::visit_instr(IRInstr* instr)
{
    //常量传播，更新格值
    auto val = lat_eval(instr);
    //如果格值变化，顺着SSA边传播
    if (has_lvalue(*instr) && val != m_lat_cell[instr->r()->get_tag()]) {
        m_lat_cell[instr->r()->get_tag()].intersect_with(val);
        for (auto& edge : m_ssa_edge_list[instr->no()]) {
            m_ssa_work_list.push(&edge);
            //std::cout << "add edge " << edge.from << " " << edge.to << std::endl;
        }
    }
    //加入流图边
    if (instr->type() == IRType::BlockCondGoto) {
        if (val.lat_value == ConstLatValue::Uncertain) {
            assert(false);
        }else if (val.lat_value == ConstLatValue::Const) {
            for (auto& edge : m_cfg_edge_list[instr->no()]) {
                if (val.int_value == 0 && !edge.is_true_edge)
                    m_cfg_work_list.push(&edge);
                else if (val.int_value != 0 && edge.is_true_edge)
                    m_cfg_work_list.push(&edge);
            }
        } else {
            for (auto& edge : m_cfg_edge_list[instr->no()])
                m_cfg_work_list.push(&edge);
        }
    }
}
//获取操作数的格值
ConstLat SparseConditionalConstantPropagation::get_lat_val(IRSymbol* sym)
{
    if (is_ssa_var(sym)) {
        return m_lat_cell[sym->get_tag()];
    } else if (sym->kind() == IRSymbolKind::Global) {
        return ConstLat(ConstLatValue::Var,sym->basic_type());
    } else if (sym->kind() == IRSymbolKind::Value) {
        auto res = ConstLat(ConstLatValue::Const, sym->basic_type());
        if (sym->basic_type() == BasicType::Int)
            res.int_value = sym->value().int_value;
        else if(sym->basic_type() == BasicType::Float)
            res.float_value= sym->value().float_value;
        return res;
    }else if(sym->array_length()==IRArrayLength::IR_ARRAY_POINTER||sym->array_length() > 0)
        return ConstLat(ConstLatValue::Var, sym->basic_type());
    assert(false);
    return ConstLat();
}
//使用格值执行指令，计算结果
ConstLat SparseConditionalConstantPropagation::lat_eval(IRInstr* instr)
{
    ConstLat res;
    ConstLat a;
    ConstLat b;
    switch (instr->type()) {
    case IRType::BinaryCalc:
        a = get_lat_val(instr->a());
        b = get_lat_val(instr->b());
        res = ConstLat::lat_eval_binaryop(instr->op(), a, b);
        break;
    case IRType::UnaryCalc:
        a = get_lat_val(instr->a());
        res = ConstLat::lat_eval_unaryop(instr->op(), a);
        break;
    case IRType::Assign:
        a = get_lat_val(instr->a());
        res = a;
        break;
    case IRType::RParam:
        break;
    case IRType::ArrayStore:
        break;
    case IRType::ArrayLoad:
        res = ConstLat(ConstLatValue::Var,instr->r()->basic_type());
        break;
    case IRType::Call:
        break;
    case IRType::CallWithRet:
        res = ConstLat(ConstLatValue::Var, instr->r()->basic_type());
        break;
    case IRType::Return:
        break;
    case IRType::ValReturn:
        break;
    case IRType::BlockGoto:
        break;
    case IRType::BlockCondGoto:
        res = get_lat_val(instr->a());
        break;
    default:
        assert(0);
        Optimizer::optimizer_error("Unexpected instruction!");
        break;
    }
    return res;
}

static std::ostream& operator<<(std::ostream& os, const ConstLat& lat_cell)
{
    if (lat_cell.lat_value == ConstLatValue::Uncertain) {
        os << "Top";
    }
    else if (lat_cell.lat_value == ConstLatValue::Const) {
        if(lat_cell.basic_type==BasicType::Int)
            os << "Const " << lat_cell.int_value;
        else if (lat_cell.basic_type == BasicType::Float)
            os << "Const " << lat_cell.float_value;
    }
    else {
        os << "Bottom";
    }
    return os;
}
//顶层-对IRUnit执行常量传播-新版，复杂度更高但是效果更好
void SparseConditionalConstantPropagation::work_unit(IRUnit& unit)
{
    m_instr_count = 0;
    m_sym_count = 0;
    m_ssa_edge_list.clear();
    m_cfg_edge_list.clear();
    m_anti_cfg_edge_list.clear();
    m_sym_to_def_instr.clear();
    m_i_instr.clear();
    m_lat_cell.clear();
    m_instr_to_block.clear();
    while(!m_cfg_work_list.empty())
        m_cfg_work_list.pop();
    while (!m_ssa_work_list.empty())
        m_ssa_work_list.pop();
    //初始化工作
    initialize_instr_index(unit);
    initialize_working_environment(unit);
    //常量传播
    while (!m_cfg_work_list.empty() || !m_ssa_work_list.empty()){
        //在流图边上的常量传播-复杂度更高但效果更好的版本
        if (!m_cfg_work_list.empty()) {
            auto edge = m_cfg_work_list.front();
            m_cfg_work_list.pop();
            if (edge->to == m_instr_count - 1)continue;
            if (true) {
                //std::cout << edge->to << std::endl;
                auto to = m_i_instr[edge->to];
                //无视exsc_flag必须为false的判断，强制沿着流图向下走，保证新的可执行边信息更新到每一个phi函数上
                if (to->type() == IRType::PhiFunc) {
                    edge->exsc_flag = true;
                    visit_phi(to);
                    //如果只有一条流图上的出边，则将出边加入队列，不论之前是否分析过。保证phi函数全部都能被更新到
                    assert(m_cfg_edge_list[edge->to].size() == 1);
                    m_cfg_work_list.push(&(m_cfg_edge_list[edge->to].front()));
                } else if (!edge->exsc_flag) {//对于其他指令，只允许沿着流图更新一次
                    edge->exsc_flag = true;
                    if (edge_count(edge->to) == 1) {
                        //std::cout << has_lvalue(*to) << std::endl;
                        visit_instr(to);
                        //如果只有一条流图上的出边，则将出边加入队列,保证分析正常进行下去
                        if (to->type() != IRType::BlockCondGoto) {
                            assert(m_cfg_edge_list[edge->to].size() == 1);
                            m_cfg_work_list.push(&(m_cfg_edge_list[edge->to].front()));
                        }
                    }
                }
            }
        }
        /*在流图边上的常量传播-复杂度为线性的版本
        if (!m_cfg_work_list.empty()) {
            auto edge = m_cfg_work_list.front();
            m_cfg_work_list.pop();
            if (edge->to == m_instr_count - 1)continue;     
            if (!edge->exsc_flag) {
                //std::cout << edge->to << std::endl;
                edge->exsc_flag = true;
                auto to = m_i_instr[edge->to];
                if (to->type() == IRType::PhiFunc) {
                    visit_phi(to);
                } else if (edge_count(edge->to) == 1) {
                    edge->exsc_flag = true;
                    //std::cout << has_lvalue(*to) << std::endl;
                    visit_instr(to);
                }
                //如果只有一条流图上的出边，则将出边加入队列,保证分析正常进行下去
                if (to->type() != IRType::BlockCondGoto) {
                    assert(m_cfg_edge_list[edge->to].size() == 1);
                    m_cfg_work_list.push(&(m_cfg_edge_list[edge->to].front()));
                }
            }
        }*/
        //在SSA边上的常量传播
        if (!m_ssa_work_list.empty()) {    
            auto edge = m_ssa_work_list.front();
            m_ssa_work_list.pop();
            if (edge->to == m_instr_count - 1)continue;
            auto to = m_i_instr[edge->to];
            if (to->type() == IRType::PhiFunc) {
                //std::cout<<edge->from<<" " << edge->to << std::endl;
                visit_phi(to);
            }else if (edge_count(edge->to) >= 1) {
                //std::cout << edge->from<< " " << edge->to << std::endl;
                visit_instr(to);
            }
        }
    }
#ifdef DEBUG_INFO_OPTIMIZER
    //输出结果,跳过参数和局部变量的初值符号
    for (int i = unit.get_definations_const().size()-1; i < m_sym_count; ++i) {
        auto r=m_i_instr[m_sym_to_def_instr[i]]->r();
        std::cout << r << "\t\t" << m_lat_cell[i] << std::endl;
    }
    std::cout << "-------------------" << std::endl;
#endif
    //指令重写
    rewrite_instructions(unit);
}
//指令重写
void SparseConditionalConstantPropagation::rewrite_instructions(IRUnit& unit)
{
    for (int i = 0; i < m_instr_count; ++i) {
        auto instr = m_i_instr[i];
        if (instr == nullptr)continue;
        //分类型对指令进行重写
        switch (instr->type()) {
        case IRType::BinaryCalc:
            if (is_ssa_var(instr->r()) && m_lat_cell[instr->r()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->r()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(instr->r()->basic_type(),lat_cell.basic_value()));
                instr->rewrite_type(IRType::Assign);
            } else {
                if (is_ssa_var(instr->a()) && m_lat_cell[instr->a()->get_tag()].lat_value == ConstLatValue::Const) {
                    const auto& lat_cell = m_lat_cell[instr->a()->get_tag()];
                    instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
                }
                if (is_ssa_var(instr->b()) && m_lat_cell[instr->b()->get_tag()].lat_value == ConstLatValue::Const) {
                    const auto& lat_cell = m_lat_cell[instr->b()->get_tag()];
                    instr->rebind_b(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
                }
            }
            break;
        case IRType::UnaryCalc:
            if (is_ssa_var(instr->r()) && m_lat_cell[instr->r()->get_tag()].lat_value == ConstLatValue::Const ) {
                const auto& lat_cell = m_lat_cell[instr->r()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
                instr->rewrite_type(IRType::Assign);
            }
            break;
        case IRType::Assign:
            if (is_ssa_var(instr->r()) && m_lat_cell[instr->r()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->r()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            break;
        case IRType::RParam:
            if (is_ssa_var(instr->a()) && m_lat_cell[instr->a()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->a()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            break;
        case IRType::ArrayStore:
            if (is_ssa_var(instr->a()) && m_lat_cell[instr->a()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->a()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            if (is_ssa_var(instr->b()) && m_lat_cell[instr->b()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->b()->get_tag()];
                instr->rebind_b(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            break;
        case IRType::ArrayLoad:
            if (is_ssa_var(instr->b()) && m_lat_cell[instr->b()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->b()->get_tag()];
                instr->rebind_b(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            break;
        case IRType::Call:
            break;
        case IRType::CallWithRet:
            break;
        case IRType::Return:
            break;
        case IRType::ValReturn:
            if(is_ssa_var(instr->a()) && m_lat_cell[instr->a()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->a()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            break;
        case IRType::BlockGoto:
            break;
        case IRType::BlockCondGoto:
            if (is_ssa_var(instr->a()) && m_lat_cell[instr->a()->get_tag()].lat_value == ConstLatValue::Const) {
                const auto& lat_cell = m_lat_cell[instr->a()->get_tag()];
                instr->rebind_a(m_ir_sym_table->create_value(lat_cell.basic_type, lat_cell.basic_value()));
            }
            break;
        case IRType::PhiFunc: {
            //对phi函数进行指令重写，删掉那些无法到达本块的参数
            auto& first_instr = m_instr_to_block[instr->no()]->get_instr_list_const().front();
            auto& phi_params = instr->a()->phi_params();
            for (auto param = phi_params.begin(); param != phi_params.end();) {
                bool is_executed_edge = false;
                if (param->from->is_entry()){
                    is_executed_edge = true;       
                } else {
                    for (const auto& edge : m_cfg_edge_list[param->from->get_instr_list_const().back().no()]) {
                        if (edge.to == first_instr.no() && edge.exsc_flag)
                            is_executed_edge = true;
                    }
                }
                
                if (!is_executed_edge)
                    param = phi_params.erase(param);
                else ++param;
            }
            break;
        }
        default:
            assert(0);
            Optimizer::optimizer_error("Unexpected instruction!");
            break;
        }
    }
}
//返回以to为终点的，可执行边的数量
int SparseConditionalConstantPropagation::edge_count(int to)
{
    int count = 0;
    for (auto edge : m_anti_cfg_edge_list[to]) {
        if (edge->exsc_flag)
            count++;
    }
    return count;
}
void SparseConditionalConstantPropagation::run()
{
    std::cout << "Running pass: Sparse Conditional Constant Propagation" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_ir_sym_table == nullptr) {
        Optimizer::optimizer_error("No IR symbol table specified");
    }
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef)
            work_unit(unit);
}