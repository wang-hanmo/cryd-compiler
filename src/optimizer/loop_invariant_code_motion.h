#pragma once
#include <optimizer.h>
#include <cfg_manager.h>
#include <bitmap.h>
#include <unordered_set>
/*
* tag使用情况：
* IRSymbol的tag： 存放其编号
* IRInstr的no：   存放其全编号
* IRBlock的tag：  存放其编号，按照bfs序
*/
class LoopInvariantCodeMotion final : public Pass
{
private:
    //循环分析（包含关系树）
    std::vector<std::vector<int>> m_sub_loop;
    //循环不变指令编号
    int m_invariant_count;
    //自然循环数量
    int m_nloop_count;
    //到达定值分析
    //为了提高效率，定值指令编号不对temp进行编号
    int m_blk_cnt = 0;          //基本块数量
    int m_instr_cnt = 0;        //指令数量
    int m_def_instr_cnt = 0;    //对local/param型非数组变量进行定值的指令数量
    int m_sym_cnt = 0;          //local/param/temp符号数量
    std::vector<int> m_idx_to_def;          //指令全编号->定值指令编号
    std::vector<IRInstr*> m_def_to_instr;   //定值指令编号->指令
    std::vector<IRBlock*> m_instr_to_blk;   //指令编号->指令所在块
    std::vector<IRBlock*> m_ibfn;           //块编号->块
    std::vector<int>    m_dominated_exit_count;//块编号->支配树上有多少个出口块在该块的子树中
    std::vector<BitMap> m_gen;  //生成定值向量，block->定值指令向量
    std::vector<BitMap> m_prsv; //保留定值向量，block->定值指令向量
    std::vector<BitMap> m_rin;  //基本块入口有效定值向量，block->定值指令向量
    std::vector<BitMap> m_rout; //基本块出口有效定值向量，block->定值指令向量
    std::vector<int> m_sym_to_the_only_def; //符号->循环内支配了所有使用的唯一定值指令的全编号。如果不满足前述条件，则值被设为-1
    //std::vector<BitMap> m_duchain; //定值语句到引用所在块的du链
    std::vector<std::vector<int>> m_sym_to_def_instr; //符号->所有定值指令的全编号(不考虑是否可到达)
    std::vector<std::vector<int>> m_udchain_a; //操作数a的ud链，指令->指令列表(全编号）
    std::vector<std::vector<int>> m_udchain_b; //操作数b的ud链，指令->指令列表(全编号)
    std::vector<int> m_invariant;              //指令->循环不变标识。按照指令被标注为循环不变量的顺序编号，0表示非循环不变量。
    void work_unit(IRUnit* unit);
    void init(IRUnit* unit);//维护指令全编号、定值指令编号、Block编号、Block编号到块的映射。初始化所有数据流分析用到的数据结构
    void solve_gen_prsv(IRUnit* unit);//求解每个block内的gen和prsv
    void solve_rin_rout();//迭代求解reach in和reach out
    void solve_ud_duchain(); //求解每一条指令中每一个符号的ud链
    //设定一个变量的ud链，并维护du链
    void set_ud_duchain(IRSymbol* sym,int belonging_to_blk, std::vector<int>& udchain_vec, BitMap& working_bitmap);
    bool is_numbered_def_instr(const IRInstr& instr);//是否为对local/param型非数组变量进行定值的语句
    bool has_index(IRSymbol* sym);//是否被编了号（必须是非数组的local/param型，或者是指针/非数组的temp型）
    //支配树上dfs求节点是否支配了所有出口
    void dfs_on_dominator_tree(NaturalLoopInfo& nloop,IRBlock* blk);
    //求一个nloop内对变量的所有使用是否来自同一个定值，且这个定值也在nloop内。以及是否对变量定值了多次。
    void calculate_dominate_all_use(NaturalLoopInfo& nloop);
    //循环的包含关系分析
    void inclusion_analysis_for_loops(std::vector< NaturalLoopInfo>& nloops);
    //在循环包含树上进行dfs分析，包含标注循环不变式和循环不变式的外提
    void analysis_inclusion_tree(std::vector<NaturalLoopInfo>& nloops, int current_index);
    void mark_invariant_code(NaturalLoopInfo& nloops);  //标注循环不变式
    bool mark_block(NaturalLoopInfo& nloops,IRBlock* block);  //对基本块标注循环不变式，返回是否增加新标注
    void move_invariant_code(NaturalLoopInfo& nloops);  //循环不变式外提
    //检测一个引用是否是循环不变的
    bool check_invariant(IRSymbol* sym,std::vector<int>aviliable_defs, std::set<IRBlock*>& loop_blk_set);
public:
    LoopInvariantCodeMotion(bool i_emit) :Pass(PassType::LoopInvariantCodeMotion, PassTarget::CFG, i_emit) {}
    void run();
};