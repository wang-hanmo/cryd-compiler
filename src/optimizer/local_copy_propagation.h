#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
namespace local_copy_propagation
{
    class CopyExpression
    {
    private:
        IRSymbol* m_a, * m_r;
    public:
        CopyExpression(IRSymbol* i_a, IRSymbol* i_r) :m_a(i_a), m_r(i_r) {}
        IRSymbol* get_a()const { return m_a; }
        IRSymbol* get_r()const { return m_r; }
        bool less_than(IRSymbol* lhs, IRSymbol* rhs)const {
            if (lhs == nullptr && rhs == nullptr)
                return false;
            else if (lhs == nullptr)
                return true;
            else if (rhs == nullptr)
                return false;
            if (lhs->kind() != rhs->kind())
                return (int)lhs->kind() < (int)rhs->kind();
            if (lhs->kind() == IRSymbolKind::Value)
                return lhs->value().int_value < rhs->value().int_value;
            else return lhs < rhs;
        }
        bool equal_to(IRSymbol* lhs, IRSymbol* rhs)const {
            if (lhs->kind() != rhs->kind())
                return false;
            if (lhs->kind() == IRSymbolKind::Value)
                assert(false);
            else return lhs == rhs;
        }
        //按照r的(任意)顺序排序，不考虑a
        bool operator <(const CopyExpression& rhs)const {
            return less_than(m_r, rhs.m_r);
        }
    };
}
class LocalCopyPropagation final: public Pass
{
private:
    IRSymbol* check_replace(IRSymbol* sym);
    void work_cfg(IRUnit* unit);
    void work_ir_list(IRInstrList& program, int unit_tag);
    std::set<local_copy_propagation::CopyExpression> m_expr_set;
    int m_sym_count = 0;
    std::vector<std::vector<IRSymbol*>> m_replaced_vars;//普通型变量编号为索引，保存哪些变量现在被本变量的某个具体定值替代了
public:
    LocalCopyPropagation(bool i_emit) :Pass(PassType::LocalCopyPropagation,PassTarget::CFG, i_emit) {}
    void run();
};