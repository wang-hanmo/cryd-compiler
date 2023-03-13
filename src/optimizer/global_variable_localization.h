#pragma once

#include <optimizer.h>
#include <cfg.h>
/*
* ȫ�ֱ����ֲ����Ż��������ȼ�����1�����ȣ�
* (TODO)1������ȫ�ֱ����������δ��ֵ����������
* 2������ȫ�ֱ��������ֻ��main�����г��֣���ֲ���Ϊmain������һ���ֲ�����
*/
class GlobalVarLocalization final :public Pass
{
private:
    std::vector<int> m_should_localization;
    std::vector<IRSymbol*> m_local_sym; //��ȫ�ֱ�����tag�������ֲ������symbol
    void work_unit(IRUnit* unit);
public:
    GlobalVarLocalization(bool i_emit) :Pass(PassType::GlobalVarLocalization,PassTarget::CFG, i_emit) {}
    void run();
};