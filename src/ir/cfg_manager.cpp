#include<linear_ir_manager.h>
#include<unordered_map>
#include<queue>
#include<set>
#include<map>
#include<stack>
#include<cassert>
#include<sstream>
#include<algorithm>
#include<cfg_manager.h>
enum CFGJumpTarget
{
    Unconditional=0,
    False=0,
    True=1
};
enum class IRToCFGState:char
{
    InGlobalArea,//不在函数内
    InDefArea,   //在函数内部定义区（包括参数和局部变量定义）
    InFuncArea,  //在函数内部功能区，但不在基本块内
    InBlock,     //在基本块内
};
//使用状态机实现线性IR到控制流图的转换
IRProgram* CFGManager::gen_from_linear_ir(const IRInstrList& list)
{
    IRProgram* prog = new IRProgram;
    prog->push_back(IRUnit::create_var_def());
    IRUnit& global_decl_unit=prog->back();
    IRUnit* current_func=nullptr;                   //当前正在处理的funciton
    std::unordered_map<int,IRBlock*> lbl_to_block;  //标号到block的映射
    IRBlock* current_block = nullptr;               //当前正在处理的block
    IRToCFGState state=IRToCFGState::InGlobalArea;
    for(auto iter=list.begin();iter!=list.end();){
        switch (state) {
        case IRToCFGState::InGlobalArea: {
            if (iter->type() == IRType::GlobalDecl) {
                global_decl_unit.add_def_instr(*iter);
            } else if (iter->type() == IRType::FuncDef) {
                // assert(state == IRToCFGState::InGlobalArea);
                if(state != IRToCFGState::InGlobalArea)
                    ir_to_cfg_error("Unexpected instruction at state: InGlobalArea", 0);
                prog->push_back(IRUnit::create_func_def());
                current_func = &prog->back();
                current_func->add_def_instr(*iter);
                state = IRToCFGState::InDefArea;
            } else {
                ir_to_cfg_error("Unexpected instruction at state: InGlobalArea", 1);
            }
            iter++;
            break;
        }
        case IRToCFGState::InDefArea: {
            if (iter->type() == IRType::FParam || iter->type() == IRType::LocalDecl) {
                current_func->add_def_instr(*iter);
                iter++;
            } else if(iter->type() == IRType::FuncEnd) {
                ir_to_cfg_error("Function must have a return statement", 2);
            } else if (iter->type() == IRType::GlobalDecl || iter->type() == IRType::FuncDef) {
                ir_to_cfg_error("Can not define global var or function in a function",  3);
            } else {
                //状态转移到InBlock，新建基本块
                state = IRToCFGState::InBlock;
                current_block = new IRBlock();
                //如果是标号语句，令当前block为标号对应的block
                if (iter->type() == IRType::Label)
                    lbl_to_block[iter->lbl0()] = current_block;
                // assert(current_func != nullptr);
                if(current_func == nullptr)
                    ir_to_cfg_error("4",  4);
                current_func->get_entry()->set_edge(CFGJumpTarget::Unconditional, current_block);
                if (iter->type() == IRType::Label)
                    iter++;
            }
            break;
        }
        case IRToCFGState::InFuncArea: {
            //此状态下除了FuncEnd和Label以外都是死代码
            if (iter->type() == IRType::FuncEnd) {
                state = IRToCFGState::InGlobalArea;
                lbl_to_block.clear();
            } else if (iter->type() == IRType::GlobalDecl || iter->type() == IRType::FuncDef) {
                ir_to_cfg_error("Can not define global var or function in a function",  5);
            } else if (iter->type() == IRType::Label) {
                current_block = lbl_to_block[iter->lbl0()];
                if (current_block == nullptr) {
                    current_block = new IRBlock();
                    lbl_to_block[iter->lbl0()] = current_block;
                }
                state = IRToCFGState::InBlock;
            }
            iter++;
            break;
        }
        case IRToCFGState::InBlock: {
            if (iter->type() == IRType::GlobalDecl || iter->type() == IRType::FuncDef) {
                ir_to_cfg_error("Can not define global var or function in a function",  6);
            }else if (iter->type() == IRType::LocalDecl || iter->type() == IRType::FParam) {
                ir_to_cfg_error("Can not define local var or parameter out of function define area", 7);
            }else if (iter->type() == IRType::FuncEnd) {
                ir_to_cfg_error("Function must have a return statement", 8);
            }else if (iter->type() == IRType::Label) {//新标号意味着新开启了一个block
                if (lbl_to_block[iter->lbl0()] == nullptr) {
                    current_block->add_instr(IRInstr::create_block_goto());
                    IRBlock* new_block = new IRBlock();
                    current_block->set_edge(CFGJumpTarget::Unconditional, new_block);
                    lbl_to_block[iter->lbl0()] = new_block;
                    current_block = new_block;
                } else {
                    current_block->add_instr(IRInstr::create_block_goto());
                    current_block->set_edge(CFGJumpTarget::Unconditional, lbl_to_block[iter->lbl0()]);
                    current_block = lbl_to_block[iter->lbl0()];
                }
            } else if (iter->type() == IRType::Goto) {
                IRBlock* blk0 = lbl_to_block[iter->lbl0()];
                if (blk0 == nullptr) {
                    blk0 = new IRBlock();
                    lbl_to_block[iter->lbl0()] = blk0;
                }
                current_block->add_instr(IRInstr::create_block_goto());
                current_block->set_edge(CFGJumpTarget::Unconditional, blk0);
                state = IRToCFGState::InFuncArea;
            } else if (iter->type() == IRType::CondGoto) {
                IRBlock* blk0 = lbl_to_block[iter->lbl0()];
                IRBlock* blk1 = lbl_to_block[iter->lbl1()];
                if (blk0 == nullptr) {
                    blk0 = new IRBlock();
                    lbl_to_block[iter->lbl0()] = blk0;
                } else blk0 = lbl_to_block[iter->lbl0()];
                if (blk1 == nullptr) {
                    blk1 = new IRBlock();
                    lbl_to_block[iter->lbl1()] = blk1;
                }
                current_block->add_instr(IRInstr::create_block_cond_goto(iter->a()));
                current_block->set_edge(CFGJumpTarget::False, blk0);
                current_block->set_edge(CFGJumpTarget::True, blk1);
                state = IRToCFGState::InFuncArea;
            } else if (iter->type() == IRType::Return || iter->type() == IRType::ValReturn) {
                current_block->add_instr(*iter);
                if (current_func == nullptr)
                    ir_to_cfg_error("Return statement out of function",  9);
                else
                    current_block->set_edge(CFGJumpTarget::Unconditional, current_func->get_exit());
                state = IRToCFGState::InFuncArea;
            } else {
                current_block->add_instr(*iter);
            }
            iter++;
            break;
        }
        default:
            ir_to_cfg_error("Undefined transfer state.",  10);
            break;
        }
    }
    return prog;
}
IRProgram* CFGManager::gen_from_ast(ASTNode* root)
{
    IRInstrList* linear_ir = LinearIRManager::gen_ir(root);
    auto res=gen_from_linear_ir(*linear_ir);
    delete linear_ir;
    return res;
}
void CFGManager::print_ir_block(IRBlock* block,std::ostream& os)
{
    os <<"....Block B"<< block->get_index();
    if (block->is_entry())os << " [Entry]";
    if (block->is_exit())os << " [Exit]";
    /*
    if (block->get_idom() != nullptr) {
        os << " [idom = B"<< block->get_idom()->get_index() << "]";
    }*/
    os << " [Tag = " << block->get_tag() << "]";
    os << " [In " << block->in_degree()<<" Out "<<block->out_degree() << "]";
    os << std::endl;
    LinearIRManager::print_ir_list(block->get_instr_list_const(),os,"    ");
    if(block->out_degree()==1){
        os <<"....Block End [Uncond] B"<<block->get_succ(0)->get_index() <<std::endl;
    }else if(block->out_degree()==2){
        os <<"....Block End [True] B"<<block->get_succ(1)->get_index();
        os <<" [False] B"<<block->get_succ(0)->get_index() <<std::endl;
    }
    //os <<"....Block End"<<std::endl;
    os <<std::endl;
}
//按照宽度优先的顺序输出整张控制流图
void CFGManager::print_control_flow_graph(IRBlock* entry,std::ostream& os)
{
    std::set<IRBlock*> is_printed;
    std::queue<IRBlock*> q;
    q.push(entry);
    while(!q.empty()){
        IRBlock* now=q.front();
        q.pop();
        print_ir_block(now,os);
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
        }
    }
}
void CFGManager::print_ir_unit(const IRUnit& unit,std::ostream& os)
{
    if(unit.get_type()==IRUnitType::FuncDef){
        os <<"Unit FuncDef"<<std::endl;
        os <<"----Def Area:"<<std::endl;
        LinearIRManager::print_ir_list(unit.get_definations_const(),os,"    ");
        os <<"----Block Area:"<<std::endl;
        print_control_flow_graph(unit.get_entry(),os);
        os <<"Unit End"<<std::endl;
    }else{
        os <<"Unit VarDef"<<std::endl;
        LinearIRManager::print_ir_list(unit.get_definations_const(), os,"    ");
        os <<"Unit End"<<std::endl;
    }
    os <<std::endl;
}
void CFGManager::print_ir_program(const IRProgram& prog,std::ostream& os)
{
    for(auto& unit:prog)
        print_ir_unit(unit,os);
}

void CFGManager::ir_to_cfg_error(const std::string &error_msg/*, const IRProgram& program_part*/, int error_code)
{
    //print_ir_program(program_part);
    std::cerr << "[IR to CFG Error]" << error_msg << std::endl;
    exit(ErrorCode::CFG_ERROR + error_code);
}
void CFGManager::build_dominator_tree(IRUnit* target)//对IRUnit求支配树
{
    //assert(target->get_type() == IRUnitType::FuncDef);
    if(target->get_type() != IRUnitType::FuncDef)
        ir_to_cfg_error("11", 11);
    if (!target->is_dominator_tree_info_valid()) {
        std::vector<BuildDomTreeInfo> node = {};
        std::set<IRBlock*> visited = {};
        build_dom_tree_init(target->get_entry(), -1, node, visited);
        build_dom_tree_solve(target->get_entry(), node);
        target->set_dominator_tree_info_valid(true);
    }
}
/*
    求解dfn，并初始化BuildDomTreeInfo
    包括:
    1、计算dfn和dfs_tree_parent
    2、初始化ufs_parent、ufs_min、sdom为自身
*/
void CFGManager::build_dom_tree_init(IRBlock* block,int dfn_parent, std::vector<BuildDomTreeInfo>& node, std::set<IRBlock*>& visited)
{
    visited.insert(block);
    node.push_back(BuildDomTreeInfo());
    int dfn = node.size() - 1;
    node[dfn].idfn = block;
    node[dfn].sdom = dfn;
    node[dfn].ufs_min = dfn;
    node[dfn].ufs_parent = dfn;
    node[dfn].dfs_tree_parent = dfn_parent;
    block->set_tag(dfn);
    int child_size = block->out_degree();
    for (int i = 0; i < child_size; ++i) {
        IRBlock* child = block->get_succ(i);
        if (visited.find(child) == visited.end()) {
            build_dom_tree_init(block->get_succ(i), dfn, node,visited);
        }
        node[child->get_tag()].cfg_pred.push_back(dfn);
    }
}
/*
    完成支配树的求解
    包括:
    1、计算半支配点sdom
    2、计算支配点idom
*/
void CFGManager::build_dom_tree_solve(IRBlock* entry, std::vector<BuildDomTreeInfo>& node)
{
    for (int u = node.size()-1; u >= 0; --u) {
        /*
            处理半支配树上子节点的idom
            这一步必须在计算半支配节点前
            原因是求解idom时要查询的sdom[v]到v的链是不包括sdom[v](也就是u)的,
            而计算半支配节点后，ufs_min[u]和sdom[u]不再等于自身，相当于u也被加入了链中，结果会产生错误
        */
        for (int j = 0; j < node[u].sdom_child.size(); ++j) {
            int v = node[u].sdom_child[j];
            build_dom_tree_ufs_find(v,node);
            //由于v是u在半支配树上的子节点，u就是sdom(v)
            if (node[node[v].ufs_min].sdom == u)node[v].idom = u;//情况1，idom已知
            else node[v].idom = node[v].ufs_min;//情况2，由于依赖ufs_min(v)的idom，暂时未知，就先存下ufs_min(v)
        }
        //计算本节点的半支配节点
        for (int j = 0; j < node[u].cfg_pred.size(); ++j) {
            int p = node[u].cfg_pred[j];
            //更新ufs_min信息
            build_dom_tree_ufs_find(p, node);
            if (node[node[p].ufs_min].sdom < node[u].sdom)
                node[u].sdom = node[node[p].ufs_min].sdom;
        }
        node[u].ufs_parent = node[u].dfs_tree_parent;  //并查集的合并
        node[node[u].sdom].sdom_child.push_back(u);//维护半支配树
    }
    //正序更新idom为正确的idom
    for (int i = 1; i < node.size(); ++i) {
        if (node[i].sdom != node[i].idom) {
            node[i].idom = node[node[i].idom].idom;
        }
        node[i].idfn->set_idom(node[node[i].idom].idfn);
        node[i].idfn->clear_idom_child();//清空child
    }
    node[0].idfn->set_idom(nullptr);
    node[0].idfn->clear_idom_child();//清空child
    //维护支配树的子节点
    for (int i = 1; i < node.size(); ++i)
        node[i].idfn->get_idom()->add_idom_child(node[i].idfn);
}
/*
    带权并查集的find操作
    返回值为并查集维护集合的根节点
    节点的大小指其dfn的大小
*/
int CFGManager::build_dom_tree_ufs_find(int dfn_x, std::vector<BuildDomTreeInfo>& node)
{
    if (dfn_x == node[dfn_x].ufs_parent)
        return dfn_x;
    int root = build_dom_tree_ufs_find(node[dfn_x].ufs_parent,node);
    //维护ufs_min为从当前节点到根的链上（按照求解顺序，只包含比当前dfn大的节点）sdom最小的那个元素
    if (node[node[dfn_x].ufs_min].sdom > node[node[node[dfn_x].ufs_parent].ufs_min].sdom)
        node[dfn_x].ufs_min = node[node[dfn_x].ufs_parent].ufs_min;
    return node[dfn_x].ufs_parent = root;//并查集的路径压缩
}

/*
*   找出所有自然循环，加入前置节点
*   要求不含有不可达基本块
*/
std::vector<NaturalLoopInfo> CFGManager::find_natural_loop_with_preheader(IRUnit* target)
{
    std::vector<NaturalLoopInfo> nloop_info;
    std::map<int, int> head_to_nloop;   //从头结点映射到对应循环
    int dfn_count = -1;
    std::vector<IRBlock*> idfn;         //逆dfn，从dfn映射到IRblock
    std::vector<int> dfn_postorder;     //记录递归返回时的dfn
    //1、对目标单元求出支配树信息
    if (!target->is_dominator_tree_info_valid())
        build_dominator_tree(target);
    //2、支配树上dfs，维护访问序和离开序（子树信息）
    find_natural_loop_dfs(target->get_entry(),idfn,dfn_postorder,dfn_count);
    //3、按照dfs序遍历BB，维护回边，以及循环首结点集合（无重复）
    for (int i = 0; i <= dfn_count; ++i) {
        IRBlock* block = idfn[i];
        for (int index = 0; index < block->out_degree(); ++index) {
            auto child = block->get_succ(index);
            assert(child != nullptr);
            //检测回边
            if (is_dominate(child->get_tag(),i, dfn_postorder[child->get_tag()], dfn_postorder[i])) {
                //在nloop_info中维护回边集合，（合并所有头结点相同的自然循环）
                if (auto iter= head_to_nloop.find(child->get_tag());iter!=head_to_nloop.end()) {
                    //如果头结点已经被加入集合，则给对应自然循环新增一个回边
                    nloop_info[iter->second].back_edge_list.insert(block);
                } else {
                    //如果头结点没被加入集合，则加入集合，并新增一个自然循环
                    nloop_info.push_back(NaturalLoopInfo(child));
                    nloop_info.back().back_edge_list.insert(block);
                    nloop_info.back().node_set.insert(child);
                    head_to_nloop[child->get_tag()] = nloop_info.size() - 1;
                }
            }
        }
    }
    //4、插入循环前置节点
    target->set_dominator_tree_info_valid(false);
    for (auto& nloop : nloop_info) {
        auto header = nloop.header;
        //创建新节点
        auto preheader = new IRBlock();
        preheader->set_tag(++dfn_count);
        idfn.push_back(preheader);//新加入的节点不再遵守dfs序，但依然用idfn数组维护其编号
        nloop.preheader = preheader;
        auto pred_nodes = header->get_pred();
        for (auto pred : pred_nodes) {
            //如果不是回边，则将边的终点转移到新节点
            if (nloop.back_edge_list.find(pred)==nloop.back_edge_list.end()) {
                for(int k=0;k<=1;++k)
                    if (pred->get_succ(k) == header) {
                        pred->delete_edge(k);
                        pred->set_edge(k, preheader);
                    }
            }
        }
        //加入前置节点到头结点的边
        preheader->set_edge(0, header);
        //加入无条件跳转指令       (不需要）
        //preheader->add_instr(IRInstr::create_block_goto());
    }
    //5、根据首节点和回边信息找出自然循环中所有点的集合
    for (auto& nloop : nloop_info) {
        std::set<int> in_loop;
        std::stack<int> s;
        in_loop.insert(nloop.header->get_tag());  
        for (auto back_node : nloop.back_edge_list) {
            in_loop.insert(back_node->get_tag());
            s.push(back_node->get_tag());
        }
        while (!s.empty()){
            auto now = s.top();
            nloop.node_set.insert(idfn[now]);
            s.pop();
            if (now == nloop.header->get_tag())
                continue;
            for (auto pred : idfn[now]->get_pred()) {
                if (in_loop.find(pred->get_tag()) == in_loop.end()) {
                    in_loop.insert(pred->get_tag());
                    s.push(pred->get_tag());
                }
            }
        }
    }
    //6、标记每个nloop的所有出口节点
    for (auto& nloop : nloop_info) {
        for (auto node : nloop.node_set) {
            for (int k = 0; k < node->out_degree(); ++k) {
                if (nloop.node_set.find(node->get_succ(k)) == nloop.node_set.end()) {
                    nloop.exit_set.insert(node);
                    break;
                }
            }
        }
    }
    return nloop_info;
}

/*
*   找出所有自然循环，不增加空基本块
*   要求不含有不可达基本块
*/
std::vector<NaturalLoopInfo> CFGManager::find_natural_loop(IRUnit* target)
{
    std::vector<NaturalLoopInfo> nloop_info;
    std::map<int, int> head_to_nloop;   //从头结点映射到对应循环
    int dfn_count = -1;
    std::vector<IRBlock*> idfn;         //逆dfn，从dfn映射到IRblock
    std::vector<int> dfn_postorder;     //记录递归返回时的dfn
    //1、对目标单元求出支配树信息
    if (!target->is_dominator_tree_info_valid())
        build_dominator_tree(target);
    //2、支配树上dfs，维护访问序和离开序（子树信息）
    find_natural_loop_dfs(target->get_entry(),idfn,dfn_postorder,dfn_count);
    //3、按照dfs序遍历BB，维护回边，以及循环首结点集合（无重复）
    for (int i = 0; i <= dfn_count; ++i) {
        IRBlock* block = idfn[i];
        for (int index = 0; index < block->out_degree(); ++index) {
            auto child = block->get_succ(index);
            assert(child != nullptr);
            //检测回边
            if (is_dominate(child->get_tag(),i, dfn_postorder[child->get_tag()], dfn_postorder[i])) {
                //在nloop_info中维护回边集合，（合并所有头结点相同的自然循环）
                if (auto iter= head_to_nloop.find(child->get_tag());iter!=head_to_nloop.end()) {
                    //如果头结点已经被加入集合，则给对应自然循环新增一个回边
                    nloop_info[iter->second].back_edge_list.insert(block);
                } else {
                    //如果头结点没被加入集合，则加入集合，并新增一个自然循环
                    nloop_info.push_back(NaturalLoopInfo(child));
                    nloop_info.back().back_edge_list.insert(block);
                    nloop_info.back().node_set.insert(child);
                    head_to_nloop[child->get_tag()] = nloop_info.size() - 1;
                }
            }
        }
    }
    //5、根据首节点和回边信息找出自然循环中所有点的集合
    for (auto& nloop : nloop_info) {
        std::set<int> in_loop;
        std::stack<int> s;
        in_loop.insert(nloop.header->get_tag());  
        for (auto back_node : nloop.back_edge_list) {
            in_loop.insert(back_node->get_tag());
            s.push(back_node->get_tag());
        }
        while (!s.empty()){
            auto now = s.top();
            nloop.node_set.insert(idfn[now]);
            s.pop();
            if (now == nloop.header->get_tag())
                continue;
            for (auto pred : idfn[now]->get_pred()) {
                if (in_loop.find(pred->get_tag()) == in_loop.end()) {
                    in_loop.insert(pred->get_tag());
                    s.push(pred->get_tag());
                }
            }
        }
    }
    //6、标记每个nloop的所有出口节点
    for (auto& nloop : nloop_info) {
        for (auto node : nloop.node_set) {
            for (int k = 0; k < node->out_degree(); ++k) {
                if (nloop.node_set.find(node->get_succ(k)) == nloop.node_set.end()) {
                    nloop.exit_set.insert(node);
                    break;
                }
            }
        }
    }
    return nloop_info;
}

//根据进入时和离开时的dfs序判断A是否支配了B。(可以支配自身)
bool CFGManager::is_dominate(int pre_a,int pre_b,int post_a,int post_b) {
    return pre_a <= pre_b && post_b <= post_a;
}
void CFGManager::find_natural_loop_dfs(
    IRBlock* block,
    std::vector<IRBlock*>& idfn,
    std::vector<int>& dfn_postorder,
    int& dfn_count)
{
    //std::cout << block->get_index() << std::endl;
    block->set_tag(++dfn_count);
    idfn.push_back(block);
    dfn_postorder.push_back(0);
    for (auto child : block->get_idom_child()) {
        //std::cout << "in" << std::endl;
        find_natural_loop_dfs(child, idfn, dfn_postorder, dfn_count);
        //std::cout << "out" << std::endl;
    }
    dfn_postorder[block->get_tag()] = dfn_count;
}

/*
* dfs设置所有block的tag为0
*/
static void loop_depth_analysis_for_blocks_dfs(IRBlock* block)
{
    block->set_tag(0);
    for (auto child : block->get_idom_child())
        loop_depth_analysis_for_blocks_dfs(child);
}
void CFGManager::loop_depth_analysis_for_blocks(IRUnit* unit)
{
    auto nloops = find_natural_loop(unit);
    loop_depth_analysis_for_blocks_dfs(unit->get_entry());
    for (auto& nloop : nloops)
        for (auto& node : nloop.node_set)
            node->set_tag(node->get_tag() + 1);
}