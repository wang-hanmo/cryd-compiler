#pragma once
#include <optimizer.h>
#include <../back_end/arm_struct.h>

class SimplifyArm final: public Pass
{
private:
    void eliminate_identical_move(ArmFunc*& func);
    void eliminate_useless_branch_before(ArmFunc*& func);
    void eliminate_useless_branch(ArmFunc*& func);
    void eliminate_useless_ldrstr(ArmFunc*& func);
    void inst_conditioning_simple(ArmFunc*& func);
    void inst_conditioning(ArmFunc*& func);
    void copy_block(ArmBlock* dst, ArmBlock* src);
    ArmBlock* choose_next_bb(ArmBlock* bb, ArmBlock* bb_ture, ArmBlock* bb_false);
public:
    SimplifyArm(bool i_emit): Pass(PassType::SimplifyArm, PassTarget::ARM, i_emit) {}
    void run();
};
