#include "arm_struct.h"

void ArmBlock::push_back(ArmInstr inst)
{
    inst_list.push_back(inst);
}

void ArmBlock::gen_asm(std::ostream& os)
{
    os << label << ":" << std::endl;
    for (auto inst: inst_list) {
        inst.gen_asm(os);
    }
}

void ArmFunc::push_back(ArmBlock* block)
{
    blocks.push_back(block);
}


void ArmFunc::gen_asm(std::ostream& os)
{
    os << ".global " << name << std::endl;
    os << "\t.type " << name << ", %function" << std::endl;
    for (auto block: blocks) {
        block->gen_asm(os);
    }
}

void ArmProg::push_back(ArmFunc* func)
{
    func_list.push_back(func);
}

void ArmProg::gen_asm(std::ostream& os)
{
    for (auto func: func_list)
        func->gen_asm(os);
}