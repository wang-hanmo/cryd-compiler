#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <map>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
#include <local_common_subexpression_elimination.h>
//使用全局值编号的方法提取公共子表达式。（不考虑全局变量）
//由于SSA转回原形式写得较为简陋，凡是遇到的phi函数均会给予一个新的值编号
namespace gcse
{
    struct CallExpression 
    {
        std::vector<int> vn_rparam{ {} };
        int vn_func{-1};
        CallExpression() {}
        CallExpression(int vn_func_, std::vector<int>&& vn_rparam_) {
            vn_rparam = vn_rparam_;
            vn_func = vn_func_;
        }
        bool operator<(const CallExpression& rhs)const {
            if (vn_func != rhs.vn_func)
                return vn_func < rhs.vn_func;
            assert(vn_rparam.size() == rhs.vn_rparam.size());
            const size_t size = vn_rparam.size();
            for (size_t i = 0; i < size; ++i) {
                if (vn_rparam[i] != rhs.vn_rparam[i])
                    return vn_rparam[i] < rhs.vn_rparam[i];
            }
            return false;
        }
    };

}
class GlobalCommonSubexpressionElimination final: public Pass
{
private:
    std::map<IRSymbol*, int, lcse::SymbolComparer> m_vn_of_sym; //符号的值编号
    std::map<gcse::CallExpression, int> m_vn_of_call;           //函数调用的值编号
    std::map<lcse::Expression, int> m_vn_of_expr;               //表达式的值编号
    std::map<int,IRSymbol*> m_vn_to_sym;                   //某个值编号的替代符号
    int m_glb_var_count;
    int m_vn;
    int find_or_new_vn(IRSymbol* sym);
    int find_vn(IRSymbol* sym);
    int new_vn(IRSymbol* sym);
    //获取函数调用表达式，first表示是否成功获取（函数有副作用或者参数含全局变量则不成功）
    std::pair<bool, gcse::CallExpression> get_call_expression(const std::vector<IRSymbol*>& params,IRSymbol* func);
    void work(IRBlock* block);
    void work_block(IRInstrList& program);
    void roll_back(IRInstrList& program);
    void init();//初始化全局非数组变量编号
public:
    GlobalCommonSubexpressionElimination(bool i_emit) :Pass(PassType::GlobalCommonSubexpressionElimination,PassTarget::CFG, i_emit) {}
    void run();
};