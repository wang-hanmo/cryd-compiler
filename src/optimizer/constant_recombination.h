#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
/*
    �����ؽ���Ż�

    tagʹ�������
    l��p������ϱ���,���鲻����
    t������������
    g������������

    �������ԣ�
    ����bfs����������ʱ������temp�����Ķ�ֵһ����ʹ��ǰ����

*/
//���ţ���������
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
    //���ʽ
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
    //�õ���Ȼ�������ļӷ�
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
    
    void check_merge(IRInstr* instr);//��鱾����Ƿ��ܺ���Ч��伯���ĳһ�����ϲ�
    std::pair<bool, IRInstr> try_merge(IRInstr* prev,IRInstr* now);//���now�Ƿ��ܺ�prev�ϲ�
    //��ָ�����ɱ�׼���ʽ
    std::vector<constant_recombination::Expr> gen_expr(IRInstr* instr);
    //��������
    void simplify_instr(IRInstr& instr);
    void init();
    std::vector<std::vector<IRSymbol*>> m_related_sym_glb;  //g����->��֮���������䶨ֵ����б�ʹ�ù�����t����
    std::vector<std::vector<IRSymbol*>> m_related_sym;      //lp����->��֮���������䶨ֵ����б�ʹ�ù�����t����
    std::set<constant_recombination::SymDefPair> m_sym_def_set;                 //��ǰ��Ч��(t��������ֵָ��)�Լ���
    //std::set<constant_recombination::SymDefPair> m_sym_def_set_glb;             //ͬ�ϣ������Ƕ�ֵ����g���������
    bool symbol_equal(IRSymbol* a, IRSymbol* b);
    IRSymbol* symbol_copy(IRSymbol* a);
    int m_temp_count;
    int m_local_param_count;
    //int m_global_count;
    bool is_non_pointer_temp_var(IRSymbol* sym);        //�Ƿ�Ϊ��ָ����temp����
    bool is_not_considered_type(IRSymbol* sym);         //�Ƿ�Ϊ�����ǽ�������ı��������顢ָ�룩
public:
    ConstantRecombination(bool i_emit) :Pass(PassType::ConstantRecombination, PassTarget::CFG, i_emit) {}
    void run();
};