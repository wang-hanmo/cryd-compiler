#pragma once
#include <optimizer.h>
#include <../ir/cfg.h>
#include <map>
#include <local_common_subexpression_elimination.h>
using MemId = int;
using MemVer = int;
using BBId = int;
using InstrId = int;
namespace gavs
{
    struct ArrayVisitExpression
    {
        int vn_array{-1};
        int version{-1};
        int vn_index{-1};
        ArrayVisitExpression() {}
        ArrayVisitExpression(int vn_array_, int version_,int vn_index_):
        vn_array(vn_array_), version(version_),vn_index(vn_index_){}
        bool operator<(const ArrayVisitExpression& rhs)const {
            if (vn_array != rhs.vn_array)
                return vn_array < rhs.vn_array;
            if (version != rhs.version)
                return version < rhs.version;
            return vn_index< rhs.vn_index;
        }
    };

}
/*
*   ���������������������������������������ȡ
*/
class GlobalArrayVisitSubstitution final :public Pass
{
private:
    /*
    1����ÿһ��memory��ţ�ά���������ͱ�����memory��ŵ�ӳ��
	    �ڶ�����ȫ�֡��ֲ������������Ӧ��memory���б��
	    �۷��ֺ�����ָ���Ͳ������򲻽��з�����
    */
    //ȫ�����������
    MemId m_global_array_conut;
    //ȫ������+��ǰ����������������
    MemId m_array_count;
    //ָ����
    InstrId m_instr_count;
    //Memory��� Symbol id -> Memory id
    //std::vector<SymId> m_memid;
    //Memory�����汾��ţ����ϵ��� Memory id -> Memory ver
    std::vector<MemVer> m_max_memver;
    //ָ���е�Memory�汾��� Instr id -> Memory ver
    std::vector<MemVer> m_memver_of_instr;
    //Memory�Ƿ������� Memory id -> bool
    std::vector<bool> m_mem_should_analyze;
    //Memory id -> symbol
    std::vector<IRSymbol*> m_mem_def;
    /*
    2����ÿ��memory��������䶨ֵ�Ļ����鼯�ϵĵ���֧��߽�
    3���ڵ���֧��߽紦����ÿ�memory���޸ĵı��
    4����ÿ��ArrayStore��䣬ά��memory�İ汾���
        ��memory�����޸ģ���汾��+1���޸�������array store,call,call with return
    */
    std::vector<BitMap> m_dominance_frontier;           //ʹ��bitmap���浱ǰ�������֧��߽�,ʹ��֧�����ϵ�dfn��Ϊ����
    std::vector<IRBlock*> m_idfn;                       //inverse dfn,ͨ��dfn���������ڵ�  BBId -> BB
    std::vector<BitMap> m_mem_modified_block;           //memory���޸Ĺ��Ļ�����             MemId -> BB List
    std::vector<BitMap> m_mem_dominance_frontiers_plus; //memory�ĵ���֧��߽�,tag����       MemId -> BB List
    //SSA��ʽ�µİ汾��ջ��list.back()ά����ջ��Ԫ�أ�ջ��Ԫ�ر�ʾ��ǰ���µİ汾��
    std::vector<std::list<MemVer>> m_current_memver;
    //ȫ�ֳ�ʼ��
    void global_init();
    //����������������SSA��ʽ
    bool build_ssa(IRUnit* unit);
    //��֧����dfs��ά��idfn
    void compute_idfn(IRBlock* entry);
    void compute_dominance_frontiers();
    //��¼memory���޸ĵĻ����鼯��
    void record_modified_block(IRUnit* unit);
    //�������֧��߽�
    void compute_dfp_for_all_vars();
    BitMap compute_dominance_frontiers_plus(const BitMap& set);
    BitMap compute_dominance_frontiers_set(const BitMap& set);
    //�ڻ����鿪ͷ�������ڽڵ��ϵ��µİ汾���±��
    void insert_memory_converge_mark();
    //���ÿ����������ʶ�Ӧ������汾��
    int mark_memver_modify(IRSymbol* sym);
    int mark_memver_use(IRSymbol* sym);
    void mark_memver_in_block(IRBlock* entry);
    void mark_memver_in_defination_area(IRUnit* unit);
    //��ȫ��ֵ��ţ�ɾ�������������
    std::map<IRSymbol*, int, lcse::SymbolComparer> m_vn_of_sym;     //���ŵ�ֵ���
    std::map<gavs::ArrayVisitExpression, int> m_vn_of_array_visit;  //�������õ�ֵ���
    std::map<int, IRSymbol*> m_vn_to_sym;                           //ĳ��ֵ��ŵ��������
    int m_vn;//��ǰ����ֵ���
    int find_or_new_vn(IRSymbol* sym);
    int find_vn(IRSymbol* sym);
    int new_vn(IRSymbol* sym);
    void work_vn_block(IRInstrList& program);
    void roll_back(IRInstrList& program);
    void work_vn(IRBlock* block);
    //ɾ��ÿ�������鿪ͷ�ı��
    void delete_mark(IRUnit* unit);
public:
    GlobalArrayVisitSubstitution(bool i_emit) :Pass(PassType::GlobalArrayVisitSubstitution,PassTarget::CFG, i_emit) {}
    void run();
};