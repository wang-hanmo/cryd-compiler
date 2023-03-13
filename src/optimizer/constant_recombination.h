#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
/*
    常数重结合优化

    tag使用情况：
    l、p变量混合编码,数组不考虑
    t变量单独编码
    g变量单独编码

    依赖特性：
    按照bfs遍历基本块时，所有temp变量的定值一定在使用前出现

*/
//符号，定义语句对
namespace constant_recombination {
    struct SymDefPair
    {
        IRSymbol* sym{ nullptr };
        IRInstr* def{ nullptr };
        SymDefPair() {}
        SymDefPair(IRSymbol* sym_, IRInstr* def_) :sym(sym_), def(def_) {};
        SymDefPair(IRInstr* def_) :def(def_) { sym = def_->r(); };
        SymDefPair(IRSymbol* sym_) :sym(sym_) {};
        bool operator<(const SymDefPair& rhs)const
        {
            return sym < rhs.sym;
        }
    };
    //表达式
    //a+b
    //a-b
    //a*b
    //a/b
    struct Expr
    {
        IRSymbol* a{ nullptr };
        IRSymbol* b{ nullptr };
        IROper op{IROper::Null};
        Expr() {};
        Expr(IRSymbol* a_, IRSymbol* b_, IROper op_) :a(a_),b(b_), op(op_) {}
    };
    //得到自然溢出结果的加法
    int natural_add(int a, int b);
    int natural_sub(int a, int b);
}
class ConstantRecombination final : public Pass
{
private:
    void delete_related_instr(IRSymbol* sym);
    void work_cfg(IRUnit* unit);
    void work_ir_list(IRInstrList& program);
    void work_assign(IRInstr* instr);
    void work_unary_calc(IRInstr* instr);
    void work_binary_calc(IRInstr* instr);
    void check_insert(IRInstr* instr);
    
    void check_merge(IRInstr* instr);//检查本语句是否能和有效语句集里的某一条语句合并
    std::pair<bool, IRInstr> try_merge(IRInstr* prev,IRInstr* now);//检查now是否能和prev合并
    //从指令生成标准表达式
    std::vector<constant_recombination::Expr> gen_expr(IRInstr* instr);
    //代数化简
    void simplify_instr(IRInstr& instr);
    void init();
    std::vector<std::vector<IRSymbol*>> m_related_sym_glb;  //g变量->与之关联（在其定值语句中被使用过）的t变量
    std::vector<std::vector<IRSymbol*>> m_related_sym;      //lp变量->与之关联（在其定值语句中被使用过）的t变量
    std::set<constant_recombination::SymDefPair> m_sym_def_set;                 //当前有效的(t变量，定值指令)对集合
    //std::set<constant_recombination::SymDefPair> m_sym_def_set_glb;             //同上，但考虑定值中有g变量的情况
    bool symbol_equal(IRSymbol* a, IRSymbol* b);
    IRSymbol* symbol_copy(IRSymbol* a);
    int m_temp_count;
    int m_local_param_count;
    //int m_global_count;
    bool is_non_pointer_temp_var(IRSymbol* sym);        //是否为非指针型temp变量
    bool is_not_considered_type(IRSymbol* sym);         //是否为不考虑进行替代的变量（数组、指针）
public:
    ConstantRecombination(bool i_emit) :Pass(PassType::ConstantRecombination, PassTarget::CFG, i_emit) {}
    void run();
};