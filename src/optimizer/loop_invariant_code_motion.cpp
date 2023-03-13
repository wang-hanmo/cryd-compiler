#include <loop_invariant_code_motion.h>
#include <cfg_manager.h>
#include <string>
#include <iostream>
#include <cassert>
#include <algorithm>
//#define DEBUG_INFO_OPTIMIZER
void LoopInvariantCodeMotion::work_unit(IRUnit* unit)
{
    //控制流分析，找出所有自然循环并插入头结点
    auto nloops=CFGManager::find_natural_loop_with_preheader(unit);
    //对变更的流图重新求出支配树信息
    CFGManager::build_dominator_tree(unit);
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "new unit" << std::endl;
    for (auto& nloop : nloops) {
        std::cout << "[Header] B" << nloop.header->get_index() << std::endl;
        for (auto node : nloop.node_set) {
            std::cout <<"B" << node->get_index() <<" ";
        }
        std::cout << std::endl;
    }
#endif
    if (nloops.empty())
        return;
    init(unit);
    //求解全局变量以及函数的副作用信息
    if (m_idfa->global_symbol_optimization_is_valid())
        m_idfa->compute_side_effect_info();
    //数据流分析，获得ud链
    solve_gen_prsv(unit);
    solve_rin_rout();
    solve_ud_duchain();
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "reach in" << std::endl;
    for (int b = 0; b < m_blk_cnt;++b) {
        std::cout << "B" << m_ibfn[b]->get_index();
        for (int i = 0; i < m_def_instr_cnt; ++i) {
            if (m_rin[b].test(i)) {
                std::cout<<" " << m_def_to_instr[i]->no();
            }
        }
        std::cout << std::endl;
    }
    std::cout << "udchain" << std::endl;
    for (int i = 0; i < m_instr_cnt; ++i) {
        std::cout << i << " A:";
        for(auto item: m_udchain_a[i])
            std::cout << " " << item;
        std::cout << " \t\tB:";
        for (auto item : m_udchain_b[i])
            std::cout << " " << item;
        std::cout << std::endl;
    }
#endif
    /*
    CFGManager::print_ir_unit(*unit, std::cout);*/
    
    //对循环进行包含关系分析，建立包含关系树（会按照包含基本块数量从小到大进行排序）
    inclusion_analysis_for_loops(nloops);
    //对包含关系树进行dfs，按照后序顺序对所有循环标注循环不变式,并进行循环不变式外提
    //树根的下标为m_nloop_count
    m_invariant.clear();
    m_invariant.resize(m_instr_cnt);
    analysis_inclusion_tree(nloops,m_nloop_count);
    //给所有preheader增加uncond goto语句
    for (auto& nloop : nloops) {
        nloop.preheader->add_instr(IRInstr::create_block_goto());
    }
    return;
}
//维护一个节点是否支配了所有出口节点
void LoopInvariantCodeMotion::dfs_on_dominator_tree(NaturalLoopInfo& nloop,IRBlock* blk)
{
    m_dominated_exit_count[blk->get_tag()] = 0;
    if (!nloop.has_node(blk))
        return;
    if (nloop.exit_set.find(blk) != nloop.exit_set.end())
        m_dominated_exit_count[blk->get_tag()] ++;
    for (auto child : blk->get_idom_child()) {
        dfs_on_dominator_tree(nloop,child);
        m_dominated_exit_count[blk->get_tag()] += m_dominated_exit_count[child->get_tag()];
    }
}
//求一个nloop内对变量的所有使用是否来自同一个定值，且这个定值也在nloop内。以及是否对变量定值了多次。
void LoopInvariantCodeMotion::calculate_dominate_all_use(NaturalLoopInfo& nloop)
{
    //首先把nloop包含的块维护到vector上，降低查询复杂度
    std::vector<bool> has_node;
    has_node.resize(m_blk_cnt);
    for (auto blk : nloop.node_set)
        has_node[blk->get_tag()] = true;
    //时间复杂度O(I*D[ref])，I表示循环内总语句数，D[ref]表示语句引用变量的定值语句数（不包含temp)
    for (auto blk : nloop.node_set) {
        for (auto& instr : blk->get_instr_list()) {
            for (const auto& [sym, udchain] : { std::make_pair(instr.a(),m_udchain_a),std::make_pair(instr.b(),m_udchain_b) }) {
                if (has_index(sym)) {
                    int tag = sym->get_tag();
                    //如果已经确定来自两个定值，就不再计算
                    if (m_sym_to_the_only_def[tag] == -1)
                        continue;
                    //查找是否使用了唯一的，在循环内的定值
                    for (int def : udchain[instr.no()]) {
                        if (has_node[m_instr_to_blk[def]->get_tag()]) {
                            //先前没有遇到过其定值，则加入
                            if (m_sym_to_the_only_def[tag] == -2) {
                                m_sym_to_the_only_def[tag] = def;
                            }
                            else if (m_sym_to_the_only_def[tag] != def) {//先前遇到过了不同的定值，说明被定值了多次
                                m_sym_to_the_only_def[tag] = -1;
                            }
                        }
                        else {//有不在循环内的定值
                            m_sym_to_the_only_def[tag] = -1;
                        }
                    }
                }
            }
            if (has_index(instr.r())) {
                int tag= instr.r()->get_tag();
                //先前没有遇到过其定值，则加入
                if (m_sym_to_the_only_def[tag] == -2) {
                    m_sym_to_the_only_def[tag] = instr.no();
                }else if(m_sym_to_the_only_def[tag]!=instr.no())//变量已经被多次定值
                    m_sym_to_the_only_def[tag] = -1;
            }
        }
    }   
}
//对循环包含关系树进行dfs
void LoopInvariantCodeMotion::analysis_inclusion_tree(std::vector<NaturalLoopInfo>& nloops,int current_index)
{
    for (auto child : m_sub_loop[current_index]) {
        analysis_inclusion_tree(nloops,child);
    }
    if (current_index < nloops.size()) {
        dfs_on_dominator_tree(nloops[current_index],nloops[current_index].header);
        //头结点一定支配了所有出口节点，确保自然循环求解正确
        assert(m_dominated_exit_count[nloops[current_index].header->get_tag()]== nloops[current_index].exit_node_count());
        //求一个nloop内对变量的所有使用是否来自同一个定值，且这个定值也在nloop内。以及是否对变量定值了多次。
        m_sym_to_the_only_def = std::vector<int>(m_sym_cnt, -2);
        calculate_dominate_all_use(nloops[current_index]);
        //对一个循环进行循环不变式的标注
        mark_invariant_code(nloops[current_index]);
        //进行循环不变式外提
        move_invariant_code(nloops[current_index]);
    }
}
static bool operator <(const std::pair<IRInstr, int>& lhs, const std::pair<IRInstr, int>& rhs)
{
    return lhs.second < rhs.second;
}
//循环不变式外提
void LoopInvariantCodeMotion::move_invariant_code(NaturalLoopInfo& nloops)
{
    //循环不变指令信息,first=指令的一个复制，second=指令被标记为循环不变指令的顺序
    std::vector<std::pair<IRInstr, int>> invar_instrs;
    for (auto node : nloops.node_set) {
        for (auto iter = node->get_instr_list().begin(); iter != node->get_instr_list().end();) {
            if (m_invariant[iter->no()] > 0) {
                invar_instrs.push_back({ *iter,m_invariant[iter->no()] });
                iter = node->get_instr_list().erase(iter);
            }
            else iter++;
        }
    }
    //按照指令被标记为循环不变指令的顺序排序
    std::sort(invar_instrs.begin(), invar_instrs.end());
    //在前置节点按顺序插入这些指令
    auto preheader = nloops.preheader;
    for (const auto& instr : invar_instrs) {
        preheader->add_instr(instr.first);
    }
}
//标注循环不变式
void LoopInvariantCodeMotion::mark_invariant_code(NaturalLoopInfo& nloop)
{
    m_invariant_count = 1;
    //按照bfs序对循环内基本块进行排序,复杂度O(BlogB)
    std::vector<int> node_set_ordered;
    for (auto blk : nloop.node_set)
        node_set_ordered.push_back(blk->get_tag());
    std::sort(node_set_ordered.begin(), node_set_ordered.end());
    for (auto node : node_set_ordered)
        for (auto& instr : m_ibfn[node]->get_instr_list_const())
            m_invariant[instr.no()]=0;
    bool change = true;
    while (change) {
        change = false;
        for (auto node : node_set_ordered) {
            change |= mark_block(nloop, m_ibfn[node]);
        }
    }
#ifdef DEBUG_INFO_OPTIMIZER
    std::cout << "Loop invariant instructions:" << std::endl;
    for (auto node : node_set_ordered)
        for (auto instr : m_ibfn[node]->get_instr_list_const())
            std::cout << instr.no() << " " << m_invariant[instr.no()] << std::endl;
    for (auto blk : node_set_ordered)
        CFGManager::print_ir_block(m_ibfn[blk], std::cout);
    std::cout << std::endl;
#endif
}
//符号是否有index
bool LoopInvariantCodeMotion::has_index(IRSymbol* sym)
{
    if (sym == nullptr)
        return false;
    return sym->kind() == IRSymbolKind::Temp||sym->kind() == IRSymbolKind::Local || sym->kind() == IRSymbolKind::Param;
}
bool LoopInvariantCodeMotion::check_invariant(IRSymbol* sym, std::vector<int>aviliable_defs, std::set<IRBlock*>& loop_blk_set)//检测一个引用是否是循环不变的
{
    //是常量
    if (sym->kind() == IRSymbolKind::Value) {
        return true;
    }
    //全局变量不是循环不变的
    if (sym->kind() == IRSymbolKind::Global) {
        return false;
    }
    //不检查数组元素（因为调用者已经排除了数组访问语句）
    //符号的定值不在循环包含的基本块中
    bool is_invariant = true;
    for (auto i : aviliable_defs) {
        if (loop_blk_set.find(m_instr_to_blk[i]) != loop_blk_set.end()) {
            is_invariant = false;
            break;
        }
    }
    if (is_invariant)
        return true;
    //符号只有一个可达定值，并且该定值是本循环内的循环不变量
    if (aviliable_defs.size() == 1&&
        loop_blk_set.find(m_instr_to_blk[aviliable_defs[0]]) != loop_blk_set.end()&&
        m_invariant[aviliable_defs[0]] != 0) {
            return true;
    }
    return false;
}
bool LoopInvariantCodeMotion::mark_block(NaturalLoopInfo& nloop, IRBlock* block)
{
    std::set<int> used_var;//本基本块内使用过的变量
    bool change = false;
    for (auto instr = block->get_instr_list().begin(); instr != block->get_instr_list().end(); ++instr) {
        if (m_invariant[instr->no()] > 0)
            continue;
        bool is_invariant = true;
        //右值符号都是循环不变的
        switch (instr->type()) {
        case IRType::BinaryCalc:
            //is_invariant &= instr.op() != IROper::DivI && instr.op() != IROper::DivF;
            is_invariant &= check_invariant(instr->a(), m_udchain_a[instr->no()], nloop.node_set)
                          && check_invariant(instr->b(), m_udchain_b[instr->no()], nloop.node_set);
            is_invariant &= !is_relation_oper(instr->op());
            break;
        case IRType::UnaryCalc:
            is_invariant &= check_invariant(instr->a(), m_udchain_a[instr->no()], nloop.node_set);
            is_invariant &= !is_relation_oper(instr->op());
            break;
        case IRType::Assign:
            is_invariant &= check_invariant(instr->a(), m_udchain_a[instr->no()], nloop.node_set);
            break;
        //函数调用的外提
        case IRType::CallWithRet:
            if (m_idfa->global_symbol_optimization_is_valid()) {
                if (instr->a()->global_sym()->is_internal_function() || m_idfa->has_side_effect(instr->a()->get_tag()) || m_idfa->affected_by_env(instr->a()->get_tag())) {
                    is_invariant = false;
                } else {
                    const int param_count = instr->b()->int_value();
                    auto iter = instr;
                    std::advance(iter, -param_count);
                    for (int i = 0; i < param_count; ++i, ++iter) {
                        assert(iter->type() == IRType::RParam);
                        //不能有数组参数，且参数必须都是循环不变的
                        if (iter->a()->is_array_or_pointer()||!check_invariant(iter->a(), m_udchain_a[iter->no()], nloop.node_set)) {
                            is_invariant = false;
                            break;
                        }
                    }
                }
            }
            else is_invariant = false;
            break;
        default:
            is_invariant = false;
            break;
        }
        /*
        if (has_index(instr.a())) {
            used_var.insert(instr.a()->get_tag());
        }
        if (has_index(instr.b())){
            used_var.insert(instr.b()->get_tag());
        } */
        if (is_invariant) {
            if (instr->r()->kind() != IRSymbolKind::Global) {
                //整个循环内
                //r支配了所有的使用,且在循环内对r只有唯一一次赋值
                assert(m_sym_to_the_only_def[instr->r()->get_tag()] != -2);
                is_invariant &= m_sym_to_the_only_def[instr->r()->get_tag()] >= 0;
                /*//本基本块内该语句前没有对左值的使用
                //is_invariant &= used_var.find(instr.r()->get_tag()) == used_var.end();*/

                //支配了所有出口基本块
                is_invariant &= m_dominated_exit_count[block->get_tag()] == nloop.exit_node_count();
            }
            else is_invariant = false;
        }
        if (is_invariant) {
            change = true;
            if (instr->type()== IRType::CallWithRet) {
                const int param_count = instr->b()->int_value();
                auto iter = instr;
                std::advance(iter, -param_count);
                for (int i = 0; i < param_count; ++i, ++iter) {
                    assert(iter->type() == IRType::RParam);
                    m_invariant[iter->no()] = m_invariant_count++;
                }
                m_invariant[iter->no()] = m_invariant_count++;
            }else
                m_invariant[instr->no()] = m_invariant_count++;
        }
    }
    return change;
}
bool loop_size_less(const NaturalLoopInfo& lhs,const NaturalLoopInfo& rhs)
{
    return lhs.node_set.size() < rhs.node_set.size();
}
void LoopInvariantCodeMotion::inclusion_analysis_for_loops(std::vector<NaturalLoopInfo>& nloops)
{
    m_sub_loop.clear();
    m_nloop_count = nloops.size();
    std::sort(nloops.begin(), nloops.end(), loop_size_less);
    m_sub_loop.resize(m_nloop_count + 1);//序号最大的"循环"为整个控制流图
    //二重循环检测循环的包含关系，复杂度O(L^2log(B))
    for (int i = 0; i < m_nloop_count; ++i) {
        bool found_parent = false;
        for (int j = i+1; j < m_nloop_count; ++j) {
            //包含了header则包含所有元素
            if (nloops[j].node_set.find(nloops[i].header) != nloops[j].node_set.end()) {
                m_sub_loop[j].push_back(i);
                found_parent = true;
                break;
            }
        }
        if (!found_parent)
            m_sub_loop[m_nloop_count].push_back(i);
    }
}
void LoopInvariantCodeMotion::init(IRUnit* unit)
{
    m_idx_to_def.clear();       //指令全编号->定值指令编号
    m_def_to_instr.clear();     //定值指令编号->指令
    m_instr_to_blk.clear();     //指令编号->指令所在块
    m_ibfn.clear();             //块编号->块
    m_gen.clear();              //生成定值向量，block->定值指令向量
    m_prsv.clear();             //保留定值向量，block->定值指令向量
    m_rin.clear();              //基本块入口有效定值向量，block->定值指令向量
    m_rout.clear();             //基本块出口有效定值向量，block->定值指令向量
    m_sym_to_def_instr.clear(); //符号->所有定值指令的全编号(不考虑是否可到达)
    m_udchain_a.clear();        //操作数a的ud链，指令->指令列表(全编号）
    m_udchain_b.clear();        //操作数b的ud链，指令->指令列表(全编号）
    m_blk_cnt = 0;              //基本块数量
    m_dominated_exit_count.clear();
    m_instr_cnt = 0;            //指令数量
    m_def_instr_cnt = 0;        //对local/param型非数组变量进行定值的指令数量
    m_sym_cnt = 0;              //local/param/temp符号数量
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    for (auto& def_instr : unit->get_definations()) {
        def_instr.set_no(m_instr_cnt++);
        m_idx_to_def.push_back(m_def_instr_cnt);
        m_instr_to_blk.push_back(unit->get_entry());
        //第一遍遍历无法在线性时间内判断符号的数量，先初始化所有定值语句中左值符号的tag为-1，留到后面进行判断
        def_instr.a()->set_tag(-1);
        //if (def_instr.a()->is_non_array()) {// is_numbered_def_instr
        m_def_to_instr.push_back(&def_instr);
        m_def_instr_cnt++;
        //}
    }
    //O(BlogB+I)
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        m_ibfn.push_back(now);
        now->set_tag(m_blk_cnt++);
        for (auto& instr : now->get_instr_list()) {
            instr.set_no(m_instr_cnt++);
            m_idx_to_def.push_back(m_def_instr_cnt);
            m_instr_to_blk.push_back(now);
            //第一遍遍历无法在线性时间内判断符号的数量，先初始化所有定值语句中左值符号的tag为-1，留到后面进行判断
            if (instr.r() != nullptr)
                instr.r()->set_tag(-1);
            //if (is_numbered_def_instr(instr)) {
            m_def_to_instr.push_back(&instr);
            m_def_instr_cnt++;
            //}
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
    m_dominated_exit_count.resize(m_blk_cnt);
    m_udchain_a.resize(m_instr_cnt);
    m_udchain_b.resize(m_instr_cnt);
    /*
    m_duchain.clear();
    for (int i = 0; i < m_def_instr_cnt; ++i)
        m_duchain.push_back(BitMap(m_blk_cnt));*/
    //第二遍遍历维护每个符号的所有定值语句，便于进行到达定值分析
    //O(B+I)
    for (auto& instr : unit->get_definations()) {
        if (has_index(instr.a())) {
            if (instr.a()->get_tag() == -1) {
                m_sym_to_def_instr.push_back({ instr.no() });
                instr.a()->set_tag(m_sym_cnt++);
            } else {
                m_sym_to_def_instr[instr.a()->get_tag()].push_back(instr.no());
            }
        }
    }
    for (int i = 0; i < m_blk_cnt; ++i) {
        m_gen.push_back(BitMap(m_def_instr_cnt));
        m_prsv.push_back(BitMap(m_def_instr_cnt));
        m_rin.push_back(BitMap(m_def_instr_cnt));
        m_rout.push_back(BitMap(m_def_instr_cnt));
        IRBlock* blk = m_ibfn[i];
        for (auto& instr : blk->get_instr_list()) {
            if (has_index(instr.r())) {
                if (instr.r()->get_tag() == -1) {
                    m_sym_to_def_instr.push_back({ instr.no() });
                    instr.r()->set_tag(m_sym_cnt++);
                }else {
                    m_sym_to_def_instr[instr.r()->get_tag()].push_back(instr.no());
                }
            }
        }
    }
}
//求解每个block内的gen和prsv
void LoopInvariantCodeMotion::solve_gen_prsv(IRUnit* unit)
{
    //参数/局部变量的初始值（对局部变量来说，无意义)
    auto entry_idx = unit->get_entry()->get_tag();
    m_prsv[entry_idx].set_all();
    for (auto& instr : unit->get_definations()) {
        if (has_index(instr.a())) {
            for (int idx : m_sym_to_def_instr[instr.a()->get_tag()]) {
                m_prsv[entry_idx].reset(m_idx_to_def[idx]);
                m_gen[entry_idx].reset(m_idx_to_def[idx]);
            }
            m_gen[entry_idx].set(m_idx_to_def[instr.no()]);
        }
    }
    //O(I*D[ref])I表示语句数，D[ref]表示语句引用变量的定值语句数(不包含temp)
    for (int i = 0; i < m_blk_cnt; ++i) {
        IRBlock* blk = m_ibfn[i];
        if (blk->is_entry())continue;
        m_prsv[i].set_all();
        for (auto& instr : blk->get_instr_list()) {
            if (is_numbered_def_instr(instr)){
                for (int idx : m_sym_to_def_instr[instr.r()->get_tag()]) {
                    m_prsv[i].reset(m_idx_to_def[idx]);
                    m_gen[i].reset(m_idx_to_def[idx]);
                }
                m_gen[i].set(m_idx_to_def[instr.no()]);
            }
        }
    }
    
}
//迭代求解reach in和reach out
void LoopInvariantCodeMotion::solve_rin_rout()
{
    //O(B*D^2)B表示基本块数，D表示定值语句数（不包含temp）
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < m_blk_cnt; ++i) {
            IRBlock* blk = m_ibfn[i];
            for(auto blk_j:blk->get_pred()){
                int j=blk_j->get_tag();
                m_rin[i] |= m_rout[j];
            }
            auto res = m_gen[i] | (m_rin[i] & m_prsv[i]);
            if (!changed && m_rout[i] != res) {
                changed = true;
            }
            m_rout[i] = res;
        }
    }
}
//设定一个符号的ud链
 //最坏O(D[ref])，I表示总语句数，D[ref]表示语句引用变量的定值语句数（不包含temp）
void LoopInvariantCodeMotion::set_ud_duchain(IRSymbol* sym,int belonging_to_blk,std::vector<int>& udchain_vec,BitMap& working_bitmap) {
    if (sym->kind() == IRSymbolKind::Local || sym->kind() == IRSymbolKind::Param) {
        if(sym->get_tag()>=0)
            for (int idx : m_sym_to_def_instr[sym->get_tag()]) {
                if (working_bitmap.test(m_idx_to_def[idx])) {
                    //m_duchain[m_idx_to_def[idx]].set(belonging_to_blk);
                    udchain_vec.push_back(idx);
                }
            }
    }else if (sym->kind() == IRSymbolKind::Temp) {
        assert(m_sym_to_def_instr[sym->get_tag()].size() == 1);
        udchain_vec.push_back(m_sym_to_def_instr[sym->get_tag()].back());
    }
}
//求解每一条指令中每一个符号的ud链
//最坏O(I*D[ref])，I表示总语句数，D[ref]表示语句引用变量的定值语句数（不包含temp）
void LoopInvariantCodeMotion::solve_ud_duchain()
{
    for (int i = 0; i < m_blk_cnt; ++i) {
        IRBlock* blk = m_ibfn[i];
        auto working_bitmap= m_rin[i];
        for (auto& instr : blk->get_instr_list()) {
            if (instr.a() != nullptr)
                set_ud_duchain(instr.a(), i, m_udchain_a[instr.no()],working_bitmap);
            if (instr.b() != nullptr)
                set_ud_duchain(instr.b(), i, m_udchain_b[instr.no()], working_bitmap);
            if (is_numbered_def_instr(instr)) {
                for (int idx : m_sym_to_def_instr[instr.r()->get_tag()])
                    working_bitmap.reset(m_idx_to_def[idx]);
                working_bitmap.set(m_idx_to_def[instr.no()]);
            }
        }
    }
}
//是否为对local/param型变量进行定值的语句
bool LoopInvariantCodeMotion::is_numbered_def_instr(const IRInstr& instr)
{
    return instr.r()!=nullptr&& (instr.r()->kind() == IRSymbolKind::Local || instr.r()->kind() == IRSymbolKind::Param);
}
void LoopInvariantCodeMotion::run()
{
    std::cout << "Running pass: Loop Invariant Code Motion" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef)
            work_unit(&unit);
}
