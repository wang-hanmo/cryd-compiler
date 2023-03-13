#pragma once
#include <ir_instr.h>
#include <vector>
#include <array>
#include <list>
#include <iostream>
#include <cfg.h>
#include <set>
//建立支配树时使用的信息
struct BuildDomTreeInfo
{
    IRBlock* idfn{nullptr};
    //以下“节点”均指节点的dfn
    int idom{-1};              //支配节点
    int dfs_tree_parent{-1};   //dfs生成树上的亲节点
    int ufs_min{-1};           //带权并查集的权值，维护了该节点到根的链上所有节点中sdom的dfn最小的节点
                               //在实际算法处理过程中，由于按照dfn逆序访问图，每次用到这个变量时其维护的链不会延伸到根，而只包含dfn大于当前正处理节点的那些节点
    int ufs_parent{-1};        //带权并查集的亲节点
    int sdom{-1};              //半支配点(半支配树上的亲节点)
    std::vector<int> sdom_child{};//半支配树上的子节点
    std::vector<int> cfg_pred{};//控制流图的反向图上的子节点
};
//自然循环相关信息
struct NaturalLoopInfo
{
    IRBlock* header{ nullptr };
    IRBlock* preheader{ nullptr };
    std::set<IRBlock*> exit_set;          //出口节点集合
    std::set<IRBlock*> node_set;          //循环包含节点的集合
    std::set<IRBlock*> back_edge_list;    //维护指向头结点的节点集合
    int tag{ 0 };
    NaturalLoopInfo() {}
    NaturalLoopInfo(IRBlock* header_):header(header_) {}
    bool has_node(IRBlock* blk) {
        return node_set.find(blk) != node_set.end();
    }
    size_t exit_node_count() {
        return exit_set.size();
    }
};
/*
*   包含线性IR转控制流图、以及控制流分析、数据流分析的通用算法
*/
class CFGManager
{
private:
    static void print_control_flow_graph(IRBlock* entry,std::ostream& os);
    static void ir_to_cfg_error(const std::string &error_msg,/*const IRProgram& program_part,*/ int error_code);
    //建立支配树的子程序
    //第一步，初始化必要的信息，求出每个节点的dfn
    static void build_dom_tree_init(IRBlock* entry,int dfn_parent, std::vector<BuildDomTreeInfo>& node,std::set<IRBlock*>& visited);
    //第二步，求解支配节点
    static void build_dom_tree_solve(IRBlock* entry, std::vector<BuildDomTreeInfo>& node);
    //辅助函数,带权并查集find操作,第一个参数表示要查找节点的dfn
    static int build_dom_tree_ufs_find(int dfn_x, std::vector<BuildDomTreeInfo>& node);
    //找出所有自然循环的子程序
    //tag用于给block编号
    static void find_natural_loop_dfs(IRBlock* block,std::vector<IRBlock*>& idfn,std::vector<int>& dfn_postorder,int& dfn_count);
    static bool is_dominate(int pre_a, int pre_b, int post_a, int post_b);//A是否支配了B
   
public:
    //static int get_block_count(IRBlock* entry, const std::vector<IRBlock*>& idfn);//求出控制流图的节点总数
    static IRProgram* gen_from_ast(ASTNode* root);
    static IRProgram* gen_from_linear_ir(const IRInstrList& list);
    static void build_dominator_tree(IRUnit* target);//对支配树信息未求解/失效的单元求解支配树，如果信息有效就什么也不做
    //找出所有自然循环,并给循环首节点前增加一个空基本块用于循环不变式外提
    static std::vector<NaturalLoopInfo> find_natural_loop_with_preheader(IRUnit* target);
    static std::vector<NaturalLoopInfo> find_natural_loop(IRUnit* target);//找出所有自然循环，不增加空基本块
    //基本块所在循环深度分析,结果保存在tag中
    static void loop_depth_analysis_for_blocks(IRUnit* unit);
    static void print_ir_block(IRBlock* block,std::ostream& os);
    static void print_ir_unit(const IRUnit& unit,std::ostream& os);
    static void print_ir_program(const IRProgram& prog,std::ostream& os=std::cout);
};
