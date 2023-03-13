#pragma once
#include <optimizer.h>
#include <cfg_manager.h>
#include <bitmap.h>
/*
    简单去掉了SSA下标，恢复原形式的符号。
    这种做法限制了一些优化的功能，今后有机会改成正规的做法
*/
class RevertSSA final :public Pass
{
private:
    void rewrite_defs(IRUnit* unit);
    void rewrite_instructions(IRBlock* entry);
public:
    RevertSSA(bool i_emit) :Pass(PassType::RevertSSA,PassTarget::CFG, i_emit) {}
    void run();
};
