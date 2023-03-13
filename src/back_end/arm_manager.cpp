#include <arm_manager.h>
#include <arm.h>
#include <cassert>
#include <sstream>
#include <arm_define.h>
#include <ir_symbol_table.h>
#include <queue>
#include <reg_alloc.h>
#include <set>
using namespace std;

Reg* ArmManager::Symbol2Reg(IRSymbol* a, bool is_rd, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    if (a->kind() == IRSymbolKind::Register)
        return machine_regs[a->index()];

    Reg* reg = nullptr;
    if (a->kind() != IRSymbolKind::Global && !rat->is_spill(a)) {
        RegID id = rat->reg_alloc(a);
        if(a->basic_type() == BasicType::Int || a->array_length() != IRArrayLength::IR_NONE_ARRAY)
        {
            if(id >= RegCount || id < 0)
                arm_error("14", 14);
        }
        else
        {
            if(id < RegCount || id >= RegCount + RegCount_S)
                arm_error("15", 15);
        }
        
        reg = machine_regs[id];
        reg_list.emplace(reg);
    } else {
        if (a->basic_type() == BasicType::Int || a->array_length() != -1) {
            for (int i = 0; i < RegCount; i++)
                if (REGISTER_USAGE[i] != RegisterUsage::special 
                    && (!rat->test_reg_live((RegID)i, current_line_no)) || dst_reg == machine_regs[i]) {
                    if (reg_list.find(machine_regs[i]) == reg_list.end() && rparam_regs.find(machine_regs[i]) == rparam_regs.end()) {
                        reg = machine_regs[i];
                        reg_list.emplace(reg);
                        break;
                    }
                }
        } else {
            for (int i = RegCount; i < RegCount + RegCount_S; i++)
                if (!rat->test_reg_live((RegID)i, current_line_no) || dst_reg == machine_regs[i]) {
                    if (reg_list.find(machine_regs[i]) == reg_list.end() && rparam_regs.find(machine_regs[i]) == rparam_regs.end()) {
                        reg = machine_regs[i];
                        reg_list.emplace(reg);
                        break;
                    }
                }
        }
        
        if (reg == nullptr) {
            if (a->basic_type() == BasicType::Int || a->array_length() != -1) {
                if (!is_rd) {
                    // 如果指令I的其余分量已经占有一个寄存器r，则该运算分量不能再将r作为自己的寄存器
                    for (int i = 0; i < RegCount; i++)
                        if (reg_list.find(machine_regs[i]) == reg_list.end() && REGISTER_USAGE[i] != RegisterUsage::special
                            && occupied_regs.find(machine_regs[i]) == occupied_regs.end()) {
                            auto r = var_of_reg[machine_regs[i]->get_reg_id()];
                            reg = machine_regs[i];
                            break;
                        }
                } else {
                    // 如果运算分量之一在之后不再使用，目的寄存器可以使用源寄存器即可（待优化）
                    for (int i = 0; i < RegCount; i++) {
                        if (reg_list.find(machine_regs[i]) == reg_list.end() && REGISTER_USAGE[i] != RegisterUsage::special
                                && occupied_regs.find(machine_regs[i]) == occupied_regs.end()) {
                                auto r = var_of_reg[machine_regs[i]->get_reg_id()];
                                reg = machine_regs[i];
                                break;
                        }
                    }
                }
            } else {
                if (!is_rd) {
                    // 如果指令I的其余分量已经占有一个寄存器r，则该运算分量不能再将r作为自己的寄存器
                    for (int i = RegCount; i < RegCount + RegCount_S; i++)
                        if (reg_list.find(machine_regs[i]) == reg_list.end() && occupied_regs.find(machine_regs[i]) == occupied_regs.end()) {
                            auto r = var_of_reg[machine_regs[i]->get_reg_id()];
                            reg = machine_regs[i];
                            break;
                        }
                } else {
                    // 如果运算分量之一在之后不再使用，目的寄存器可以使用源寄存器即可（待优化）
                    for (int i = RegCount; i < RegCount + RegCount_S; i++) {
                        if (reg_list.find(machine_regs[i]) == reg_list.end() && occupied_regs.find(machine_regs[i]) == occupied_regs.end()) {
                            auto r = var_of_reg[machine_regs[i]->get_reg_id()];
                            reg = machine_regs[i];
                            break;
                        }
                    }
                }
            }
            if(reg == nullptr)
                arm_error("Register nullptr", 0);
            reg_list.emplace(reg);
            push_reg.emplace_back(reg);
            store_var(var_of_reg[reg->get_reg_id()], reg, reg_list, push_reg);
        }
    }
    
    // 若符号a出现在等号的右边，则需要判断a的值之前是否在reg中，若不在，则需要先将a的值load进来
    if (var_map.find(a) == var_map.end()) {
        if (!is_rd) {
            load_var(a, reg);
        }
        if (a->kind() != IRSymbolKind::Global && !rat->is_spill(a)) {
            var_map.emplace(a, reg);
        }
    }
    if (a->kind() != IRSymbolKind::Global && !rat->is_spill(a)) {
        var_of_reg[reg->get_reg_id()] = a;
    }
    current_func->used_regs.emplace(reg);
    return reg;
}

Reg* ArmManager::find_free_reg(unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    Reg* reg = nullptr;
    for (int i = 0; i < RegCount; i++)
        if (REGISTER_USAGE[i] != RegisterUsage::special && !rat->test_reg_live((RegID)i, current_line_no)) {
            if (reg_list.find(machine_regs[i]) == reg_list.end() && rparam_regs.find(machine_regs[i]) == rparam_regs.end()) {
                reg = machine_regs[i];
                reg_list.emplace(reg);
                break;
            }
        }
    
    if (reg == nullptr) {
        // 如果指令I的其余分量已经占有一个寄存器r，则该运算分量不能再将r作为自己的寄存器
        for (int i = 0; i < RegCount; i++) {
            if (reg_list.find(machine_regs[i]) == reg_list.end() && occupied_regs.find(machine_regs[i]) == occupied_regs.end() 
                && REGISTER_USAGE[i] != RegisterUsage::special) {
                reg = machine_regs[i];
                break;
            }
        }
        if (reg == nullptr)
            reg = machine_regs[0];//assert(0);
        reg_list.emplace(reg);
        push_reg.emplace_back(reg);
        store_var(var_of_reg[reg->get_reg_id()], reg, reg_list, push_reg);
    }
    current_func->used_regs.emplace(reg);
    return reg;
}

Reg* ArmManager::find_free_freg(unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    Reg* reg = nullptr;
    for (int i = RegCount; i < RegCount + RegCount_S; i++)
        if (!rat->test_reg_live((RegID)i, current_line_no)) {
            if (reg_list.find(machine_regs[i]) == reg_list.end()) {
                reg = machine_regs[i];
                reg_list.emplace(reg);
                break;
            }
        }
    
    if (reg == nullptr) {
        // 如果指令I的其余分量已经占有一个寄存器r，则该运算分量不能再将r作为自己的寄存器
        for (int i = RegCount; i < RegCount + RegCount_S; i++)
            if (reg_list.find(machine_regs[i]) == reg_list.end() && occupied_regs.find(machine_regs[i]) == occupied_regs.end()) {
                reg = machine_regs[i];
                break;
            }
        reg_list.emplace(reg);
        push_reg.emplace_back(reg);
        store_var(var_of_reg[reg->get_reg_id()], reg, reg_list, push_reg);
    }
    current_func->used_regs.emplace(reg);
    return reg;
}

void ArmManager::move_imm(int imm, Reg* reg)
{
    if (Operand2::checkImm8m(imm)) {
        Operand2* operand2 = new Operand2(imm);
        current_block->push_back(ArmInstr::create_move(Cond::AL, false, reg, operand2, false));
    } else if (imm < 0 && Operand2::checkImm8m(-imm - 1)) {
        Operand2* operand2 = new Operand2(-imm - 1);
        current_block->push_back(ArmInstr::create_move(Cond::AL, false, reg, operand2, true));
    } else {
        current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, reg, imm));
    }
}

void ArmManager::load_var(IRSymbol* a, Reg* reg)
{
    if (a == nullptr || reg == nullptr)
        return;//assert(0);
    unordered_set<Reg*> reg_list;
    vector<Reg*> push_reg;
    if (a->kind() == IRSymbolKind::Local || a->kind() == IRSymbolKind::Param || a->kind() == IRSymbolKind::Temp || a->kind() == IRSymbolKind::Register ||
        a->kind() == IRSymbolKind::Memory) {
        if (local_var_offset.find(a) == local_var_offset.end()) {
            if (a->kind() == IRSymbolKind::Local)
                arm_error("1", 1);
            else if (a->kind() == IRSymbolKind::Param)
                arm_error("9", 9);
            else if (a->kind() != IRSymbolKind::Memory) {
                cout << a->get_string() << reg->get_string() << endl;
                arm_error("10", 10);
            }
            else {bool flag = false;
                for (auto var: current_func->var_in_stack) {
                    if (var == a) {
                        flag = true;
                        break;
                    }
                }
                if (!flag)
                    current_func->var_in_stack.emplace_back(a);
                GenImmLdrStrInst(InstrType::Store, reg, machine_regs[RegID::sp], -4, reg_list, push_reg);
                stack_size += 4;
                local_var_offset.emplace(a, stack_size);
                Reg* rd = machine_regs[RegID::sp];
                current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SUB, false, rd, rd, new Operand2(4)));
            }
        }
        int offset = stack_size - local_var_offset.at(a);
        if (a->kind() == IRSymbolKind::Local && a->array_length() > 0) {
            Operand2* operand2 = nullptr;
            if (Operand2::checkImm8m(offset)) {
                operand2 = new Operand2(offset);
            } else {
                move_imm(offset, reg);
                operand2 = new Operand2(reg);
            }
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, reg, machine_regs[sp], operand2));
        }
        else {
            if (reg->get_reg_id() < RegCount)
                GenImmLdrStrInst(InstrType::Load, reg, machine_regs[RegID::sp], offset, reg_list, push_reg);
            else
                GenImmLdrStrInst(InstrType::LoadS, reg, machine_regs[RegID::sp], offset, reg_list, push_reg);
        }
    } else if (a->kind() == IRSymbolKind::Global) {
        // mov32 r0, label
        // ld r0, [r0]
        Reg* addr = nullptr;
        if (a->array_length() != -1 || a->basic_type() == BasicType::Int)
            current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, reg, a->global_sym()->name()));
        else {
            addr = find_free_reg(reg_list, push_reg);
            current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, addr, a->global_sym()->name()));
        }
        if (a->array_length() == -1) {
            if (a->basic_type() == BasicType::Int)
                current_block->push_back(ArmInstr::create_load(Cond::AL, reg, reg));
            else
                current_block->push_back(ArmInstr::create_float_load(Cond::AL, reg, addr));
        }
        for (auto reg: push_reg)
            load_var(var_of_reg[reg->get_reg_id()], reg);
    } else if (a->kind() == IRSymbolKind::Value) {
        // mov r0, operand2
        if (a->basic_type() == BasicType::Int) {
            int imm = a->value().int_value;
            move_imm(imm, reg);
        } else {
            float imm = a->value().float_value;
            if (false/*ArmInstr::checkFpconst(imm)*/)
                current_block->push_back(ArmInstr::create_float_move(Cond::AL, reg, imm));
            else {
                // 先将浮点数放到通用寄存器，然后再移动到浮点寄存器
                int imm = a->value().int_value;
                Reg* rd = find_free_reg(reg_list, push_reg);
                move_imm(imm, rd);
                current_block->push_back(ArmInstr::create_float_move(Cond::AL, reg, rd));
                for (auto reg: push_reg)
                    load_var(var_of_reg[reg->get_reg_id()], reg);
            }
        }
    }
    if (a->kind() != IRSymbolKind::Global && !rat->is_spill(a))
        var_map.emplace(a, reg);
}

void ArmManager::store_var(IRSymbol* s, Reg* reg, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    if (s == nullptr || reg == nullptr)
        return;//assert(0);
    if (s->kind() == IRSymbolKind::Local || s->kind() == IRSymbolKind::Param || s->kind() == IRSymbolKind::Temp || s->kind() == IRSymbolKind::Register ||
        s->kind() == IRSymbolKind::Memory) {
        // 需要先分配空间
        if (s->kind() == IRSymbolKind::Local && s->array_length() > 0) {  // nothing to do
        }
        else if (local_var_offset.find(s) == local_var_offset.end()) {
            bool flag = false;
            for (auto var: current_func->var_in_stack) {
                if (var == s) {
                    flag = true;
                    break;
                }
            }
            if (!flag)
                current_func->var_in_stack.emplace_back(s);
            GenImmLdrStrInst(InstrType::Store, reg, machine_regs[RegID::sp], -4, reg_list, push_reg);
            stack_size += 4;
            local_var_offset.emplace(s, stack_size);
            Reg* rd = machine_regs[RegID::sp];
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SUB, false, rd, rd, new Operand2(4)));
        } else {
            int offset = stack_size - local_var_offset.at(s);
            if (reg->get_reg_id() < RegCount)
                GenImmLdrStrInst(InstrType::Store, reg, machine_regs[RegID::sp], offset, reg_list, push_reg);
            else {
                if (reg->get_reg_id() < 16)
                    ArmManager::arm_error("27", 27);
                GenImmLdrStrInst(InstrType::StoreS, reg, machine_regs[RegID::sp], offset, reg_list, push_reg);
            }
        }
    } else if (s->kind() == IRSymbolKind::Global) {
        Reg* addr = nullptr;
        addr = find_free_reg(reg_list, push_reg);
        current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, addr, s->global_sym()->name()));
        if (s->basic_type() == BasicType::Int || s->array_length() != -1)
            current_block->push_back(ArmInstr::create_store(Cond::AL, reg, addr));
        else
            current_block->push_back(ArmInstr::create_float_store(Cond::AL, reg, addr));
    }
    if (s->kind() != IRSymbolKind::Global && !rat->is_spill(s))
        var_map.erase(s);
}

void ArmManager::store_global_array(IRSymbol* s, Reg* reg, Operand2* offset, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    if (s == nullptr || reg == nullptr || offset == nullptr)
        return;//assert(0);
    Reg* base = nullptr;
    base = find_free_reg(reg_list, push_reg);

    current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, base, s->global_sym()->name()));
    if (offset->get_reg() != nullptr) {
        if (reg->get_reg_id() < 16)
            current_block->push_back(ArmInstr::create_store(Cond::AL, reg, base, offset));
        else {
            // 浮点数的偏移量没有寄存器这一选项，因此需要把基址和偏移加起来作为基址
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, base, base, offset));
            current_block->push_back(ArmInstr::create_float_store(Cond::AL, reg, base));
        }
    }
    else {
        if (reg->get_reg_id() < 16)
            GenImmLdrStrInst(InstrType::Store, reg, base, offset->get_imm_val(), reg_list, push_reg);
        else
            GenImmLdrStrInst(InstrType::StoreS, reg, base, offset->get_imm_val(), reg_list, push_reg);
    }
}

Operand2* ArmManager::Symbol2Operand2(IRSymbol* b, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg, bool is_offset)
{
    if (b == nullptr)
        return new Operand2(0);//assert(0);
    // 是立即数且是整数
    if (b->kind() == IRSymbolKind::Value && b->basic_type() == BasicType::Int && Operand2::checkImm8m(b->value().int_value)) {
        if (is_offset)
            return new Operand2(b->value().int_value * 4);
        else
            return new Operand2(b->value().int_value);
    } else {
        if (is_offset) {
            auto reg = Symbol2Reg(b, false, reg_list, push_reg);
            auto shift = new Shift(ShiftOp::LSL, 2);
            return new Operand2(reg, shift);
        } else {
            return new Operand2(Symbol2Reg(b, false, reg_list, push_reg));
        }
    }
    arm_error("2", 2);
}

void ArmManager::GenImmLdrStrInst(InstrType type, Reg* rd, Reg* rn, int imm, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    if (rd == nullptr || rn == nullptr)
        return;//assert(0);
    if (type == InstrType::Load && ArmInstr::checkImm12(imm))
        current_block->push_back(ArmInstr::create_load(Cond::AL, rd, rn, imm));
    else if (type == InstrType::Store && ArmInstr::checkImm12(imm))
        current_block->push_back(ArmInstr::create_store(Cond::AL, rd, rn, imm));
    else if (type == InstrType::LoadS && ArmInstr::checkImmed(imm))
        current_block->push_back(ArmInstr::create_float_load(Cond::AL, rd, rn, imm));
    else if (type == InstrType::StoreS && ArmInstr::checkImmed(imm))
        current_block->push_back(ArmInstr::create_float_store(Cond::AL, rd, rn, imm));
    else {
        Reg* offset = nullptr;
        if (type == InstrType::Load) {
            if (rn != rd)
                offset = rd;
            else {
                reg_list.emplace(rd); 
                offset = find_free_reg(reg_list, push_reg);
                // offset = machine_regs[lr];
                // current_func->used_regs.emplace(offset);
            }
        }
        else {
            reg_list.emplace(rd);
            reg_list.emplace(rn);
            offset = find_free_reg(reg_list, push_reg);
            // offset = machine_regs[lr];
            // current_func->used_regs.emplace(offset);
        }
        if (Operand2::checkImm8m(imm)) {  // mov
            current_block->push_back(ArmInstr::create_move(Cond::AL, false, offset, new Operand2(imm), false));
        } else if (imm < 0 && Operand2::checkImm8m(-imm - 1)) {  // mvn
            current_block->push_back(ArmInstr::create_move(Cond::AL, false, offset, new Operand2(-imm - 1), true));
        } else {  // ldr-pseudo
            current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, offset, imm));
        }
        if (type == InstrType::Load)
            current_block->push_back(ArmInstr::create_load(Cond::AL, rd, rn, new Operand2(offset)));
        else if (type == InstrType::LoadS) {
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, offset, rn, new Operand2(offset)));
            current_block->push_back(ArmInstr::create_float_load(Cond::AL, rd, offset));
        } else if (type == InstrType::Store)
            current_block->push_back(ArmInstr::create_store(Cond::AL, rd, rn, new Operand2(offset)));
        else {
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, offset, rn, new Operand2(offset)));
            current_block->push_back(ArmInstr::create_float_store(Cond::AL, rd, offset));
        }
    }
}

void ArmManager::CreateShiftInst(IROper op, ShiftOp shift_op, IRSymbol* r, IRSymbol* a, IRSymbol* b, IRSymbol* c, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    Reg* rn = nullptr;
    if (a != nullptr)
        rn = Symbol2Reg(a, false, reg_list, push_reg);
    auto operand2 = Symbol2Operand2(b, reg_list, push_reg);
    auto rd = Symbol2Reg(r, true, reg_list, push_reg);
    Shift* shift = nullptr;
    if (c->kind() == IRSymbolKind::Value)
        shift = new Shift(shift_op, c->value().int_value);
    else {
        auto rs = Symbol2Reg(c, false, reg_list, push_reg);
        shift = new Shift(shift_op, rs);
    }
    operand2->set_shift(shift);

    switch(op) {
        case IROper::ShiftI:
            current_block->push_back(ArmInstr::create_move(Cond::AL, false, rd, operand2));
            break;
        case IROper::ShiftAddI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, rd, rn, operand2));
            break;
        case IROper::ShiftSubI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SUB, false, rd, rn, operand2));
            break;
        case IROper::ShiftRsbI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::RSB, false, rd, rn, operand2));
            break;
        default:
            assert(0);
    }
    if (rat->is_spill(r))
        store_var(r, rd, reg_list, push_reg);
    for (auto reg: push_reg)
        load_var(var_of_reg[reg->get_reg_id()], reg);
}

void ArmManager::CreateBinaryInst(IROper op, IRSymbol* r, IRSymbol* a, IRSymbol* b, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    bool is_offset = false;
    if (r != nullptr)
        is_offset = (r->array_length() != -1);
    
    if (op == IROper::ModI)
        update_occupied_regs(a, b, r, false);
    else
        update_occupied_regs(a, b, r);
    // 这些操作可以交换源操作数
    bool exchange = false;
    if (op == IROper::AddI || op == IROper::SubI || op == IROper::BitAnd || op == IROper::BitOr || op == IROper::BitXor || op == IROper::BitClear) {
        if (a->kind() == IRSymbolKind::Value) {
            // exchange order: a->op2 b->rn res->rd
            auto temp = a;
            a = b;
            b = temp;
            exchange = true;
        }
    }
    auto rn = Symbol2Reg(a, false, reg_list, push_reg);
    Operand2* operand2 = nullptr;
    Reg* rm = nullptr;
    // 有cmpwith0指令，不需要operand2
    if (op != IROper::EqualF && op != IROper::NotEqualF && op != IROper::GreaterEqualF && op != IROper::LessEqualF && op != IROper::GreaterF && op != IROper::LessF ||
        b->value().float_value != 0) {
        if (op == IROper::MulI || op == IROper::DivI || op == IROper::ModI || op == IROper::SmmulI)
            operand2 = new Operand2(Symbol2Reg(b, false, reg_list, push_reg));
        else
            operand2 = Symbol2Operand2(b, reg_list, push_reg, is_offset);
        rm = operand2->get_reg();
    }
    Reg* rd = nullptr;
    bool sign;
    // BlockBinaryGoto
    if (r == nullptr) {
        // rd = machine_regs[lr];
        if (op != IROper::EqualF && op != IROper::NotEqualF && op != IROper::GreaterEqualF && op != IROper::LessEqualF && op != IROper::GreaterF && op != IROper::LessF && 
            op != IROper::EqualI && op != IROper::NotEqualI && op != IROper::GreaterEqualI && op != IROper::LessEqualI && op != IROper::GreaterI && op != IROper::LessI) {
            rd = find_free_reg(reg_list, push_reg);
            current_func->used_regs.emplace(rd);
            }
        sign = true;
    } else {
        rd = Symbol2Reg(r, true, reg_list, push_reg);
        sign = false;
    }
    dst_reg = rd;
    BinaryOp opcode = exchange ? BinaryOp::RSB : BinaryOp::SUB;

    switch (op) {
        case IROper::AddI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, sign, rd, rn, operand2));
            break;
        case IROper::AddF:
            current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::ADD, rd, rn, rm));
            break;
        case IROper::SubI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, opcode, sign, rd, rn, operand2));
            break;
        case IROper::RsbI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::RSB, sign, rd, rn, operand2));
            break;
        case IROper::SubF:
            current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, opcode, rd, rn, rm));
            break;
        case IROper::MulI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::MUL, sign, rd, rn, operand2));
            break;
        case IROper::MulF:
            current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::MUL, rd, rn, rm));
            break;
        case IROper::SmmulI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SMMUL, sign, rd, rn, operand2));
            break;
        case IROper::DivI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SDIV, sign, rd, rn, operand2));
            break;
        case IROper::DivF:
            current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::SDIV, rd, rn, rm));
            break;
        case IROper::ModI:
            // a mod b = a-(a/b)*b
            // sdiv rd, rn, op2
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SDIV, sign, rd, rn, operand2));
            // mul rd, rd, op2
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::MUL, sign, rd, rd, operand2));
            // sub rd, rn, rd
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SUB, sign, rd, rn, new Operand2(rd)));
            break;
        case IROper::BitAnd:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::AND, sign, rd, rn, operand2));
            break;
        case IROper::BitOr:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ORR, sign, rd, rn, operand2));
            break;
        case IROper::BitXor:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::EOR, sign, rd, rn, operand2));
            break;
        case IROper::BitClear:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::BIC, sign, rd, rn, operand2));
            break;
        case IROper::EqualI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::EQ, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::NE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::EqualF:
            if (b->value().float_value != 0) {
                current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::CMP, nullptr, rn, rm));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            } else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::EQ, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::NE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::NotEqualI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::NE, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::EQ, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::NotEqualF:
            if (b->value().float_value != 0) {
                current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::CMP, nullptr, rn, rm));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            } else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::NE, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::EQ, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::GreaterI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::GT, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::LE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::GreaterF:
            if (b->value().float_value != 0) {
                current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::CMP, nullptr, rn, rm));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            } else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::GT, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::LE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::LessI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::LT, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::GE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::LessF:
            if (b->value().float_value != 0) {
                current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::CMP, nullptr, rn, rm));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            } else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::LT, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::GE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::GreaterEqualI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::GE, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::LT, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::GreaterEqualF:
            if (b->value().float_value != 0) {
                current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::CMP, nullptr, rn, rm));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            } else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::GE, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::LT, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::LessEqualI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::LE, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::GT, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::LessEqualF:
            if (b->value().float_value != 0) {
                current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::CMP, nullptr, rn, rm));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            } else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::LE, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::GT, false, rd, new Operand2(0), false));
            }
            break;
        default:
            assert(0);
    }

    if (r != nullptr && rat->is_spill(r))
        store_var(r, rd, reg_list, push_reg);
    for (auto reg: push_reg)
        load_var(var_of_reg[reg->get_reg_id()], reg);
}

void ArmManager::CreateUnaryInst(IROper op, IRSymbol* r, IRSymbol* a, unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    update_occupied_regs(a, nullptr, r);
    auto rn = Symbol2Reg(a, false, reg_list, push_reg);
    Reg* rd = nullptr;
    Reg* sd = nullptr;
    bool sign;
    if (r == nullptr) {
        // rd = machine_regs[lr];
        rd = find_free_reg(reg_list, push_reg);
        if (op != IROper::NotI && op != IROper::NotF)
            current_func->used_regs.emplace(rd);
        sign = true;
    } else {
        rd = Symbol2Reg(r, true, reg_list, push_reg);
        sign = false;
    }

    switch (op) {
        case IROper::NegI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::RSB, sign, rd, rn, new Operand2(0)));
            break;
        case IROper::NegF:
            current_block->push_back(ArmInstr::create_float_binary_calc(Cond::AL, BinaryOp::NEG, rd, nullptr, rn));
            break;
        case IROper::NotI:
            current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, new Operand2(0)));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::EQ, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::NE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::NotF:
            current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
            current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            if (r != nullptr) {
                current_block->push_back(ArmInstr::create_move(Cond::EQ, false, rd, new Operand2(1), false));
                current_block->push_back(ArmInstr::create_move(Cond::NE, false, rd, new Operand2(0), false));
            }
            break;
        case IROper::IToF:
            // vmov sm, rn
            // vcvt.f32.s32 sd, sm
            current_block->push_back(ArmInstr::create_float_move(Cond::AL, rd, rn));
            current_block->push_back(ArmInstr::create_int_to_float(Cond::AL, rd, rd));
            break;
        case IROper::FToI:
            // vcvt.s32.f32 sd, sm
            // vmov rd, sd
            sd = find_free_freg(reg_list, push_reg);
            current_block->push_back(ArmInstr::create_float_to_int(Cond::AL, sd, rn));
            current_block->push_back(ArmInstr::create_float_move(Cond::AL, rd, sd));
            break;
        default:
            assert(0);
    }

    if (r != nullptr && rat->is_spill(r))
        store_var(r, rd, reg_list, push_reg);
    for (auto reg: push_reg)
        load_var(var_of_reg[reg->get_reg_id()], reg);
}

void ArmManager::CreateTernaryInst(IROper op, IRShiftOper sop, IRSymbol* r, IRSymbol* a, IRSymbol* b, IRSymbol* c, 
    unordered_set<Reg*>& reg_list, vector<Reg*>& push_reg)
{
    update_occupied_regs(a, b, r, true, c);
    // IRShiftOper->ShiftOp
    ShiftOp shift_op;
    switch (sop) {
        case IRShiftOper::ASR:
            shift_op = ShiftOp::ASR;
            break;
        case IRShiftOper::LSL:
            shift_op = ShiftOp::LSL;
            break;
        case IRShiftOper::LSR:
            shift_op = ShiftOp::LSR;
            break;
        default:
            break;  
    }
    if (op == IROper::SignedLargeMulI) {
        auto rd = Symbol2Reg(c, true, reg_list, push_reg);
        auto rd_hi = Symbol2Reg(r, true, reg_list, push_reg);
        auto rn = Symbol2Reg(a, false, reg_list, push_reg);
        auto operand2 = new Operand2(Symbol2Reg(b, false, reg_list, push_reg));
        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SMULL, false, rd, rd_hi, rn, operand2));
        if (rat->is_spill(c))
            store_var(c, rd, reg_list, push_reg);
        for (auto reg: push_reg)
            load_var(var_of_reg[reg->get_reg_id()], reg);
    } else if (op == IROper::MulAddI || op == IROper::SignedLargeMulAddI) {
        // mla rd, rm, rs, rn   rd=rn+rm*rs
        auto rd = Symbol2Reg(r, true, reg_list, push_reg);
        auto rn = Symbol2Reg(a, false, reg_list, push_reg);
        auto rm = Symbol2Reg(b, false, reg_list, push_reg);
        auto rs = Symbol2Reg(c, false, reg_list, push_reg);
        BinaryOp op_;
        if (op == IROper::MulAddI)
            op_ = BinaryOp::MLA;
        else
            op_ = BinaryOp::SMMLA;
        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, op_, false, rd, rm, rs, new Operand2(rn)));
        if (rat->is_spill(r))
            store_var(r, rd, reg_list, push_reg);
        for (auto reg: push_reg)
            load_var(var_of_reg[reg->get_reg_id()], reg);
    } else {
        CreateShiftInst(op, shift_op, r, a, b, c, reg_list, push_reg);
    }
}

void ArmManager::update_occupied_regs(IRSymbol* a, IRSymbol* b, IRSymbol* r, bool has_dst, IRSymbol* c)
{
    occupied_regs.clear();
    dst_reg = nullptr;
    Reg* rn = nullptr;
    Reg* rm = nullptr;
    Reg* rs = nullptr;
    if (a != nullptr && a->kind() != IRSymbolKind::Global && !rat->is_spill(a)) {
        occupied_regs.emplace(machine_regs[rat->reg_alloc(a)]);
        rn = machine_regs[rat->reg_alloc(a)];
    }
    if (b != nullptr && b->kind() != IRSymbolKind::Global && !rat->is_spill(b)) {
        occupied_regs.emplace(machine_regs[rat->reg_alloc(b)]);
        rm = machine_regs[rat->reg_alloc(b)];
    }
    if (c != nullptr && !rat->is_spill(c)) {
        occupied_regs.emplace(machine_regs[rat->reg_alloc(c)]);
        rs = machine_regs[rat->reg_alloc(c)];
    }
    if (!has_dst && r != nullptr && r->kind() != IRSymbolKind::Global && !rat->is_spill(r))
        occupied_regs.emplace(machine_regs[rat->reg_alloc(r)]);
    if (has_dst && r != nullptr && r->kind() != IRSymbolKind::Global && !rat->is_spill(r)) {
        dst_reg = machine_regs[rat->reg_alloc(r)];
        if (dst_reg == rn || dst_reg == rm || dst_reg == rs)
            dst_reg = nullptr;
    }
}

void ArmManager::update_sp(BinaryOp op, int offset, unordered_set<IRSymbol*> allocate_var)
{
    if (first_scan) {
        move_sp(offset);
    }
    vector<IRSymbol*> symbols = rat->get_ir_symbol();
    for (auto s: symbols) {
        if (!rat->is_spill(s))
            var_of_reg[rat->reg_alloc(s)] = s;
    }
    if (first_scan) {
        for (auto iter = local_var_offset.begin(); iter != local_var_offset.end();) {
            if (allocate_var.find(iter->first) == allocate_var.end())
                local_var_offset.erase(iter++);
            else
                ++iter;
        }
    }
}

void ArmManager::move_sp(int offset)
{
    if (offset == 0) return;
    if (Operand2::checkImm8m(offset)) {
        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, machine_regs[sp], machine_regs[sp], new Operand2(offset)));
        return;
    }
    if (Operand2::checkImm8m(-offset)) {
        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SUB, false, machine_regs[sp], machine_regs[sp], new Operand2(-offset)));
        return;
    }
    if (offset > 0) {
        for (int i = 0; i < 4; ++i) {
            int cur = offset & (0xff << (i * 8));
            if (cur)
                current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, machine_regs[sp], machine_regs[sp], new Operand2(cur)));
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            int cur = (-offset) & (0xff << (i * 8));
            if (cur)
                current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::SUB, false, machine_regs[sp], machine_regs[sp], new Operand2(cur)));
        }
    }
}

void ArmManager::ResetFuncData(RAT* rat)
{
    machine_regs.clear();
    var_of_reg.clear();
    var_of_reg.resize(RegCount + RegCount_S);
    for (int i = 0; i < RegCount + RegCount_S; ++i) {
        machine_regs.push_back(new Reg(i));
    }
    if (first_scan)
        this->local_var_offset.clear();
    var_map.clear();
    stack_size = 0;
    this->rat = rat;
    current_line_no = 0;
    current_block = nullptr;
    occupied_regs.clear();
    rparam_regs.clear();
    dst_reg = nullptr;
    visited.clear();
    block_table.clear();
}

bool ArmManager::gen_block(IRBlock *block, ArmFunc*& func)
{
    if (func == nullptr)
        return false;//assert(0);
    stringstream ss;
    ss << ".L" << block->get_index();
    // cout << ss.str() << endl;
    ArmBlock* arm_block = new ArmBlock(ss.str());
    block_table.emplace(block->get_index(), arm_block);
    current_block = arm_block;
    func->push_back(current_block);
    const auto &program = block->get_instr_list_const();
    // 在基本块开头记录当前栈大小，并在基本块结尾恢复
    int stack_size_start = stack_size;
    unordered_set<IRSymbol*> allocate_var;
    for (auto iter = local_var_offset.begin(); iter != local_var_offset.end(); iter++)
        allocate_var.emplace(iter->first);
    for (const auto &instr : program) {
        unordered_set<Reg*> reg_list;
        // 如果寄存器分配发生溢出，则需要使用别的变量占用的寄存器，使用完后再还回去
        // 对于目的寄存器：push->执行指令->store->pop
        // 对于源寄存器: push->load->执行指令->pop
        vector<Reg*> push_reg;
        current_line_no = instr.no() - rat->get_start_no();
        update_occupied_regs(nullptr, nullptr, nullptr);
        if (instr.type() == IRType::BinaryCalc) {
            // r = a op b
            auto a = instr.a();
            auto b = instr.b();
            auto r = instr.r();
            if (r->kind() != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no)) {
                continue;
            }
            CreateBinaryInst(instr.op(), r, a, b, reg_list, push_reg);
        } else if (instr.type() == IRType::UnaryCalc) {
            auto a = instr.a();
            auto r = instr.r();
            if (r->kind() != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no))
                continue;
            CreateUnaryInst(instr.op(), r, a, reg_list, push_reg);
        } else if (instr.type() == IRType::TernaryCalc) {
            // r = a op b sop c
            auto r = instr.r();
            auto a = instr.a();
            auto b = instr.b();
            auto c = instr.c();
            if (r->kind() != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no)) {
                continue;
            }
            CreateTernaryInst(instr.op(), instr.sop(), r, a, b, c, reg_list, push_reg);
        } else if (instr.type() == IRType::BlockGoto) {
            int offset = stack_size - stack_size_start;
            update_sp(BinaryOp::ADD, offset, allocate_var);
            stack_size = stack_size_start;
            // 如果本块的false结点没被访问过，那么下一个就会被访问到，因此不需要显式branch
            std::stringstream block_false;
            block_false << ".L" << block->get_succ(0)->get_index();
            current_block->push_back(ArmInstr::create_branch(Cond::AL, false, block_false.str()));
            return false;
        } else if (instr.type() == IRType::BlockCondGoto) {
            auto a = instr.a();
            auto rn = Symbol2Reg(a, false, reg_list, push_reg);
            auto operand2 = new Operand2(0);
            // 比较a和0的大小
            // if a != 0, goto block->get_succ(1)
            // else, goto block->get_succ(0)
            if (a->basic_type() == BasicType::Int)
                current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::CMP, false, nullptr, rn, operand2));
            else {
                current_block->push_back(ArmInstr::create_cmp_with_0(Cond::AL, rn));
                current_block->push_back(ArmInstr::create_move_flags(Cond::AL));
            }
            for (auto reg: push_reg) {
                load_var(var_of_reg[reg->get_reg_id()], reg);
            }
            int offset = stack_size - stack_size_start;
            update_sp(BinaryOp::ADD, offset, allocate_var);
            stack_size = stack_size_start;
            std::stringstream block_true;
            block_true << ".L" << block->get_succ(1)->get_index();
            std::stringstream block_false;
            block_false << ".L" << block->get_succ(0)->get_index();
            // 自己跳自己且条件为false
            if (block->get_succ(0) == block) {
                current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_false.str()));
                current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_true.str()));
            } else {
                current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_true.str()));
                current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_false.str()));
            }
            return false;
        } else if (instr.type() == IRType::BlockBinaryGoto) {
            CreateBinaryInst(instr.op(), nullptr, instr.a(), instr.b(), reg_list, push_reg);
            stringstream block_true;
            block_true << ".L" << block->get_succ(1)->get_index();
            stringstream block_false;
            block_false << ".L" << block->get_succ(0)->get_index();
            switch (instr.op()) {
                case IROper::EqualI:
                    [[fallthrough]];
                case IROper::EqualF:
                    current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_false.str()));
                    break;
                case IROper::NotEqualI:
                    [[fallthrough]];
                case IROper::NotEqualF:
                    current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_false.str()));
                    break;
                case IROper::GreaterI:
                    [[fallthrough]];
                case IROper::GreaterF:
                    current_block->push_back(ArmInstr::create_branch(Cond::GT, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::LE, false, block_false.str()));
                    break;
                case IROper::LessI:
                    [[fallthrough]];
                case IROper::LessF:
                    current_block->push_back(ArmInstr::create_branch(Cond::LT, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::GE, false, block_false.str()));
                    break;
                case IROper::GreaterEqualI:
                    [[fallthrough]];
                case IROper::GreaterEqualF:
                    current_block->push_back(ArmInstr::create_branch(Cond::GE, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::LT, false, block_false.str()));
                    break;
                case IROper::LessEqualI:
                    [[fallthrough]];
                case IROper::LessEqualF:
                    current_block->push_back(ArmInstr::create_branch(Cond::LE, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::GT, false, block_false.str()));
                    break;
                default:
                    // 条件判断为算术运算
                    current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_true.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_false.str()));
                    break;
            }
            return false;
        } else if (instr.type() == IRType::BlockUnaryGoto) {
            auto a = instr.a();
            CreateUnaryInst(instr.op(), nullptr, instr.a(), reg_list, push_reg);
            stringstream block_true;
            block_true << ".L" << block->get_succ(1)->get_index();
            stringstream block_false;
            block_false << ".L" << block->get_succ(0)->get_index();
            switch (instr.op()) {
                case IROper::NotI:
                    [[fallthrough]];
                case IROper::NotF:
                    current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_false.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_true.str()));
                    break;
                default:
                    current_block->push_back(ArmInstr::create_branch(Cond::EQ, false, block_false.str()));
                    current_block->push_back(ArmInstr::create_branch(Cond::NE, false, block_true.str()));
                    break;
            }
            return false;
        } else if (instr.type() == IRType::RParam) {
            // nothing to do
        } else if (instr.type() == IRType::Call || instr.type() == IRType::CallWithRet) {
            //生成BL指令
            current_block->push_back(ArmInstr::create_branch(Cond::AL, true, instr.a()->global_sym()->name()));
            current_func->used_regs.emplace(machine_regs[lr]);
        } else if (instr.type() == IRType::Return || instr.type() == IRType::ValReturn) {
            // Operand2* operand2 = nullptr;
            // if (instr.type() == IRType::ValReturn) {
            //     operand2 = Symbol2Operand2(instr.a(), reg_list, push_reg);
            //     // mov r0, op2
            //     if (instr.a()->basic_type() == BasicType::Int)
            //         current_block->push_back(ArmInstr::create_move(Cond::AL, false, machine_regs[r0], operand2, false));
            //     else
            //         current_block->push_back(ArmInstr::create_float_move(Cond::AL, machine_regs[s0], operand2->get_reg()));
            //     for (auto reg: push_reg)
            //         load_var(var_of_reg[reg->get_reg_id()], reg);
            // }
            int offset = stack_size - stack_size_start;
            update_sp(BinaryOp::ADD, offset, allocate_var);
            stack_size = stack_size_start;
            return true;
        } else if (instr.type() == IRType::ArrayLoad) {
            auto r = instr.r();
            auto a = instr.a();
            auto b = instr.b(); 
            if (r->kind() != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no))
                continue;
            update_occupied_regs(a, b, r);
            // r = a[b];
            // load r, [sp, offset]
            auto rd = Symbol2Reg(r, true, reg_list, push_reg);
            // 检查偏移是否为常数
            if (b->kind() == IRSymbolKind::Value) {
                if (a->kind() == IRSymbolKind::Local || a->kind() == IRSymbolKind::Param || a->kind() == IRSymbolKind::Temp) {
                    int offset = b->value().int_value * 4;
                    // 加上数组相对sp的偏移就是总偏移
                    if (a->kind() == IRSymbolKind::Local && a->array_length() > 0) {
                        offset += stack_size - local_var_offset.at(a);
                        // auto base = Symbol2Reg(a, false, reg_list, push_reg);
                        if (r->basic_type() == BasicType::Int)
                            GenImmLdrStrInst(InstrType::Load, rd, machine_regs[sp], offset, reg_list, push_reg);
                        else
                            GenImmLdrStrInst(InstrType::LoadS, rd, machine_regs[sp], offset, reg_list, push_reg);
                    } else {
                        auto base = Symbol2Reg(a, false, reg_list, push_reg);
                        if (r->basic_type() == BasicType::Int)
                            GenImmLdrStrInst(InstrType::Load, rd, base, offset, reg_list, push_reg);
                        else
                            GenImmLdrStrInst(InstrType::LoadS, rd, base, offset, reg_list, push_reg);
                    }
                } else if (a->kind() == IRSymbolKind::Global) {
                    int offset = b->value().int_value * 4;
                    if (r->basic_type() == BasicType::Int) {
                        current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, rd, a->global_sym()->name()));
                        GenImmLdrStrInst(InstrType::Load, rd, rd, offset, reg_list, push_reg);
                    }
                    else {
                        auto base = find_free_reg(reg_list, push_reg);
                        current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, base, a->global_sym()->name()));
                        GenImmLdrStrInst(InstrType::LoadS, rd, base, offset, reg_list, push_reg);
                    }
                }
            } else {
                if (a->kind() == IRSymbolKind::Local || a->kind() == IRSymbolKind::Param || a->kind() == IRSymbolKind::Temp) {
                    auto offset = Symbol2Reg(b, false, reg_list, push_reg);
                    auto base = Symbol2Reg(a, false, reg_list, push_reg);
                    // move offset, offset, lsl #2
                    Shift* shift = new Shift(ShiftOp::LSL, 2);
                    Operand2* operand2 = new Operand2(offset, shift);
                    if (r->basic_type() == BasicType::Int)
                        current_block->push_back(ArmInstr::create_load(Cond::AL, rd, base, operand2));
                    else {
                        auto base_add_offset = find_free_reg(reg_list, push_reg);
                        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, base_add_offset, base, operand2));
                        current_block->push_back(ArmInstr::create_float_load(Cond::AL, rd, base_add_offset));
                    }
                } else if (a->kind() == IRSymbolKind::Global) {
                    auto offset = Symbol2Reg(b, false, reg_list, push_reg);
                    Shift* shift = new Shift(ShiftOp::LSL, 2);
                    Operand2* operand2 = new Operand2(offset, shift);
                    if (r->basic_type() == BasicType::Int) {
                        Reg* base = nullptr;
                        if (offset != rd)
                            base = rd;
                        else {
                            // base = machine_regs[lr];
                            // current_func->used_regs.emplace(base);
                            base = find_free_reg(reg_list, push_reg);
                        }
                        current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, base, a->global_sym()->name()));
                        current_block->push_back(ArmInstr::create_load(Cond::AL, rd, base, operand2));
                    }
                    else {
                        auto base_add_offset = find_free_reg(reg_list, push_reg);
                        current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, base_add_offset, a->global_sym()->name()));
                        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, base_add_offset, base_add_offset, operand2));
                        current_block->push_back(ArmInstr::create_float_load(Cond::AL, rd, base_add_offset));
                    }
                }
            }
            if (r->kind() == IRSymbolKind::Global || rat->is_spill(r))
                store_var(r, rd, reg_list, push_reg);
            for (auto reg: push_reg)
                load_var(var_of_reg[reg->get_reg_id()], reg);
        } else if (instr.type() == IRType::ArrayStore) {
            auto r = instr.r();
            auto a = instr.a();
            auto b = instr.b();
            if (r->kind() != IRSymbolKind::Local && r->kind()  != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no))
                continue;
            update_occupied_regs(a, b, r, false);
            // r[a] = b;
            // store b, [base, offset]
            auto rd = Symbol2Reg(b, false, reg_list, push_reg);
            // 检查偏移是否为常数
            if (a->kind() == IRSymbolKind::Value) {
                if (r->kind() == IRSymbolKind::Local || r->kind() == IRSymbolKind::Param || r->kind() == IRSymbolKind::Temp) {
                    int offset = a->value().int_value * 4;
                    if (r->kind() == IRSymbolKind::Local && r->array_length() > 0) {
                        offset += stack_size - local_var_offset.at(r);
                        // auto base = Symbol2Reg(r, false, reg_list, push_reg);
                        if (r->basic_type() == BasicType::Int)
                            GenImmLdrStrInst(InstrType::Store, rd, machine_regs[sp], offset, reg_list, push_reg);
                        else
                            GenImmLdrStrInst(InstrType::StoreS, rd, machine_regs[sp], offset, reg_list, push_reg);
                    } else {
                        auto base = Symbol2Reg(r, false, reg_list, push_reg);
                        if (b->basic_type() == BasicType::Int)
                            GenImmLdrStrInst(InstrType::Store, rd, base, offset, reg_list, push_reg);
                        else {
                            if (rd->get_reg_id() < 16)
                                ArmManager::arm_error("29", 29);
                            GenImmLdrStrInst(InstrType::StoreS, rd, base, offset, reg_list, push_reg);
                        }
                    }
                } else if (r->kind() == IRSymbolKind::Global) {
                    int offset = a->value().int_value * 4;
                    store_global_array(r, rd, new Operand2(offset), reg_list, push_reg);
                }
            } else {
                if (r->kind() == IRSymbolKind::Local || r->kind() == IRSymbolKind::Param || r->kind() == IRSymbolKind::Temp) {
                    auto offset = Symbol2Reg(a, false, reg_list, push_reg);
                    auto base = Symbol2Reg(r, false, reg_list, push_reg);
                    // move offset, offset, lsl #2
                    Shift* shift = new Shift(ShiftOp::LSL, 2);
                    Operand2* operand2 = new Operand2(offset, shift);
                    if (b->basic_type() == BasicType::Int)
                        current_block->push_back(ArmInstr::create_store(Cond::AL, rd, base, operand2));
                    else {
                        auto base_add_offset = find_free_reg(reg_list, push_reg);
                        current_block->push_back(ArmInstr::create_binary_calc(Cond::AL, BinaryOp::ADD, false, base_add_offset, base, operand2));
                        current_block->push_back(ArmInstr::create_float_store(Cond::AL, rd, base_add_offset));
                    }
                } else if (r->kind() == IRSymbolKind::Global) {
                    auto offset = Symbol2Reg(a, false, reg_list, push_reg);
                    Shift* shift = new Shift(ShiftOp::LSL, 2);
                    Operand2* operand2 = new Operand2(offset, shift);
                    store_global_array(r, rd, operand2, reg_list, push_reg);
                }
            }
            for (auto reg: push_reg)
                load_var(var_of_reg[reg->get_reg_id()], reg);
        } else if (instr.type() == IRType::Load) {
            // r = [a]
            auto r = instr.r();
            auto a = instr.a();
            if (r->kind() != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no))
                continue;
            auto rd = Symbol2Reg(r, true, reg_list, push_reg);
            if (a->kind() != IRSymbolKind::Memory) {
                auto base = Symbol2Reg(a, false, reg_list, push_reg);
                current_block->push_back(ArmInstr::create_load(Cond::AL, rd, base));
            }
            else if (a->index() < 0) {
                int offset = stack_size + a->index() * 4;
                if (rd->get_reg_id() < RegCount)
                    GenImmLdrStrInst(InstrType::Load, rd, machine_regs[sp], offset, reg_list, push_reg);
                else
                    GenImmLdrStrInst(InstrType::LoadS, rd, machine_regs[sp], offset, reg_list, push_reg);
            }
            else {
                load_var(a, rd);
            }
            if (r->kind() == IRSymbolKind::Global || rat->is_spill(r))
                store_var(r, rd, reg_list, push_reg);
            for (auto reg: push_reg)
                load_var(var_of_reg[reg->get_reg_id()], reg);
        } else if (instr.type() == IRType::Store) {
            // [r] = a
            auto r = instr.r();
            auto a = instr.a();
            auto rd = Symbol2Reg(a, false, reg_list, push_reg);
            if (r->kind() != IRSymbolKind::Memory) {
                auto base = Symbol2Reg(r, false, reg_list, push_reg);
                current_block->push_back(ArmInstr::create_store(Cond::AL, rd, base));
            }
            else if (r->index() >= 0) {
                store_var(r, rd, reg_list, push_reg);
            } else {
                int offset;
                // 函数实参存储到被调函数的栈空间
                if (current_func->fparams_in_stack.find(r) == current_func->fparams_in_stack.end())
                    offset = r->index() * 4;
                else    // 形参存储到函数的栈空间
                    offset = stack_size + r->index() * 4;
                if (a->basic_type() == BasicType::Int || a->array_length() != -1)
                    GenImmLdrStrInst(InstrType::Store, rd, machine_regs[sp], offset,  reg_list, push_reg);
                else
                    GenImmLdrStrInst(InstrType::StoreS, rd, machine_regs[sp], offset,  reg_list, push_reg);
            }

            for (auto reg: push_reg)
                load_var(var_of_reg[reg->get_reg_id()], reg);
        } else if (instr.type() == IRType::Assign) {
            auto r = instr.r();
            auto a = instr.a();
            if (r->kind() != IRSymbolKind::Global && !rat->test_sym_live(r, current_line_no))
                continue;
            update_occupied_regs(a, nullptr, r);
            Operand2* operand2 = nullptr;
            auto rd = Symbol2Reg(r, true, reg_list, push_reg);
            if (a->kind() == IRSymbolKind::Global) {
                // 全局变量视为地址
                current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, rd, a->global_sym()->name()));
            } else if (a->kind() == IRSymbolKind::Local && a->array_length() > 0) {
                load_var(a, rd);
            } else {
                if (rd->get_reg_id() < 16) {
                    if (a->kind() == IRSymbolKind::Value) {
                        int imm = a->value().int_value;
                        if (Operand2::checkImm8m(imm)) {  // mov
                            current_block->push_back(ArmInstr::create_move(Cond::AL, false, rd, new Operand2(imm), false));
                        } else if (imm < 0 && Operand2::checkImm8m(-imm - 1)) {  // mvn
                            current_block->push_back(ArmInstr::create_move(Cond::AL, false, rd, new Operand2(-imm - 1), true));
                        } else {  // ldr-pseudo
                            current_block->push_back(ArmInstr::create_ldrPseudo(Cond::AL, rd, imm));
                        }
                    } else {
                        operand2 = Symbol2Operand2(a, reg_list, push_reg);
                        current_block->push_back(ArmInstr::create_move(Cond::AL, false, rd, operand2, false));
                    }
                }
                else {
                    if (false/*a->kind() == IRSymbolKind::Value && ArmInstr::checkFpconst(a->value().float_value)*/)
                        current_block->push_back(ArmInstr::create_float_move(Cond::AL, rd, a->value().float_value));
                    else {
                        auto rm = Symbol2Reg(a, false, reg_list, push_reg);
                        current_block->push_back(ArmInstr::create_float_move(Cond::AL, rd, rm));
                    }
                }
            }
            if (r->kind() == IRSymbolKind::Global || rat->is_spill(r))
                store_var(r, rd, reg_list, push_reg);
            for (auto reg: push_reg)
                load_var(var_of_reg[reg->get_reg_id()], reg);
        }

    }
    return false;
}

void ArmManager::dfs_block(IRBlock* block, ArmFunc*& func, vector<Reg*> general_regs, vector<Reg*> vfp_regs, int stack_size)
{
    visited.emplace(block);
    if (!block->is_exit() && !block->is_entry()) {
        if (gen_block(block, func)) {           // block有return语句
            std::stringstream exit;
            exit << ".L" << block->get_succ(0)->get_index();
            current_block->push_back(ArmInstr::create_branch(Cond::AL, false, exit.str()));
        }
    } else if (block->is_exit()) {
        // 恢复现场统一放到退出块做
        stringstream ss;
        ss << ".L" << block->get_index();
        // cout << ss.str() << endl;
        ArmBlock* arm_block = new ArmBlock(ss.str());
        block_table.emplace(block->get_index(), arm_block);
        current_block = arm_block;
        func->push_back(current_block);
        current_block->push_back(ArmInstr::create_float_pop(Cond::AL, vfp_regs));
        if (!general_regs.empty() && general_regs.back()->get_reg_id() == lr && stack_size == 0) {
            general_regs.back() = machine_regs[pc];
        }
        current_block->push_back(ArmInstr::create_pop(Cond::AL, general_regs));

        move_sp(stack_size);
    
        // 返回主调函数, bx lr，如果pop最后是pc，可以省略
        if (!general_regs.empty() && general_regs.back() != machine_regs[pc] || general_regs.empty())
            current_block->push_back(ArmInstr::create_branch(Cond::AL, false, "lr", true));
    }
    for (int k = 0; k <= 1; ++k) {
        if (block->get_succ(k) != nullptr && visited.find(block->get_succ(k)) == visited.end()) {
            dfs_block(block->get_succ(k), func, general_regs, vfp_regs, stack_size);
        }
    }
}

void ArmManager::bfs_block(IRBlock* block, ArmFunc*& func, vector<Reg*> general_regs, vector<Reg*> vfp_regs, int stack_size)
{
    std::queue<IRBlock *> q;
    q.push(block);
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        if (!now->is_exit() && !now->is_entry()) {
            if (gen_block(now, func)) {     // 有return语句
                // current_block->push_back(ArmInstr::create_float_pop(Cond::AL, vfp_regs));
                // if (!general_regs.empty() && general_regs.back()->get_reg_id() == lr && stack_size == 0) {
                //     general_regs.back() = machine_regs[pc];
                // }
                // current_block->push_back(ArmInstr::create_pop(Cond::AL, general_regs));

                // move_sp(stack_size);
            
                // // 返回主调函数, bx lr，如果pop最后是pc，可以省略
                // if (!general_regs.empty() && general_regs.back() != machine_regs[pc] || general_regs.empty())
                //     current_block->push_back(ArmInstr::create_branch(Cond::AL, false, "lr", true));
                std::stringstream exit;
                exit << ".L" << now->get_succ(0)->get_index();
                current_block->push_back(ArmInstr::create_branch(Cond::AL, false, exit.str()));
            }
        } else if (now->is_exit()) {
            // 恢复现场统一放到退出块做
            stringstream ss;
            ss << ".L" << now->get_index();
            // cout << ss.str() << endl;
            ArmBlock* arm_block = new ArmBlock(ss.str());
            block_table.emplace(now->get_index(), arm_block);
            current_block = arm_block;
            func->push_back(current_block);
            current_block->push_back(ArmInstr::create_float_pop(Cond::AL, vfp_regs));
            if (!general_regs.empty() && general_regs.back()->get_reg_id() == lr && stack_size == 0) {
                general_regs.back() = machine_regs[pc];
            }
            current_block->push_back(ArmInstr::create_pop(Cond::AL, general_regs));

            move_sp(stack_size);
        
            // 返回主调函数, bx lr，如果pop最后是pc，可以省略
            if (!general_regs.empty() && general_regs.back() != machine_regs[pc] || general_regs.empty())
                current_block->push_back(ArmInstr::create_branch(Cond::AL, false, "lr", true));
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.emplace(now->get_succ(k));
            }
        }
    }
}

void ArmManager::gen_func(const IRUnit* func, RAT* rat, ArmFunc*& arm_func)
{
    if (arm_func == nullptr)
        return;//assert(0);
    ResetFuncData(rat);
    int cur_param_int = 0;
    int cur_param_float = 0;
    ArmBlock* prologue;
    // 用来存储在栈指针更新被占用的寄存器
    // unordered_set<Reg*> used_regs;
    // 形参列表
    vector<IRSymbol*> fparams_int;
    vector<IRSymbol*> fparams_float;
    const IRInstrList &list = func->get_definations_const();
    // 第一次遍历，处理函数形参分配栈空间相关
    for (const auto &instr: list) {
        if (instr.type() == IRType::FuncDef) {
            string func_name = instr.a()->global_sym()->get_name();
            if (first_scan)
                arm_func->set_name(func_name);
            current_func = arm_func;
            prologue = new ArmBlock(func_name);
            current_block = prologue;
            arm_func->push_back(current_block);
            for (auto s: instr.a()->get_fparam_reg())
                if (s->kind() == IRSymbolKind::Memory)
                    current_func->fparams_in_stack.emplace(s);
        } else if (instr.type() == IRType::FParam) {
            //将函数形参入栈（实际在主调函数中完成，这里是更新var_map）
            auto a = instr.a();
            if (a->basic_type() == BasicType::Int || a->array_length() != -1) {
                if (cur_param_int >= 4) {
                    stack_size += 4;
                    local_var_offset.emplace(a, stack_size);
                    current_func->var_in_stack.emplace_back(a);
                }
                cur_param_int++;
            } else {
                if (cur_param_float >= 16) {
                    stack_size += 4;
                    local_var_offset.emplace(a, stack_size);
                    current_func->var_in_stack.emplace_back(a);
                }
                cur_param_float++;
            }  
        } 
    }

    // 第二次遍历，处理前四个函数形参分配栈空间相关
    cur_param_int = 0;
    cur_param_float = 0;
    for (const auto &instr: list) {
        if (instr.type() == IRType::FParam) {
            auto a = instr.a();
            if (a->basic_type() == BasicType::Int || a->array_length() != -1) {
                if (cur_param_int < 4 && rat->is_spill(a)) {
                    stack_size += 4;
                    local_var_offset.emplace(a, stack_size);
                    current_func->var_in_stack.emplace_back(a);
                }
                cur_param_int++;
            } else {
                if (cur_param_float < 16 && rat->is_spill(a)) {
                    stack_size += 4;
                    local_var_offset.emplace(a, stack_size);
                    current_func->var_in_stack.emplace_back(a);
                }
                cur_param_float++;
            }  
        }
    }

    // 第三次遍历，将局部数组添加到var_offset
    for (const auto &instr: list) {
        if (instr.type() == IRType::LocalDecl) {
            auto a = instr.a();
            if (a->array_length() > 0) {
                stack_size += 4 * a->array_length();
                local_var_offset.emplace(a, stack_size);
                current_func->var_in_stack.emplace_back(a);
            }
        }
    }

    // 更新sp
    // 这里先预计算一下有哪些变量是spill，只给spill的分配栈空间
    vector<IRSymbol*> symbols = rat->get_ir_symbol();
    for (auto s: symbols) {
        if (s->kind() == IRSymbolKind::Local && s->array_length() > 0) {
            stack_size += 4 * s->array_length();
            local_var_offset.emplace(s, stack_size);
            current_func->var_in_stack.emplace_back(s);
        }
        else if (rat->is_spill(s) && (s->kind() == IRSymbolKind::Local || s->kind() == IRSymbolKind::Temp || s->kind() == IRSymbolKind::Param)) {
            stack_size += 4;
            local_var_offset.emplace(s, stack_size);
            current_func->var_in_stack.emplace_back(s);    
        }
        if (!rat->is_spill(s) && (s->kind() == IRSymbolKind::Local || s->kind() == IRSymbolKind::Temp)) {
            var_map.emplace(s, machine_regs[rat->reg_alloc(s)]);
            var_of_reg[rat->reg_alloc(s)] = s;
        }
    }

    if (!first_scan)
        stack_size = current_func->stack_size;        // 第二次扫描就能确定具体空间，一次分配完

    
    int total_size = stack_size + current_func->save_regs.size() * 4 + current_func->save_vfp.size() * 4;
    if ((total_size % 8) != 0) {
        total_size = (total_size + 8) / 8 * 8;
    }
    stack_size = total_size - current_func->save_regs.size() * 4 - current_func->save_vfp.size() * 4;
    move_sp(-stack_size);
    int stack_size_start = stack_size;

    current_block->push_back(ArmInstr::create_push(Cond::AL, current_func->save_regs));
    stack_size += current_func->save_regs.size() * 4;
    
    current_block->push_back(ArmInstr::create_float_push(Cond::AL, current_func->save_vfp));
    stack_size += current_func->save_vfp.size() * 4;

    if (!first_scan) {
        local_var_offset = current_func->var_offset;
    }

    // 执行函数体, 遍历每一个block
    IRBlock* entry = func->get_entry();
    block_table.emplace(entry->get_index(), current_block);
    dfs_block(entry, current_func, current_func->save_regs, current_func->save_vfp, stack_size_start);
    // bfs_block(entry, current_func, current_func->save_regs, current_func->save_vfp, stack_size_start);
    visited.clear();
    build_edge(entry);
    prologue->push_back(ArmInstr::create_branch(Cond::AL, false, prologue->get_edge(0)->get_label()));


    // 调整callee_save_regs
    if (first_scan) {
        current_func->save_regs.clear();
        for (int i = 0; i < RegCount; i++)
            if (REGISTER_USAGE[i] == callee_save && current_func->used_regs.find(machine_regs[i]) != current_func->used_regs.end())
                current_func->save_regs.emplace_back(machine_regs[i]);
        // push lr
        // if (current_func->used_regs.find(machine_regs[lr]) != current_func->used_regs.end())     
        //     current_func->save_regs.emplace_back(machine_regs[lr]);
        
        for (int i = RegCount; i < RegCount + RegCount_S; i++)
            if (REGISTER_USAGE_S[i - RegCount] == callee_save && current_func->used_regs.find(machine_regs[i]) != current_func->used_regs.end())
                current_func->save_vfp.push_back(machine_regs[i]);
    }
}

void ArmManager::build_edge(IRBlock* block)
{
    visited.emplace(block);
    auto arm_block = block_table.at(block->get_index());
    for (int k = 0; k <= 1; ++k) {
        if (block->get_succ(k) != nullptr && !block->get_succ(k)->is_exit()) {
            if (block_table.find(block->get_succ(k)->get_index()) == block_table.end()) {
                cout << block->get_succ(k)->get_index() << endl;
                assert(0);
            }
            auto edge_k = block_table.at(block->get_succ(k)->get_index());
            arm_block->set_edge(edge_k);
            edge_k->add_pred(arm_block);
            if (visited.find(block->get_succ(k)) == visited.end())
                build_edge(block->get_succ(k));
        }
    }
}

void ArmManager::gen_global_var(const IRInstrList &list, std::ostream &os)
{
    for (const auto &instr : list) {
        bool in_bss = true;
        int last_not0 = 0;
        auto symbol = instr.a()->global_sym();
        auto& init_val = symbol->get_init_value();
        for (int i = 0; i < init_val.size(); ++i) {
            if (0 != init_val[i].val.int_value || 0 != init_val[i].val.float_value) {
                in_bss = false;
                last_not0 = init_val[i].pos;
            }
        }
        if (in_bss) {
        // .comm symbol length align
            os << ".comm " << symbol->get_name() << ", " << (!symbol->is_array() ? 4 : instr.a()->array_length() * 4) << ", 4"
                << std::endl;
        } else {
            os << "\t.global " << symbol->get_name() << std::endl;
            os << "\t.data" << std::endl;
            os << "\t.align 4" << std::endl;
            os << "\t.type " << symbol->get_name() << ", %object" << std::endl;
            os << symbol->get_name() << ":" << std::endl;
            int cnt0 = 0;
            for (int i = 0; i < init_val.size(); ++i) {
                if (i == 0)
                    cnt0 = init_val[i].pos;
                else
                    cnt0 += init_val[i].pos - init_val[i - 1].pos - 1;
                if (init_val[i].val.int_value == 0) {
                    ++cnt0;
                } else {
                    if (cnt0 > 0) {
                        os << "\t.space " << cnt0 * 4 << std::endl;         //space 申请一片内存空间
                        cnt0 = 0;
                    }
                    os << "\t.word " << init_val[i].val.int_value << std::endl;
                }
            }
            int space = (instr.a()->array_length() - last_not0 - 1) * 4;
            if (space > 0) {
                os << "\t.space " << space << std::endl;
            }
        }
        os << std::endl;
    }

    // .text
  os << ".section .text" << std::endl;
  os << std::endl;
}

void ArmManager::gen_arm(const IRProgram &prog, std::list<RAT*> RATs, ArmProg*& arm_prog, ostream &os)
{
    os << ".arch armv7ve" << std::endl;
    os << ".arm" << std::endl;
    os << ".fpu vfpv4" << std::endl;
    os << std::endl;
    os << R"(
    .macro mov32, cond, reg, val
        movw\cond \reg, #:lower16:\val
        movt\cond \reg, #:upper16:\val
    .endm
    )" << std::endl;
    os << std::endl;

    auto iter = RATs.begin();
    for (auto& unit: prog) {
        if (unit.get_type() == IRUnitType::VarDef) {
            gen_global_var(unit.get_definations_const(), os);
        } else {
            ArmFunc* arm_func = new ArmFunc("default");
            first_scan = true;
            arm_prog->push_back(arm_func);
            gen_func(&unit, *iter, arm_func);
            first_scan = false;
            // 给局部变量分配空间
            int offset = 0;
            for (auto s: arm_func->var_in_stack) {
                if (s->array_length() <= 0)
                    offset += 4;
                else
                    offset += 4 * s->array_length();
                arm_func->var_offset.emplace(s, offset);
            }
            arm_func->stack_size = offset;
            // // 给每个基本块的临时变量分配空间
            // for (auto block: arm_func->get_blocks()) {
            //     int temp_offset = offset;
            //     for (auto s: block->temp_in_stack) {
            //         temp_offset += 4;
            //         offset += 4;
            //         arm_func->var_offset.emplace(s, temp_offset);
            //     }
            //     arm_func->stack_size = max(arm_func->stack_size, temp_offset);
            // }
            arm_func->clear_block();
            gen_func(&unit, *iter, arm_func);
            ++iter;
        }
    }
}

void ArmManager::arm_error(std::string msg, int error_code)
{
    std::cerr << "[Arm error] " << msg << std::endl;
    exit(ErrorCode::ARM_ERROR + error_code);
}