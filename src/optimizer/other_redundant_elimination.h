#pragma once

#include <optimizer.h>
#include <cfg.h>
class OtherRedundantElimination final :public Pass
{
private:
    void work_unit(IRUnit* unit);
    void work_block(IRBlock* block);
public:
    OtherRedundantElimination(bool i_emit) :Pass(PassType::OtherRedundantElimination,PassTarget::CFG, i_emit) {}
    void run();
};