#pragma once
#include <ir_instr.h>
#include <optimizer.h>
#include <cfg.h>
#include <cassert>
namespace sccp 
{
    //常量传播格的值类型
    enum class ConstLatValue :int 
    {
        Uncertain = 0,  //未确定的
        Const = 1,      //某常数
        Var = 2         //变量
    };
    //常量传播格
    struct ConstLat
    {
        union {
            int int_value{ 0 };                             //常数值-整数
            float float_value;                              //常数值-浮点数
        };
        BasicType basic_type{ BasicType::Uncertain };         //基本值类型,只在lat_value为Const时有效
        ConstLatValue lat_value{ ConstLatValue::Uncertain };  //格元素类型
        BasicValue basic_value()const
        {
            if (basic_type == BasicType::Int)
                return int_value;
            return float_value;
        }
        ConstLat() {}
        ConstLat(ConstLatValue lat_value_, BasicType basic_type_) :lat_value(lat_value_), basic_type(basic_type_) {}
        //格的相等运算
        bool operator ==(const ConstLat& rhs)const
        {
            if (lat_value != rhs.lat_value)
                return false;
            if (lat_value == ConstLatValue::Const) {
                if (basic_type == BasicType::Int)
                    return int_value == rhs.int_value;
                else return float_value == rhs.float_value;
            }
            return true;
        }
        bool operator !=(const ConstLat& rhs)const
        {
            return !(*this == rhs);
        }

        static ConstLat lat_eval_binaryop(const IROper op, const ConstLat& lhs, const ConstLat& rhs);
        static ConstLat lat_eval_unaryop(const IROper op, const ConstLat& oprand);
        //格的交运算
        void intersect_with(const ConstLat& rhs);

    };
    /*
    *  从指令到指令的边，用于记录虚拟流图边（虚拟流图假设每条指令独立成一个block）和SSA边（定义指向所有引用）
    */
    struct InstrEdge
    {
        int from{ false };          //有向边起点
        int to{ false };            //有向边终点
        bool is_true_edge{ false }; //仅对条件跳转指令有效，表示此边为True边还是False边
        bool exsc_flag{ false };    //是否可执行
        InstrEdge() {}
        InstrEdge(int from_, int to_, bool is_true_edge_ = false) :from(from_), to(to_), is_true_edge(is_true_edge_) {}
    };
}
/*
*  稀疏条件常量传播 
*  依赖性质：
*  IR是SSA形式
*/
class SparseConditionalConstantPropagation final : public Pass
{
private:
    int m_instr_count;                                          //指令个数
    int m_sym_count;                                            //SSA符号个数
    std::vector<std::list<sccp::InstrEdge>> m_ssa_edge_list;          //SSA边表
    std::vector<std::list<sccp::InstrEdge>> m_cfg_edge_list;          //CFG边表
    std::vector<std::vector<sccp::InstrEdge*>> m_anti_cfg_edge_list;  //反向CFG边表
    std::queue<sccp::InstrEdge*> m_ssa_work_list;                     //SSA工作边表
    std::queue<sccp::InstrEdge*> m_cfg_work_list;                     //CFG工作边表
    std::vector<int> m_sym_to_def_instr;                        //符号到其定值指令编号的映射
    std::vector<IRInstr*> m_i_instr;                            //指令编号到指令(即虚拟BB)的映射
    std::vector<IRBlock*> m_instr_to_block;                     //指令编号到真实BasicBlock的映射
    std::vector<sccp::ConstLat> m_lat_cell;                           //通过symbol的index(存在tag中)来索引格元素
    bool is_ssa_var(IRSymbol* sym);                             //是否为SSA形式的变量
    bool has_lvalue(const IRInstr& instr);                      //是否含有SSA形式的左值
    std::vector<IRSymbol*> rvalue_list(const IRInstr& instr);   //取出所有SSA形式的右值
    
    void work_unit(IRUnit& unit);                     //顶层-对IRUnit执行常量传播
    void initialize_instr_index(IRUnit& unit);        //1 初始化指令、符号的编号，并维护符号到定值指令的映射。
    void initialize_working_environment(IRUnit& unit);//2 构建最小流图结构（每条指令一个BB)，CFG、SSA工作边表，lat cell的初始值。
    void initialize_block(IRBlock* block);            //2-1 basic block级的初始化,index表示指令编号
    int get_next_instr_index(IRBlock* block);         //2-1-1 获取指定block中第一条指令的编号（如果为空，继续从后续block里找）。
    void visit_phi(IRInstr* instr);                   //3-1 phi函数上常量传播
    void visit_instr(IRInstr* instr);                 //3-2 其他指令上常量传播
    int edge_count(int to);                           //3-3 返回以to为终点，并且可执行的边数
    sccp::ConstLat lat_eval(IRInstr* instr);                //3-4 使用格值执行指令
    sccp::ConstLat get_lat_val(IRSymbol* sym);              //3-4-1 获得符号的格值
    void rewrite_instructions(IRUnit& unit);          //4 指令重写
public:
    SparseConditionalConstantPropagation(bool i_emit) :Pass(PassType::SparseConditionalConstantPropagation,PassTarget::CFG, i_emit) {}
    void run();
};