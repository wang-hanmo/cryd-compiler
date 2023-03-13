#pragma once
#include <cfg.h>
#include <optimizer.h>
#include <set>
#include <map>
#include <ir_instr.h>
#include <ir_define.h>
#include <ir_symbol_table.h>
#include <local_common_subexpression_elimination.h>
//ʹ��ȫ��ֵ��ŵķ�����ȡ�����ӱ��ʽ����������ȫ�ֱ�����
//����SSAת��ԭ��ʽд�ý�Ϊ��ª������������phi�����������һ���µ�ֵ���
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
    std::map<IRSymbol*, int, lcse::SymbolComparer> m_vn_of_sym; //���ŵ�ֵ���
    std::map<gcse::CallExpression, int> m_vn_of_call;           //�������õ�ֵ���
    std::map<lcse::Expression, int> m_vn_of_expr;               //���ʽ��ֵ���
    std::map<int,IRSymbol*> m_vn_to_sym;                   //ĳ��ֵ��ŵ��������
    int m_glb_var_count;
    int m_vn;
    int find_or_new_vn(IRSymbol* sym);
    int find_vn(IRSymbol* sym);
    int new_vn(IRSymbol* sym);
    //��ȡ�������ñ��ʽ��first��ʾ�Ƿ�ɹ���ȡ�������и����û��߲�����ȫ�ֱ����򲻳ɹ���
    std::pair<bool, gcse::CallExpression> get_call_expression(const std::vector<IRSymbol*>& params,IRSymbol* func);
    void work(IRBlock* block);
    void work_block(IRInstrList& program);
    void roll_back(IRInstrList& program);
    void init();//��ʼ��ȫ�ַ�����������
public:
    GlobalCommonSubexpressionElimination(bool i_emit) :Pass(PassType::GlobalCommonSubexpressionElimination,PassTarget::CFG, i_emit) {}
    void run();
};