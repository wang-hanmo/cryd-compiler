#pragma once
#include <cfg.h>
#include <optimizer.h>
class ConstantFoldAndAlgebraicSimplification final : public Pass
{
private:
    void ir_list_fold(IRInstrList& program);
    void control_flow_graph_fold(IRBlock* entry);
public:
    void work_binary_calc(IRInstr& instr);
    void work_unary_calc(IRInstr& instr);
    ConstantFoldAndAlgebraicSimplification(bool i_emit) :Pass(PassType::ConstantFoldAndAlgebraicSimplification,PassTarget::CFG, i_emit) {}
    void run();
};
