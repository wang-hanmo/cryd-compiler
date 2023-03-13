#include <arm.h>
#include <cassert>
#include <math.h>
#include <arm_manager.h>

std::string Reg::get_string()
{
    if (reg_id == RegID::sp)
        return "sp";
    else if (reg_id == RegID::lr)
        return "lr";
    else if (reg_id == RegID::pc)
        return "pc";
    else if (reg_id < 16)
        return "r" + std::to_string(reg_id);
    else
        return "s" + std::to_string(reg_id - 16);
}

std::string Shift::get_string()
{
    std::string opcode = "";
    switch (op) {
        case ShiftOp::ASR:
        opcode = "asr";
        break;
        case ShiftOp::LSL:
        opcode = "lsl";
        break;
        case ShiftOp::LSR:
        opcode = "lsr";
        break;
        case ShiftOp::ROR:
        opcode = "ror";
        break;
        case ShiftOp::RRX:
        return "rrx";
        default:
        ArmManager::arm_error("5", 5);
        break;
    }
    if (is_imm) {
        return opcode + " #" + std::to_string(imm_val);
    } else {
        return opcode + " " + reg->get_string();
    }
}

std::string Operand2::get_string()
{
    if (is_imm8m)
        return "#" + std::to_string(imm_val);
    else
        return reg->get_string() + (nullptr == shift || shift->is_shift() ? "" : ", " + shift->get_string()); 
}

bool Operand2::checkImm8m(int imm)
{
    // NOTE: assign a int to unsigned int
    unsigned int encoding = imm;
    for (int ror = 0; ror < 32; ror += 2) {
        // 如果encoding的前24bit都是0 说明imm能被表示成一个8bit循环右移偶数位得到
        if (!(encoding & ~0xFFu)) {
        return true;
        }
        encoding = (encoding << 30u) | (encoding >> 2u);
    }
    return false;
}

ArmInstr::ArmInstr( InstrType _type, Cond _cond, BinaryOp _op, bool _s, Reg* _rd, Reg* _rn, 
        Operand2* _operand2 , bool _is_mvn, std::string _label, int _imm, bool _is_x,  bool _is_l, std::vector<Reg*> _reg_list, Reg* _rd_hi):
    type(_type), cond(_cond), op(_op), s(_s), rd(_rd), rn(_rn), operand2(_operand2), 
    is_mvn(_is_mvn), label(_label), imm(_imm), is_x(_is_x), is_l(_is_l), reg_list(_reg_list), rd_hi(_rd_hi)
{
}

ArmInstr::ArmInstr(InstrType _type, Cond _cond, BinaryOp _op, Reg* _sd,
        Reg* _sn, Reg* _sm, int immed, float _fpconst, std::vector<Reg*> _reg_list):
    type(_type), cond(_cond), op(_op), rd(_sd), rn(_sn), rm(_sm), imm(immed), fpconst(_fpconst), reg_list(_reg_list)
{
}

ArmInstr ArmInstr::create_binary_calc(Cond cond, BinaryOp op, bool s, Reg* rd, Reg* rn, Operand2* operand2)
{
    ArmInstr inst = ArmInstr(InstrType::BinaryCalc, cond, op, s, rd, rn, operand2);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_binary_calc(Cond cond, BinaryOp op, bool s, Reg* rd, Reg* rd_hi, Reg* rn, Operand2* operand2)
{
    ArmInstr inst = ArmInstr(InstrType::BinaryCalc, cond, op, s, rd, rn, operand2);
    inst.rd_hi = rd_hi;
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_move(Cond cond, bool s, Reg* rd, Operand2* operand2, bool is_mvn)
{
    ArmInstr inst = ArmInstr(InstrType::Move, cond, BinaryOp::NONE, s, rd, nullptr, operand2, is_mvn);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_branch(Cond cond, bool is_l, std::string label, bool is_x)
{
    ArmInstr inst = ArmInstr(InstrType::Branch, cond, BinaryOp::NONE, false, nullptr, nullptr, nullptr, false, label, -1, is_x, is_l);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_load(Cond cond, Reg* rd, Reg* rn, int imm)
{
    ArmInstr inst = ArmInstr(InstrType::Load, cond, BinaryOp::NONE, false, rd, rn, nullptr, false, "", imm);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_load(Cond cond, Reg* rd, Reg* rn, Operand2* operand2)
{
    ArmInstr inst = ArmInstr(InstrType::Load, cond, BinaryOp::NONE, false, rd, rn, operand2);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_load(Cond cond, Reg* rd, Reg* rn)
{
    ArmInstr inst = ArmInstr(InstrType::Load, cond, BinaryOp::NONE, false, rd, rn);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_store(Cond cond, Reg* rd, Reg* rn, int imm)
{
    ArmInstr inst = ArmInstr(InstrType::Store, cond, BinaryOp::NONE, false, rd, rn, nullptr, false, "", imm);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_store(Cond cond, Reg* rd, Reg* rn, Operand2* operand2)
{
    ArmInstr inst = ArmInstr(InstrType::Store, cond, BinaryOp::NONE, false, rd, rn, operand2);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_store(Cond cond, Reg* rd, Reg* rn)
{
    ArmInstr inst = ArmInstr(InstrType::Store, cond, BinaryOp::NONE, false, rd, rn);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_ldrPseudo(Cond cond, Reg* rd, int imm)
{
    ArmInstr inst = ArmInstr(InstrType::LdrPseudo, cond, BinaryOp::NONE, false, rd, nullptr, nullptr, false, "", imm);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_ldrPseudo(Cond cond, Reg* rd, std::string label)
{
    ArmInstr inst = ArmInstr(InstrType::LdrPseudo, cond, BinaryOp::NONE, false, rd, nullptr, nullptr, false, label);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_push(Cond cond, std::vector<Reg*> reg_list)
{
    ArmInstr inst = ArmInstr(InstrType::Push, cond, BinaryOp::NONE, false, nullptr, nullptr, nullptr, false, "", -1, false, false, reg_list);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_pop(Cond cond, std::vector<Reg*> reg_list)
{
    ArmInstr inst = ArmInstr(InstrType::Pop, cond, BinaryOp::NONE, false, nullptr, nullptr, nullptr, false, "", -1, false, false, reg_list);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_float_binary_calc(Cond cond, BinaryOp op, Reg* sd, Reg* sn, Reg* sm)
{
    ArmInstr inst = ArmInstr(InstrType::BinaryCalcS, cond, op, sd, sn, sm);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 || sn && sn->get_reg_id() < 16 || sm && sm->get_reg_id() < 16)
        ArmManager::arm_error("16", 16);
    return inst;
}

ArmInstr ArmInstr::create_float_move(Cond cond, Reg* sd, Reg* sm)
{
    ArmInstr inst = ArmInstr(InstrType::MoveS, cond, BinaryOp::NONE, sd, nullptr, sm);
    //inst.gen_asm(std::cout);
    if (sd && sm && sm->get_reg_id() < 16 && sd->get_reg_id() < 16)
        ArmManager::arm_error("17", 17);
    return inst;
}

ArmInstr ArmInstr::create_float_move(Cond cond, Reg* sd, float fpconst)
{
    ArmInstr inst = ArmInstr(InstrType::MoveS, cond, BinaryOp::NONE, sd, nullptr, nullptr, -1, fpconst);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_float_to_int(Cond cond, Reg* sd, Reg* sm)
{
    ArmInstr inst = ArmInstr(InstrType::FloatToInt, cond, BinaryOp::NONE, sd, nullptr, sm);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 || sm && sm->get_reg_id() < 16)
        ArmManager::arm_error("18", 18);
    return inst;
}

ArmInstr ArmInstr::create_int_to_float(Cond cond, Reg* sd, Reg* sm)
{
    ArmInstr inst = ArmInstr(InstrType::IntToFloat, cond, BinaryOp::NONE, sd, nullptr, sm);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 || sm && sm->get_reg_id() < 16)
        ArmManager::arm_error("19", 19);
    return inst;
}

ArmInstr ArmInstr::create_cmp_with_0(Cond cond, Reg* sd)
{
    ArmInstr inst = ArmInstr(InstrType::CmpWith0, cond, BinaryOp::NONE, sd);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16)
        ArmManager::arm_error("20", 20);
    return inst;
}

ArmInstr ArmInstr::create_float_load(Cond cond, Reg* sd, Reg* rn, int immed)
{
    ArmInstr inst = ArmInstr(InstrType::LoadS, cond, BinaryOp::NONE, sd, rn, nullptr, immed);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 || rn && rn->get_reg_id() >= 16)
        ArmManager::arm_error("21", 21);
    return inst;
}

ArmInstr ArmInstr::create_float_load(Cond cond, Reg* sd, Reg* rn)
{
    ArmInstr inst = ArmInstr(InstrType::LoadS, cond, BinaryOp::NONE, sd, rn);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 || rn && rn->get_reg_id() >= 16)
        ArmManager::arm_error("22", 22);
    return inst;
}

ArmInstr ArmInstr::create_float_store(Cond cond, Reg* sd, Reg* rn, int immed)
{
    ArmInstr inst = ArmInstr(InstrType::StoreS, cond, BinaryOp::NONE, sd, rn, nullptr, immed);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 && rn && rn->get_reg_id() >= 16)
        ArmManager::arm_error("23", 23);
    if (sd && sd->get_reg_id() < 16)
        ArmManager::arm_error("24", 24);
    if (rn && rn->get_reg_id() >= 16)
        ArmManager::arm_error("25", 25);
    return inst;
}

ArmInstr ArmInstr::create_float_store(Cond cond, Reg* sd, Reg* rn)
{
    ArmInstr inst = ArmInstr(InstrType::StoreS, cond, BinaryOp::NONE, sd, rn);
    //inst.gen_asm(std::cout);
    if (sd && sd->get_reg_id() < 16 || rn && rn->get_reg_id() >= 16)
        ArmManager::arm_error("26", 26);
    return inst;
}

ArmInstr ArmInstr::create_float_push(Cond cond, std::vector<Reg*> reg_list)
{
    ArmInstr inst = ArmInstr(InstrType::PushS, cond, BinaryOp::NONE, nullptr, nullptr, nullptr, -1, -1, reg_list);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_float_pop(Cond cond, std::vector<Reg*> reg_list)
{
    ArmInstr inst = ArmInstr(InstrType::PopS, cond, BinaryOp::NONE, nullptr, nullptr, nullptr, -1, -1, reg_list);
    //inst.gen_asm(std::cout);
    return inst;
}

ArmInstr ArmInstr::create_move_flags(Cond cond)
{
    ArmInstr inst = ArmInstr(InstrType::MoveS, cond, BinaryOp::NONE, nullptr);
    //inst.gen_asm(std::cout);
    return inst;
}

void ArmInstr::gen_asm(std::ostream& os)
{
    switch(type) {
        case InstrType::BinaryCalc: {
            GenAsm_binary_calc(os);
            break;
        }
        case InstrType::Move: {
            GenAsm_move(os);
            break;
        }
        case InstrType::Branch: {
            GenAsm_branch(os);
            break;
        }
        case InstrType::Load: {
            GenAsm_load(os);
            break;
        }
        case InstrType::Store: {
            GenAsm_store(os);
            break;
        }
        case InstrType::LdrPseudo: {
            GenAsm_ldrPseudo(os);
            break;
        }
        case InstrType::Push: {
            GenAsm_push(os);
            break;
        }
        case InstrType::Pop: {
            GenAsm_pop(os);
            break;
        }
        case InstrType::BinaryCalcS: {
            GenAsm_float_binary_calc(os);
            break;
        }
        case InstrType::MoveS: {
            GenAsm_float_move(os);
            break;
        }
        case InstrType::FloatToInt: {
            GenAsm_float_to_int(os);
            break;
        }
        case InstrType::IntToFloat: {
            GenAsm_int_to_float(os);
            break;
        }
        case InstrType::CmpWith0: {
            GenAsm_cmp_with_0(os);
            break;
        }
        case InstrType::LoadS: {
            GenAsm_float_load(os);
            break;
        }
        case InstrType::StoreS: {
            GenAsm_float_store(os);
            break;
        }
        case InstrType::PushS: {
            GenAsm_float_push(os);
            break;
        }
        case InstrType::PopS: {
            GenAsm_float_pop(os);
            break;
        }
        default:
        ArmManager::arm_error("6", 6);
    }
}

void ArmInstr::GenAsm_binary_calc(std::ostream& os)
{
    std::string opcode;
    switch (op) {
        case BinaryOp::ADD:
        opcode = "add";
        break;
        case BinaryOp::SUB:
        opcode = "sub";
        break;
        case BinaryOp::RSB:
        opcode = "rsb";
        break;
        case BinaryOp::MUL:
        opcode = "mul";
        break;
        case BinaryOp::SMMUL:
        opcode = "smmul";
        break;
        case BinaryOp::MLA:
        opcode = "mla";
        break;
        case BinaryOp::AND:
        opcode = "and";
        break;
        case BinaryOp::EOR:
        opcode = "eor";
        break;
        case BinaryOp::ORR:
        opcode = "orr";
        break;
        case BinaryOp::RON:
        opcode = "ron";
        break;
        case BinaryOp::BIC:
        opcode = "bic";
        break;
        case BinaryOp::SDIV:
        opcode = "sdiv";
        break;
        case BinaryOp::CMP:
        opcode = "cmp";
        break;
        case BinaryOp::CMN:
        opcode = "cmn";
        break;
        case BinaryOp::TST:
        opcode = "tst";
        break;
        case BinaryOp::TEQ:
        opcode = "teq";
        break;
        case BinaryOp::SMMLA:
        opcode = "smmla";
        break;
        case BinaryOp::SMULL:
        opcode = "smull";
        break;
        default:
        ArmManager::arm_error("7", 7);
        break;
    }
    if (s) opcode += "s";
    opcode += CondToString(cond);
    os << "\t" << opcode << " " << (nullptr != rd ? (rd->get_string() + ", ") : "") << 
            (nullptr != rd_hi ? (rd_hi->get_string() + ", ") : "") << rn->get_string() << ", " << operand2->get_string() << std::endl;
}

void ArmInstr::GenAsm_move(std::ostream& os)
{
    std::string opcode = is_mvn ? "mvn" : "mov";
    if (s) opcode += "s";
    opcode += CondToString(cond);
    if (!is_mvn && rd->get_string() == operand2->get_string())
        return;
    os << "\t" << opcode << " " << rd->get_string();
    os << ", " << operand2->get_string() << std::endl;
}

void ArmInstr::GenAsm_branch(std::ostream& os)
{
    std::string opcode = "b";
    if (is_l) opcode += "l";
    if (is_x) opcode += "x";
    opcode += CondToString(cond);
    os << "\t" << opcode << " " << label << std::endl;
}

void ArmInstr::GenAsm_load(std::ostream& os)
{
    std::string prefix = "ldr";
    prefix += CondToString(cond) + " " + rd->get_string() + ", ";
    std::string offset ;
    if (imm != -1) {
        offset = "#" + std::to_string(imm);
        prefix += "[" + rn->get_string() + ", " + offset + "]";
    }
    else if (operand2 != nullptr) {
        offset = operand2->get_string();
        prefix += "[" + rn->get_string() + ", " + offset + "]";
    } else {
        prefix += "[" + rn->get_string() + "]";
    }
    
    os << "\t" << prefix << std::endl;
}

void ArmInstr::GenAsm_store(std::ostream& os)
{
    std::string prefix = "str";
    prefix += CondToString(cond) + " " + rd->get_string() + ", ";
    std::string offset ;
    if (imm != -1) {
        offset = "#" + std::to_string(imm);
        prefix += "[" + rn->get_string() + ", " + offset + "]";
    }
    else if (operand2 != nullptr) {
        offset = operand2->get_string();
        prefix += "[" + rn->get_string() + ", " + offset + "]";
    } else {
        prefix += "[" + rn->get_string() + "]";
    }
    
    os << "\t" << prefix << std::endl;
}

void ArmInstr::GenAsm_ldrPseudo(std::ostream& os)
{
    if (imm != -1) {
        // movw:把 16 位立即数放到寄存器的低16位，高16位清0
        // movt:把 16 位立即数放到寄存器的高16位，低 16位不影响
        if (0 == ((imm >> 16) & 0xFFFF)) {
            // 如果高16位本来就为0，直接movw
            os << "\tmovw" << CondToString(cond) << " " << rd->get_string() + ", #"
                << (imm & 0xFFFF) << std::endl;
        }
        else {
            // 如果高16位不为0，先movw，然后movt
            os << "\tmov32 " << CondToString(cond) << ", " << rd->get_string() + ", " << imm
                << std::endl;
        }
    } else {
            os << "\tmov32 " << CondToString(cond) << ", " << rd->get_string() + ", " << label
                << std::endl;
    }
}

void ArmInstr::GenAsm_push(std::ostream& os)
{
    if (reg_list.empty())
        return;
    std::string opcode = "push";
    opcode += CondToString(cond);
    os << "\t" << opcode << " {" << reg_list[0]->get_string();
    for (auto iter = reg_list.begin() + 1; iter != reg_list.end(); ++iter) {
        os << ", " << (*iter)->get_string();
    }
    os << "}" << std::endl;
}

void ArmInstr::GenAsm_pop(std::ostream& os)
{
    if (reg_list.empty())
        return;
    std::string opcode = "pop";
    opcode += CondToString(cond);
    os << "\t" << opcode << " {" << reg_list[0]->get_string();
    for (auto iter = reg_list.begin() + 1; iter != reg_list.end(); ++iter) {
        os << ", " << (*iter)->get_string();
    }
    os << "}" << std::endl;
}

// 浮点数相关
void ArmInstr::GenAsm_float_binary_calc(std::ostream &os)
{
    std::string opcode;
    switch (op) {
        case BinaryOp::ADD:
        opcode = "vadd";
        break;
        case BinaryOp::SUB:
        opcode = "vsub";
        break;
        case BinaryOp::MUL:
        opcode = "vmul";
        break;
        case BinaryOp::SMMUL:
        opcode = "vmmul";
        break;
        case BinaryOp::MLA:
        opcode = "vmla";
        break;
        case BinaryOp::SDIV:
        opcode = "vdiv";
        break;
        case BinaryOp::CMP:
        opcode = "vcmp";
        break;
        case BinaryOp::NEG:
        opcode = "vneg";
        break;
        default:
        ArmManager::arm_error("8", 8);
        break;
    }
    opcode += CondToString(cond);
    opcode += ".f32";
    if (op == BinaryOp::NEG)
        os << "\t" << opcode << " " << rd->get_string() << ", " << rm->get_string() << std::endl;    
    else
        os << "\t" << opcode << " " << (nullptr != rd ? (rd->get_string() + ", ") : "")
            << rn->get_string() << ", " << rm->get_string() << std::endl;
}

void ArmInstr::GenAsm_float_move(std::ostream &os)
{
    if (rd != nullptr) {
        std::string opcode = "vmov";
        opcode += CondToString(cond);
        if (rd->get_reg_id() >= 16 && rm != nullptr && rm->get_reg_id() >= 16 || rm == nullptr)
            opcode += ".f32";
        if (rm != nullptr && rd->get_reg_id() == rm->get_reg_id())
            return;
        os << "\t" << opcode << " " << rd->get_string();
        if (rm != nullptr)
            os << ", " << rm->get_string() << std::endl;
        else
            os << ", #" << fpconst << std::endl;
    } else {
        std::string opcode = "vmrs";
        opcode += CondToString(cond);
        os << "\t" << opcode << " APSR_nzcv, FPSCR" << std::endl;
    }
}

void ArmInstr::GenAsm_float_to_int(std::ostream &os)
{
    std::string opcode = "vcvt" + CondToString(cond) + ".s32.f32";
    os << "\t" << opcode << " " << rd->get_string() + ", " << rm->get_string() << std::endl;
}

void ArmInstr::GenAsm_int_to_float(std::ostream &os)
{
    std::string opcode = "vcvt" + CondToString(cond) + ".f32.s32";
    os << "\t" << opcode << " " << rd->get_string() + ", " << rm->get_string() << std::endl;
}

void ArmInstr::GenAsm_cmp_with_0(std::ostream &os)
{
    std::string opcode = "vcmp" + CondToString(cond) + ".f32";
    os << "\t" << opcode << " " << rd->get_string() + ", #0.0" << std::endl;
}

void ArmInstr::GenAsm_float_load(std::ostream &os)
{
    std::string opcode = "vldr" + CondToString(cond) + " " + rd->get_string() + ", ";
    std::string offset;
    if (imm != -1) {
        offset = "#" + std::to_string(imm);
        opcode += "[" + rn->get_string() + ", " + offset + "]";
    } else {
        opcode += "[" + rn->get_string() + "]";
    }
    os << "\t" << opcode << std::endl;
}

void ArmInstr::GenAsm_float_store(std::ostream &os)
{
    std::string opcode = "vstr" + CondToString(cond) + " " + rd->get_string() + ", ";
    std::string offset;
    if (imm != -1) {
        offset = "#" + std::to_string(imm);
        opcode += "[" + rn->get_string() + ", " + offset + "]";
    } else {
        opcode += "[" + rn->get_string() + "]";
    }
    os << "\t" << opcode << std::endl;
}

void ArmInstr::GenAsm_float_push(std::ostream &os)
{
    if (reg_list.empty())
        return;
    std::string opcode = "vpush";
    opcode += CondToString(cond);
    os << "\t" << opcode << " {" << reg_list[0]->get_string();
    for (auto iter = reg_list.begin() + 1; iter != reg_list.end(); ++iter) {
        os << ", " << (*iter)->get_string();
    }
    os << "}" << std::endl;
}

void ArmInstr::GenAsm_float_pop(std::ostream &os)
{
    if (reg_list.empty())
        return;
    std::string opcode = "vpop";
    opcode += CondToString(cond);
    os << "\t" << opcode << " {" << reg_list[0]->get_string();
    for (auto iter = reg_list.begin() + 1; iter != reg_list.end(); ++iter) {
        os << ", " << (*iter)->get_string();
    }
    os << "}" << std::endl;
}

bool ArmInstr::checkFpconst(float imm)
{
    for (int m = 16; m <= 31; m++)
        for (int n = 0; n <= 7; n++)
            if (imm == m * pow(2, -n))
                return true;

    return false;
}