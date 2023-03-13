#pragma once
#include <optimizer.h>
#include <../ir/cfg.h>
#include <map>
#include <local_common_subexpression_elimination.h>
using MemId = int;
using MemVer = int;
using BBId = int;
using InstrId = int;
namespace gavs
{
    struct ArrayVisitExpression
    {
        int vn_array{-1};
        int version{-1};
        int vn_index{-1};
        ArrayVisitExpression() {}
        ArrayVisitExpression(int vn_array_, int version_,int vn_index_):
        vn_array(vn_array_), version(version_),vn_index(vn_index_){}
        bool operator<(const ArrayVisitExpression& rhs)const {
            if (vn_array != rhs.vn_array)
                return vn_array < rhs.vn_array;
            if (version != rhs.version)
                return version < rhs.version;
            return vn_index< rhs.vn_index;
        }
    };

}
/*
*   对数组做标量化分析，并进行冗余数组访问提取
*/
class GlobalArrayVisitSubstitution final :public Pass
{
private:
    /*
    1、给每一块memory编号，维护从数组型变量到memory编号的映射
	    ②对所有全局、局部型数组变量对应的memory进行编号
	    ③发现函数有指针型参数，则不进行分析。
    */
    //全局数组的数量
    MemId m_global_array_conut;
    //全局数组+当前处理函数的数组总数
    MemId m_array_count;
    //指令数
    InstrId m_instr_count;
    //Memory编号 Symbol id -> Memory id
    //std::vector<SymId> m_memid;
    //Memory的最大版本编号，不断递增 Memory id -> Memory ver
    std::vector<MemVer> m_max_memver;
    //指令中的Memory版本编号 Instr id -> Memory ver
    std::vector<MemVer> m_memver_of_instr;
    //Memory是否参与分析 Memory id -> bool
    std::vector<bool> m_mem_should_analyze;
    //Memory id -> symbol
    std::vector<IRSymbol*> m_mem_def;
    /*
    2、对每块memory，计算对其定值的基本块集合的迭代支配边界
    3、在迭代支配边界处插入该块memory被修改的标记
    4、对每条ArrayStore语句，维护memory的版本编号
        对memory进行修改，则版本号+1，修改语句包括array store,call,call with return
    */
    std::vector<BitMap> m_dominance_frontier;           //使用bitmap保存当前基本块的支配边界,使用支配树上的dfn作为索引
    std::vector<IRBlock*> m_idfn;                       //inverse dfn,通过dfn反向索引节点  BBId -> BB
    std::vector<BitMap> m_mem_modified_block;           //memory被修改过的基本块             MemId -> BB List
    std::vector<BitMap> m_mem_dominance_frontiers_plus; //memory的迭代支配边界,tag索引       MemId -> BB List
    //SSA形式下的版本号栈，list.back()维护了栈顶元素，栈顶元素表示当前最新的版本号
    std::vector<std::list<MemVer>> m_current_memver;
    //全局初始化
    void global_init();
    //构建数组标量化后的SSA形式
    bool build_ssa(IRUnit* unit);
    //对支配树dfs，维护idfn
    void compute_idfn(IRBlock* entry);
    void compute_dominance_frontiers();
    //记录memory被修改的基本块集合
    void record_modified_block(IRUnit* unit);
    //计算迭代支配边界
    void compute_dfp_for_all_vars();
    BitMap compute_dominance_frontiers_plus(const BitMap& set);
    BitMap compute_dominance_frontiers_set(const BitMap& set);
    //在基本块开头插入由于节点汇合导致的版本更新标记
    void insert_memory_converge_mark();
    //标记每个对数组访问对应的数组版本号
    int mark_memver_modify(IRSymbol* sym);
    int mark_memver_use(IRSymbol* sym);
    void mark_memver_in_block(IRBlock* entry);
    void mark_memver_in_defination_area(IRUnit* unit);
    //做全局值编号，删除冗余数组访问
    std::map<IRSymbol*, int, lcse::SymbolComparer> m_vn_of_sym;     //符号的值编号
    std::map<gavs::ArrayVisitExpression, int> m_vn_of_array_visit;  //函数调用的值编号
    std::map<int, IRSymbol*> m_vn_to_sym;                           //某个值编号的替代符号
    int m_vn;//当前最大的值编号
    int find_or_new_vn(IRSymbol* sym);
    int find_vn(IRSymbol* sym);
    int new_vn(IRSymbol* sym);
    void work_vn_block(IRInstrList& program);
    void roll_back(IRInstrList& program);
    void work_vn(IRBlock* block);
    //删除每个基本块开头的标记
    void delete_mark(IRUnit* unit);
public:
    GlobalArrayVisitSubstitution(bool i_emit) :Pass(PassType::GlobalArrayVisitSubstitution,PassTarget::CFG, i_emit) {}
    void run();
};