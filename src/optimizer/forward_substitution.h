#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
/*
    tagʹ�������
    l��p������ϱ���,���鲻����
    t������������
    g������������
    �������ԣ�
    ����bfs����������ʱ������temp�����Ķ�ֵһ����ʹ��ǰ����
*/
//���ţ���������
namespace forward_substitution {
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
}
class ForwardSubstitution final : public Pass
{
private:
    IRSymbol* check_replace(IRSymbol* sym);
    void delete_related_instr(IRSymbol* sym);
    void work_cfg(IRUnit* unit);
    void work_ir_list(IRInstrList& program);
    void work_assign(IRInstr* instr);
    void work_unary_calc(IRInstr* instr);
    void work_binary_calc(IRInstr* instr);
    void work_ternary_calc(IRInstr* instr);
    void work_block_cond_goto(IRInstr* instr);
    void init();
    void get_use_info_for_temp_vars(IRUnit* unit);      //����t�����Ķ�ֵʹ����Ϣ
    std::vector<int> m_use_count;                       //t�������->ʹ�ô���
    std::vector<std::vector<IRSymbol*>> m_related_sym_glb;  //g����->��֮���������䶨ֵ����б�ʹ�ù�����t����
    std::vector<std::vector<IRSymbol*>> m_related_sym;      //lp����->��֮���������䶨ֵ����б�ʹ�ù�����t����
    std::set<forward_substitution::SymDefPair> m_sym_def_set;                 //��ǰ��Ч��(t��������ֵָ��)�Լ���
    std::set<forward_substitution::SymDefPair> m_sym_def_set_glb;             //ͬ�ϣ������Ƕ�ֵ����g���������
    int m_temp_count;
    int m_local_param_count;
    int m_global_count;
    bool is_non_pointer_temp_var(IRSymbol* sym);        //�Ƿ�Ϊ��ָ����temp����
    bool is_not_considered_type(IRSymbol* sym);         //�Ƿ�Ϊ�����ǽ�������ı�����ȫ�ֱ��������顢ָ�롢�Ĵ������ڴ������
public:
    ForwardSubstitution(bool i_emit) :Pass(PassType::ForwardSubstitution, PassTarget::LIR, i_emit) {}
    void run();
};