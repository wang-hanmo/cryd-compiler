#include <global_array_visit_substitution.h>
#include <cfg_manager.h>
#include <linear_ir_manager.h>
using namespace gavs;
//#define DEBUG_INFO_OPTIMIZER
void GlobalArrayVisitSubstitution::run()
{
    std::cout << "Running pass: Global Array Visit Substitution" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    global_init();
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::FuncDef) {
#ifdef DEBUG_INFO_OPTIMIZER
            std::cout << "********new unit********" << std::endl;
#endif
            bool anaylze_this_unit = build_ssa(&unit);
            if (!anaylze_this_unit) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "no analyze" << std::endl;
#endif
                continue;
            }
            m_vn = 0;
            m_vn_to_sym.clear();
            m_vn_of_sym.clear();
            m_vn_of_array_visit.clear();
            for (auto& instr : unit.get_definations_const())
                if (!instr.a()->is_global())
                    new_vn(instr.a());
            work_vn(unit.get_entry());
            //CFGManager::print_ir_unit(unit, std::cout);
            delete_mark(&unit);
        }
    }
}
void GlobalArrayVisitSubstitution::global_init()
{
    m_idfa->compute_side_effect_info();
    m_mem_def.clear();
    m_global_array_conut = 0;
    //初始化全局memory id
    for (auto& unit : *m_cfg) {
        if (unit.get_type() == IRUnitType::VarDef) {
            for (auto& instr : unit.get_definations()) {
                if (instr.a()->is_array()) {
                    m_mem_def.push_back(instr.a());
                    instr.a()->set_tag(m_global_array_conut++);
                }
            }
        }
    }
}
bool is_considered_array(IRSymbol* sym)
{
    return sym->is_array();
}
bool GlobalArrayVisitSubstitution::build_ssa(IRUnit* unit)
{
    bool anaylze_global_var = true;
    int param_array_count=0;
    m_mem_def.resize(m_global_array_conut);
    m_array_count = m_global_array_conut;
    for (auto& instr : unit->get_definations()) {
        if (instr.a()->is_array()) {
            m_mem_def.push_back(instr.a());
            instr.a()->set_tag(m_array_count++);
        }else if (instr.a()->kind()==IRSymbolKind::Param&&instr.a()->array_length() == IRArrayLength::IR_ARRAY_POINTER) {
            //m_mem_def.push_back(instr.a());
            //instr.a()->set_tag(m_array_count++);
            anaylze_global_var=false;
            //param_array_count++;
        }
    }
    //计算支配树,O(nlogn),n为基本块数
    CFGManager::build_dominator_tree(unit);
    //对支配树做dfs，求出每个节点的dfn，并填写idfn数组,O(n)
    m_idfn.clear();
    compute_idfn(unit->get_entry());
    //计算每个基本块的支配边界
    compute_dominance_frontiers();
    //计算每个memory被修改的基本块，维护哪些memory具有别名，不考虑这些有别名的memory
    m_mem_should_analyze = std::vector<bool>(m_array_count, true);
    if (!anaylze_global_var) {
        for (int i = 0; i < m_global_array_conut; ++i)
            m_mem_should_analyze[i] = false;
    }
    record_modified_block(unit);
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "memory should analyze:" << std::endl;
    for (int i = 0; i < m_array_count; ++i) {
        std::cout << m_mem_should_analyze[i];
    }
    std::cout << std::endl;
#endif
    //计算每个memory的迭代支配边界
    compute_dfp_for_all_vars();
    //插入版本更新标识（相当于SSA里的phi函数，但是不记录参数信息）
    insert_memory_converge_mark();
    //对每条指令编号
    m_instr_count = 0;
    for (auto& instr : unit->get_definations())
        instr.set_no(m_instr_count++);
    const int bb_count = m_idfn.size();
    for (BBId bbid = 0; bbid < bb_count; ++bbid) {
        auto bb=m_idfn[bbid];
        for (auto& instr : bb->get_instr_list())
            instr.set_no(m_instr_count++);
    }
    //维护memory版本编号
    m_max_memver = std::vector<int>(m_array_count,0);
    m_memver_of_instr = std::vector<int>(m_instr_count,-1);
    mark_memver_in_defination_area(unit);
    mark_memver_in_block(unit->get_entry());
#ifdef DEBUG_INFO_OPTIMIZER
    for (int i = 0; i < m_instr_count; ++i) {
        if (m_memver_of_instr[i] != -1)
            std::cout<<"instr " << i << " mem ver = " << m_memver_of_instr[i] << std::endl;
    }
#endif
    return true;
}
//计算每个block的dfn，并保存dfn到block的反向索引（inverse dfn)
void GlobalArrayVisitSubstitution::compute_idfn(IRBlock* block)
{
    BBId dfn = m_idfn.size();
    m_idfn.push_back(block);
    block->set_tag(dfn);//block的tag中保存dfn
    for (auto child : block->get_idom_child()) {
        compute_idfn(child);
    }
}
void GlobalArrayVisitSubstitution::compute_dominance_frontiers()
{
    const int block_count = m_idfn.size();
    m_dominance_frontier.clear();
    for (int i = 0; i < block_count; ++i)
        m_dominance_frontier.push_back(BitMap(block_count));
    //后序遍历
    for (int i = block_count - 1; i >= 0; --i) {
        IRBlock* block = m_idfn[i];
        int odegree = block->out_degree();
        //第一种情况，对于所有流图上的子节点，如果block不是它的支配节点，则加入支配边界
        for (int j = 0; j < odegree; ++j) {
            if (block->get_succ(j)->get_idom() != block) {
                m_dominance_frontier[i].set(block->get_succ(j)->get_tag());
            }
        }
        //第二种情况
        for (auto z : block->get_idom_child()) {
            BitMap& df_z = m_dominance_frontier[z->get_tag()];
            for (int y = 0; y < block_count; ++y) {
                if (df_z.test(y) && m_idfn[y]->get_idom() != block) {
                    m_dominance_frontier[i].set(y);
                }
            }
        }
    }
    /*
    std::cout << "Dominance Frontiers for blocks:" << std::endl;
    for (int i = 0; i < block_count; ++i) {
        std::cout << m_idfn[i]->get_index() << " " << m_dominance_frontier[i].get_string()<<std::endl;
    }*/
}
void GlobalArrayVisitSubstitution::record_modified_block(IRUnit* unit)
{
    const int block_count = m_idfn.size();
    m_mem_modified_block = std::vector<BitMap>(m_array_count,BitMap(block_count));
    m_current_memver = std::vector<std::list<MemVer>>(m_array_count, {0});
    //遍历每个block的每条语句，找到定值语句并填写变量在哪些block中被定过值
    for (int i = block_count - 1; i >= 0; --i) {
        IRBlock* block = m_idfn[i];
        //entry节点必须被添加进去
        if (block->is_entry()) {
            for (int j = 0; j < m_array_count; ++j) {
                m_mem_modified_block[j].set(i);
            }
        }
        for (const auto& instr : block->get_instr_list_const()) {
            //只考虑对原始定义的array而不是对指针的访问
            if (instr.type() == IRType::ArrayStore && is_considered_array(instr.r())) {
                m_mem_modified_block[instr.r()->get_tag()].set(i);
            }else if (instr.type()== IRType::Assign) {//如果遇到数组给指针赋值，则不分析这个数组
                if (is_considered_array(instr.a())) {
                    m_mem_should_analyze[instr.a()->get_tag()] = false;
                }
            }
            else if (instr.type() == IRType::BinaryCalc) {//如果遇到数组给指针赋值，则不分析这个数组
                if (is_considered_array(instr.a())) {
                    m_mem_should_analyze[instr.a()->get_tag()] = false;
                }
                if (is_considered_array(instr.b())) {
                    m_mem_should_analyze[instr.b()->get_tag()] = false;
                }
            }
            else if (instr.type() == IRType::Call|| instr.type() == IRType::CallWithRet) {
                if (instr.a()->global_sym()->is_internal_function()||m_idfa->has_side_effect(instr.a()->get_tag())) {
                    //遇到函数调用，如果它有副作用，认为所有数组都被定了值
                    for (int j = 0; j < m_array_count; ++j) {
                        m_mem_modified_block[j].set(i);
                    }
                }
            }
        }
    }
}
void GlobalArrayVisitSubstitution::compute_dfp_for_all_vars()
{
    m_mem_dominance_frontiers_plus.clear();
    for (int i = 0; i < m_array_count; ++i) {
        if (m_mem_should_analyze[i]) {
            m_mem_dominance_frontiers_plus.push_back(
                compute_dominance_frontiers_plus(m_mem_modified_block[i]));
        }
        else {
            m_mem_dominance_frontiers_plus.push_back(BitMap(0));
        }
    }

/*
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "Dominance Frontiers for arrays:" << std::endl;
    for (int i = 0; i < m_array_count; ++i) {
        std::cout << "l" << i << " " << m_mem_dominance_frontiers_plus[i].get_string() << std::endl;
    }
#endif*/
}
//计算迭代的支配边界,时间复杂度O(n^2)
BitMap GlobalArrayVisitSubstitution::compute_dominance_frontiers_plus(const BitMap& set)
{
    BitMap delta = set;
    BitMap res = set;
    bool change = true;
    while (change) {
        delta = compute_dominance_frontiers_set(delta);
        delta -= res;
        if (delta.empty())
            change = false;
        else res |= delta;
    }
    return compute_dominance_frontiers_set(res);
}
//对基本块集计算支配边界
BitMap GlobalArrayVisitSubstitution::compute_dominance_frontiers_set(const BitMap& set)
{
    BitMap res(set.size());
    for (int i = 0; i < set.size(); ++i)
        if (set.test(i))
            res |= m_dominance_frontier[i];
    return res;
}
void GlobalArrayVisitSubstitution::insert_memory_converge_mark()
{
    const int block_count = m_idfn.size();
    //遍历每个变量，在需要的block中添加phi函数
    for (MemId k = m_array_count - 1; k >= 0; --k) {
        for (BBId i = block_count - 1; i >= 0; --i) {
            if (m_mem_should_analyze[k]&&m_mem_dominance_frontiers_plus[k].test(i)) {
                IRBlock* block = m_idfn[i];
                if(!block->is_entry()&&!block->is_exit())
                    block->add_instr_to_front(IRInstr::create_mem_converge_mark(m_mem_def[k]));
            }
        }
    }
}
//对memory的修改，给予一个新的编号
int GlobalArrayVisitSubstitution::mark_memver_modify(IRSymbol* sym)
{
    int ver = ++m_max_memver[sym->get_tag()];
    m_current_memver[sym->get_tag()].push_back(ver);
    return ver;
}
//对memory的使用，给予一个现有编号
int GlobalArrayVisitSubstitution::mark_memver_use(IRSymbol* sym)
{
    return m_current_memver[sym->get_tag()].back();
}
void GlobalArrayVisitSubstitution::mark_memver_in_block(IRBlock* block)
{
    //维护符号栈增量记录，方便回退到本函数处理前的状态
    std::vector<int> rewrite_def_count(m_array_count);
    for (auto& instr : block->get_instr_list()) {
        if (instr.type() == IRType::ArrayLoad) {
            if(is_considered_array(instr.a()) && m_mem_should_analyze[instr.a()->get_tag()])
                m_memver_of_instr[instr.no()] = mark_memver_use(instr.a());
        }else if (instr.type() == IRType::ArrayStore) {
            if(is_considered_array(instr.r()) && m_mem_should_analyze[instr.r()->get_tag()]) {
                m_memver_of_instr[instr.no()] = mark_memver_modify(instr.r());
                rewrite_def_count[instr.r()->get_tag()]++;
            }
        }
        else if (instr.type() == IRType::MemoryConvergeMark) {
            assert(is_considered_array(instr.a()) &&m_mem_should_analyze[instr.a()->get_tag()]);
            m_memver_of_instr[instr.no()] = mark_memver_modify(instr.a());
            rewrite_def_count[instr.a()->get_tag()]++;
        }
        else if (instr.type() == IRType::Call || instr.type() == IRType::CallWithRet) {
            if (instr.a()->global_sym()->is_internal_function() || m_idfa->has_side_effect(instr.a()->get_tag())) {
                for (int i = 0; i < m_array_count;++i) {
                    if (m_mem_should_analyze[i]) {
                        mark_memver_modify(m_mem_def[i]);
                        rewrite_def_count[i]++;
                    }
                }
            }
        }
    }
    for (auto child : block->get_idom_child()) {
        //递归
        mark_memver_in_block(child);
    }
    //回退符号栈
    for (int i = 0; i < m_array_count; ++i) {
        while (rewrite_def_count[i] > 0) {
            m_current_memver[i].pop_back();
            rewrite_def_count[i]--;
        }
    }
}
void GlobalArrayVisitSubstitution::mark_memver_in_defination_area(IRUnit* unit)
{
    for (auto& instr : unit->get_definations()) {
        if (is_considered_array(instr.a()))
            m_memver_of_instr[instr.no()] = 0;
    }
}
void GlobalArrayVisitSubstitution::delete_mark(IRUnit* unit)
{
    const int bb_count = m_idfn.size();
    for (BBId bbid = 0; bbid < bb_count; ++bbid) {
        auto& instr_list= m_idfn[bbid]->get_instr_list();
        if (!m_idfn[bbid]->is_entry() && !m_idfn[bbid]->is_exit()) {
            while (instr_list.front().type() == IRType::MemoryConvergeMark)
                instr_list.pop_front();
        }
    }
}
static std::ostream& operator <<(std::ostream& os, const IRInstr& instr) {
    LinearIRManager::print_ir_instr(instr, os);
    return os;
}
static std::ostream& operator <<(std::ostream& os, IRSymbol* sym) {
    LinearIRManager::print_ir_symbol(sym, os);
    return os;
}
void GlobalArrayVisitSubstitution::work_vn_block(IRInstrList& program)
{
    for (auto instr = program.begin(); instr != program.end(); ++instr) {
        switch (instr->type()) {
        case IRType::PhiFunc: {
            int vn = -1;
            for (auto& phi_param : instr->a()->phi_params()) {
                if (auto iter = m_vn_of_sym.find(phi_param.sym); iter == m_vn_of_sym.end()) {
                    vn = -2;//缺失值编号的phi参数
                    break;
                }
                else {
                    if (vn == -1) {
                        vn = iter->second;
                    }
                    else if (vn != iter->second)
                        vn = -3;//不同值编号的phi参数
                }
            }
            if (vn == -2) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "unnumbered phi param" << std::endl;
#endif
                new_vn(instr->r());
            }
            else if (vn == -1) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "no valid path to phi param" << std::endl;
#endif
                new_vn(instr->r());
            }
            else if (vn == -3) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "diffirent numbered phi param" << std::endl;
#endif
                new_vn(instr->r());
            }
            else {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "same vn for all phi params" << vn << std::endl;
#endif
                m_vn_of_sym[instr->r()] = vn;
            }
            break;
        }
        case IRType::Assign: {
            int vn;
            if (instr->r()->is_global())
                break;
            if (instr->a()->is_global()) {
                vn = new_vn(instr->r());
            }
            else {
                vn = find_or_new_vn(instr->a());
                m_vn_of_sym[instr->r()] = vn;
            }

#ifdef DEBUG_INFO_OPTIMIZER
            std::cout << instr->r() << " vn = " << vn << std::endl;
#endif
            break;
        }
        case IRType::BinaryCalc:
            new_vn(instr->r());
            break;
        case IRType::UnaryCalc:
            new_vn(instr->r());
            break;
        case IRType::ArrayLoad: {
            if (!is_considered_array(instr->a()) ||
                instr->b()->is_global() || 
                !m_mem_should_analyze[instr->a()->get_tag()]) {
                new_vn(instr->r());
                break;
            }
            int vn_array = find_or_new_vn(instr->a());
            int ver = m_memver_of_instr[instr->no()];
            int vn_index = find_or_new_vn(instr->b());
            auto expr = ArrayVisitExpression(vn_array, ver, vn_index);
            //找到现有表达式，进行替代
            if (auto iter = m_vn_of_array_visit.find(expr); iter != m_vn_of_array_visit.end()) {
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "find expr " << *instr;

                std::cout << " vn = " << iter->second << " detail: "<<vn_array<<"<"<<ver<<">["<<vn_index<<"]" << std::endl;
#endif
                m_vn_of_sym[instr->r()] = iter->second;
                assert(m_vn_to_sym.find(iter->second) != m_vn_to_sym.end());
                auto sym = m_vn_to_sym[iter->second];
                if (sym->is_value())
                    sym=m_ir_sym_table->create_value(sym->basic_type(), sym->value());
                instr->rebind_a(sym);
                instr->rewrite_type(IRType::Assign);
            }
            else {//加入load表达式
                int vn = new_vn(instr->r());
                m_vn_of_array_visit[expr] = vn;
                m_vn_to_sym[vn] = instr->r();
#ifdef DEBUG_INFO_OPTIMIZER
                std::cout << "add expr "<< *instr << " vn = " << vn << " detail: " << vn_array << "<" << ver << ">[" << vn_index << "]" << std::endl;
#endif
            }
            break;
        }
        case IRType::ArrayStore: {
            //对array store做记录，做复写传播
            //由于revert SSA的缺陷，暂时只能考虑b是temp或者value的情况
            if (instr->b()->kind() != IRSymbolKind::Temp && instr->b()->kind() != IRSymbolKind::Value)
                break;
            if (is_considered_array(instr->r()) &&
                m_mem_should_analyze[instr->r()->get_tag()]&& 
                !instr->a()->is_global() && !instr->b()->is_global()) {
                int vn_array = find_or_new_vn(instr->r());
                int ver = m_memver_of_instr[instr->no()];
                int vn_index = find_or_new_vn(instr->a());
                auto expr = ArrayVisitExpression(vn_array,ver,vn_index);
                int vn_b = find_or_new_vn(instr->b());
                m_vn_of_array_visit[expr] = vn_b;
                m_vn_to_sym[vn_b] = instr->b();
            }
            break;
        }
        case IRType::CallWithRet: {
            new_vn(instr->r());
            break;
        }
        default:
            break;
        }
    }
}

void GlobalArrayVisitSubstitution::roll_back(IRInstrList& program)
{
    for (auto instr = program.rbegin(); instr != program.rend(); ++instr) {
        switch (instr->type()) {
        case IRType::BinaryCalc:
            [[fallthrough]];
        case IRType::UnaryCalc:
            m_vn_of_sym.erase(instr->r());
            break;
        case IRType::Assign: {
#ifdef DEBUG_INFO_OPTIMIZER
            auto iter = m_vn_of_sym.find(instr->r());
            assert(instr->r()->is_global() || iter != m_vn_of_sym.end());
            if(iter != m_vn_of_sym.end())
                std::cout << "erase vn " << iter->second << " sym=" << instr->r() << std::endl;
#endif
            m_vn_of_sym.erase(instr->r());
            break;
        }
        case IRType::PhiFunc:
            m_vn_of_sym.erase(instr->r());
            break;
        case IRType::ArrayLoad: {
            if (!is_considered_array(instr->a()) ||
                instr->b()->is_global() ||
                !m_mem_should_analyze[instr->a()->get_tag()]) {
                m_vn_of_sym.erase(instr->r());
                break;
            }
            int vn_array = find_or_new_vn(instr->a());
            int ver = m_memver_of_instr[instr->no()];
            int vn_index = find_or_new_vn(instr->b());
            auto expr = ArrayVisitExpression(vn_array, ver, vn_index);
            if (auto iter = m_vn_of_array_visit.find(expr); iter != m_vn_of_array_visit.end()) {
#ifdef DEBUG_INFO_OPTIMIZER
                auto it = m_vn_of_sym.find(instr->r());
                assert(it != m_vn_of_sym.end());
                std::cout << "erase vn " << iter->second << " sym=" << instr->r() << std::endl;
                std::cout << "erase vn " << iter->second << " expr=" << *instr << " detail: " << vn_array << "<" << ver << ">[" << vn_index << "]" << std::endl;
#endif
                m_vn_of_sym.erase(instr->r());
                m_vn_to_sym.erase(iter->second);
                m_vn_of_array_visit.erase(iter);
            }
            break;
        }
        case IRType::ArrayStore: {
            //由于revert SSA的缺陷，暂时只能考虑b是temp或者value的情况
            if (instr->b()->kind() != IRSymbolKind::Temp && instr->b()->kind() != IRSymbolKind::Value)
                break;
            if (is_considered_array(instr->r()) && m_mem_should_analyze[instr->r()->get_tag()]
                && !instr->a()->is_global()) {
                int vn_array = find_vn(instr->r());
                int ver = m_memver_of_instr[instr->no()];
                int vn_index = find_vn(instr->a());
                auto expr = ArrayVisitExpression(vn_array, ver, vn_index);
                if (auto iter = m_vn_of_array_visit.find(expr); iter != m_vn_of_array_visit.end()) {
#ifdef DEBUG_INFO_OPTIMIZER
                    std::cout << "erase vn " << iter->second << " expr=" << *instr << " detail: " << vn_array << "<" << ver << ">[" << vn_index << "]" << std::endl;
#endif
                    m_vn_to_sym.erase(iter->second);
                    m_vn_of_array_visit.erase(iter);
                }
                else assert(false);
            }
            break;
        }
        case IRType::CallWithRet: {
            m_vn_of_sym.erase(instr->r());
            break;
        }
        default:
            break;
        }
    }
}
void GlobalArrayVisitSubstitution::work_vn(IRBlock* block)
{
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "Work B" << block->get_index() << std::endl;
#endif
    work_vn_block(block->get_instr_list());
    for (auto to : block->get_idom_child())
        work_vn(to);
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "Roll back B" << block->get_index() << std::endl;
#endif
    roll_back(block->get_instr_list());
}
int GlobalArrayVisitSubstitution::find_or_new_vn(IRSymbol* sym) {
    if (auto iter = m_vn_of_sym.find(sym); iter == m_vn_of_sym.end()) {
        assert(sym == nullptr || sym->is_value() || sym->is_global());
#ifdef DEBUG_INFO_OPTIMIZER
        if (sym != nullptr)
            std::cout << sym << " vn = " << m_vn << std::endl;
        else std::cout << "nullptr vn = " << m_vn << std::endl;
#endif
        return m_vn_of_sym[sym] = m_vn++;
    }
    else return iter->second;
}
int GlobalArrayVisitSubstitution::find_vn(IRSymbol* sym) {
    if (auto iter = m_vn_of_sym.find(sym); iter == m_vn_of_sym.end())
        assert(false);
    else return iter->second;
    return -1;
}
int GlobalArrayVisitSubstitution::new_vn(IRSymbol* sym) {
#ifdef DEBUG_INFO_OPTIMIZER
    if (sym != nullptr)
        std::cout << sym << " vn = " << m_vn << std::endl;
    else std::cout << "nullptr vn = " << m_vn << std::endl;
#endif
    assert(m_vn_of_sym.find(sym) == m_vn_of_sym.end());
    return m_vn_of_sym[sym] = m_vn++;
}