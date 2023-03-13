#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
/*
    ָ���ں��Ż�
    ��Ҫ��Գ��ۼ�ָ��

    tagʹ�������
    l��p������ϱ���,���鲻����
    t������������
    g������������

    �������ԣ�
    ����bfs����������ʱ������temp�����Ķ�ֵһ����ʹ��ǰ����

    ������
    *�����Ը�������������������
*/
//���ţ���������
namespace instruction_combination {
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
    //���ۼӱ��ʽ
    //a+b*c
    struct MulAddExpr
    {
        IRSymbol* a{ nullptr };
        IRSymbol* b{ nullptr };
        IRSymbol* c{ nullptr };
        MulAddExpr() {};
        MulAddExpr(IRSymbol* a_, IRSymbol* b_, IRSymbol* c_) :a(a_),b(b_),c(c_) {}
    };
}
class InstructionCombination final : public Pass
{
private:
    void delete_related_instr(IRSymbol* sym);
    void work_cfg(IRUnit* unit);
    void work_ir_list(IRInstrList& program);
    void work_assign(IRInstr* instr);
    void work_unary_calc(IRInstr* instr);
    void work_binary_calc(IRInstr* instr);
    void work_ternary_calc(IRInstr* instr);
    void check_insert(IRInstr* instr);
    void check_merge(IRInstr* instr);//��鱾����Ƿ��ܺ���Ч��伯���ĳһ�����ϲ�
    std::pair<bool, IRInstr> try_merge(IRInstr* prev,IRInstr* now);//���now�Ƿ��ܺ�prev�ϲ�
    //��ָ�����ɱ�׼���ۼ�ʽ
    std::vector<instruction_combination::MulAddExpr> gen_mul_add_expr(IRInstr* instr);
    //�������ۼ�ָ��
    void simplify_mul_add_instr(IRInstr& instr);
    void simplify_binary_calc_instr(IRInstr& instr);
    void init();
    void get_use_info_for_temp_vars(IRUnit* unit);      //����t�����Ķ�ֵʹ����Ϣ
    std::vector<int> m_use_count;                       //t�������->ʹ�ô���
    std::vector<std::vector<IRSymbol*>> m_related_sym_glb;  //g����->��֮���������䶨ֵ����б�ʹ�ù�����t����
    std::vector<std::vector<IRSymbol*>> m_related_sym;      //lp����->��֮���������䶨ֵ����б�ʹ�ù�����t����
    std::set<instruction_combination::SymDefPair> m_sym_def_set;                 //��ǰ��Ч��(t��������ֵָ��)�Լ���
    std::set<instruction_combination::SymDefPair> m_sym_def_set_glb;             //ͬ�ϣ������Ƕ�ֵ����g���������
    bool symbol_equal(IRSymbol* a, IRSymbol* b);
    IRSymbol* symbol_copy(IRSymbol* a);
    int m_temp_count;
    int m_local_param_count;
    int m_global_count;
    bool is_non_pointer_temp_var(IRSymbol* sym);        //�Ƿ�Ϊ��ָ����temp����
    bool is_not_considered_type(IRSymbol* sym);         //�Ƿ�Ϊ�����ǽ�������ı�����ȫ�ֱ��������顢ָ�롢�Ĵ������ڴ������
public:
    InstructionCombination(bool i_emit) :Pass(PassType::InstructionCombination, PassTarget::LIR, i_emit) {}
    void run();
};