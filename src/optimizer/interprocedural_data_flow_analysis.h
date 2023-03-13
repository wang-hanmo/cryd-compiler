#pragma once
#include <iostream>
#include <cfg.h>
#include <vector>
#include <bitmap.h>
#include <set>
#include <queue>
/*
    ���̼�����������
    ȫ�ֱ�������Ż�
    ʹ��tag�����
    unit�Լ�������������tag���ڶԺ������б��
    ȫ�ֱ�����tag���ڶ�ȫ�ֱ������б��
*/
class InterproceduralDataFlowAnalysis
{
private:
    bool m_global_symbol_optimization_is_valid;  //�Ƿ���ȫ�ַ�������Ż�          (������ɱ���ʱ������������
    // bool m_side_effect_info_is_valid;            //������������Ϣ�Ƿ���Ч
    //����ͼ
    std::vector<std::set<int>> m_converse_call_map;
    std::vector<std::set<int>> m_call_map;
    /*  ��������Ϣ   */
    std::vector<BitMap> m_global_var_use;        //������ȫ�ַ����������ʹ����Ϣ(�ڼ����ڵĿ���ʹ�ã����ڼ����ڵ�һ������ʹ��)
    std::vector<BitMap> m_global_var_def;        //������ȫ�ַ���������Ķ�ֵ��Ϣ(�ڼ����ڵĿ��ܶ�ֵ�����ڼ����ڵ�һ�����ᶨֵ��
    std::vector<bool> m_has_side_effect;         //�����Ƿ��и�����(��ȫ�ֱ�����������鶨ֵ,���ÿ⺯����
    std::vector<bool> m_affected_by_env;         //�����Ƿ��ܻ���Ӱ��(���ε��ò�����ͬ����ֵ��ͬ)����ȫ�ֱ������з���,���ÿ⺯����
    std::vector<IRSymbol*> m_global_var;         //ȫ�ֱ�����ŵ����ŵ�ӳ��
    std::vector<IRUnit*> m_procedure;            //������ŵ�������ӳ��
    //��������
    int m_func_count;
    //������ȫ�ֱ�������
    int m_global_var_count;
    IRProgram* m_cfg{ nullptr };
    IRSymbolTable* m_ir_sym_table{ nullptr };
    void build_call_map_and_local_side_effect(IRUnit* unit);
public:
    //�Ժ������б�ţ�����tag��
    void procedure_numbering();
    //�Է������ȫ�ֱ������б�ţ�����tag��
    void global_var_numbering();
    //���㺯����������Ϣ,�������Ǻ�������ʱֵ�ͱ����ǵ������Ϊ�˽��͸��Ӷȣ�,��Ժ�����ȫ�ֱ������±��
    void compute_side_effect_info();
    //��ֱ�ӵ����Լ��ĵݹ麯��
    bool is_direct_recursion_function(int index) { return m_call_map[index].find(index) != m_call_map[index].end(); }
    int get_func_count()const { return m_func_count; }
    int get_global_var_count()const { return m_global_var_count; }
    const BitMap& get_global_var_use(int index) { return m_global_var_use[index]; }
    const BitMap& get_global_var_def(int index) { return m_global_var_def[index]; }
    IRUnit* get_procedure(int index) { return m_procedure[index]; }
    IRSymbol* get_global_var(int index) { return m_global_var[index]; }
    bool has_side_effect(int index) { return m_has_side_effect[index]; }
    bool affected_by_env(int index) { return m_affected_by_env[index]; }
    void set_target(IRProgram* cfg, IRSymbolTable* ir_sym_table) { m_cfg = cfg; m_ir_sym_table = ir_sym_table; }
    void enable_global_symbol_optimization() { m_global_symbol_optimization_is_valid = true; }
    bool global_symbol_optimization_is_valid() { return m_global_symbol_optimization_is_valid; }
};
