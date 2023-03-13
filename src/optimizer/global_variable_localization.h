#pragma once

#include <optimizer.h>
#include <cfg.h>
/*
* 全局变量局部化优化（按优先级排序，1最优先）
* (TODO)1、对于全局变量，如果从未定值过，则常量化
* 2、对于全局变量，如果只在main函数中出现，则局部化为main函数的一个局部变量
*/
class GlobalVarLocalization final :public Pass
{
private:
    std::vector<int> m_should_localization;
    std::vector<IRSymbol*> m_local_sym; //从全局变量的tag索引到局部化后的symbol
    void work_unit(IRUnit* unit);
public:
    GlobalVarLocalization(bool i_emit) :Pass(PassType::GlobalVarLocalization,PassTarget::CFG, i_emit) {}
    void run();
};