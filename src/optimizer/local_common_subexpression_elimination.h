#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <map>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
namespace lcse {
    struct Expression
    {
        IROper op{IROper::Null};
        int vn_a{-1};
        int vn_b{-1};
        Expression() {}
        Expression(int vn_a_,int vn_b_,IROper op_) {
            if (op_ == IROper::AddI || op_ == IROper::AddF || 
                op_ == IROper::SubI || op_ == IROper::SubF || 
                op_ == IROper::EqualI || op_ == IROper::EqualF) {
                if (vn_a_ > vn_b_)
                    std::swap(vn_a_, vn_b_);
            }
            vn_a = vn_a_;
            vn_b = vn_b_;
            op = op_;
        }
        bool operator<(const Expression& rhs)const {
            if (vn_a != rhs.vn_a)
                return vn_a < rhs.vn_a;
            if (vn_b != rhs.vn_b)
                return vn_b < rhs.vn_b;
            return (int)op < (int)rhs.op;
        }
    };
    struct SymbolComparer {
        bool operator()(IRSymbol* lhs, IRSymbol* rhs)const {
            if (lhs == nullptr && rhs == nullptr)
                return false;
            if (lhs == nullptr)
                return true;
            if (rhs == nullptr)
                return false;
            if (lhs->kind() != rhs->kind())
                return lhs->kind() < rhs->kind();
            if (lhs->basic_type() != rhs->basic_type())
                return lhs->basic_type() < rhs->basic_type();
            if (lhs->kind() == IRSymbolKind::Value)
                return lhs->int_value() < rhs->int_value();
            return lhs < rhs;
        }
    };
}
//使用局部值编号的方法提取公共子表达式
//考虑全局变量
class LocalCommonSubexpressionElimination final: public Pass
{
private:

    std::map<IRSymbol*, int, lcse::SymbolComparer> m_vn_of_sym; //符号的值编号
    std::map<lcse::Expression, int> m_vn_of_expr;               //表达式的值编号
    std::map<int,IRSymbol*> m_vn_to_sym;                   //某个值编号的替代符号
    int m_glb_var_count;
    int m_vn;
    int find_or_new_vn(IRSymbol* sym);
    void control_flow_graph_eliminate(IRBlock* entry);
    void ir_list_eliminate(IRInstrList& program);
    void init();//初始化全局非数组变量编号
public:
    LocalCommonSubexpressionElimination(bool i_emit) :Pass(PassType::LocalCommonSubexpressionElimination,PassTarget::CFG, i_emit) {}
    void run();
};