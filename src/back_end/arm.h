#pragma once
#include <symbol_table.h>
#include <vector>
#include <list>
#include <iostream>
#include <arm_define.h>

class Reg
{
private:
    int reg_id;
    //寄存器描述符，用来跟踪哪些变量的当前值存放在此寄存器内
    std::vector<Symbol*> reg_desc;
public:
    Reg(int _reg_id): reg_id(_reg_id) {}
    Reg(RegID _reg_id): reg_id(static_cast<int>(_reg_id)) {}
    int get_reg_id() {return reg_id;}
    std::string get_string();
};

//作为Operand2的第二部分，即移位部分，可能是常数也可能是寄存器的值
class Shift 
{
private:
    ShiftOp op;
    bool is_imm;
    int imm_val;
    Reg* reg;
public:
    Shift(ShiftOp _op, int _imm_val): op(_op), is_imm(true), imm_val(_imm_val), reg(nullptr) {}
    Shift(ShiftOp _op, Reg* _reg): op(_op), is_imm(false), imm_val(0), reg(_reg) {}
    ShiftOp get_ShiftOp() {return op;}
    bool get_is_imm() {return is_imm;}
    int get_imm_val() {return imm_val;}
    Reg* get_reg() {return reg;}
    bool is_shift() {return op== ShiftOp::LSL && is_imm && 0 == imm_val;}   //左移0位等于没移
    std::string get_string();
};

//Operand2在指令中通常作为第二个操作数，具有多种形式
//1. imm8m 在芯片处理时表示一个32位数，但是它是由一个8位数通过循环移位偶数位得到
//2. Reg {, <opsh>} (移动常数个位)
//3. Reg, LSL/LSR/ASR/ROR Rs （移动位数为寄存器的值）
class Operand2
{
private:
    bool is_imm8m;
    int imm_val;
    Reg* reg;
    Shift* shift;
public:
    Operand2(int _imm_val): is_imm8m(true), imm_val(_imm_val), reg(nullptr), shift(nullptr) {}
    Operand2(Reg* _reg): is_imm8m(false), imm_val(0), reg(_reg), shift(nullptr) {}
    Operand2(Reg* _reg, Shift* _shift): is_imm8m(false), imm_val(0), reg(_reg), shift(_shift) {}
    bool get_is_imm8m() {return is_imm8m;}
    static bool checkImm8m(int imm);
    int get_imm_val() {return imm_val;}
    Reg* get_reg() {return reg;}
    Shift* get_shift() {return shift;}
    std::string get_string();
    void set_shift(Shift* shift) {this->shift = shift;}
};

class ArmInstr
{
private:
    Cond cond;              // 用于条件执行，条件标记满足cond指示的条件时指令才执行
    InstrType type;         // 指令类型
    BinaryOp op;            // 用于二元运算
    bool s;                 // s=1则更新条件标记
    bool is_l;              // 用于branch，is_l=1表示链接
    bool is_mvn;
    Reg* rn;                // 第一个源操作寄存器
    Reg* rm;                // 第二个源操作寄存器（在浮点指令里是sm）
    Reg* rd;                // 目标寄存器
    Reg* rd_hi;             // 存放smull指令的高32位
    Operand2* operand2;     // 在load/store中作为reg_offset
    std::string label;      // 用于branch指令
    int imm;                // 用于ldrpseudo伪指令
    bool is_x;              // 用于return，bx lr
    float fpconst;           // 用于浮点指令，m*2^(-n),16<=m<=31,0<=n<=7
    std::vector<Reg*> reg_list;
    ArmInstr(InstrType _type, Cond _cond = Cond::AL, BinaryOp _op = BinaryOp::NONE, bool _s = false,
        Reg* _rd = nullptr, Reg* _rn = nullptr, Operand2* _operand2 = nullptr, bool is_mvn = false,
        std::string _label = "", int _imm = -1, bool _is_x = false, bool _is_l = false, std::vector<Reg*> _reg_list = {}, Reg* rd_hi = nullptr);

    ArmInstr(InstrType _type, Cond _cond = Cond::AL, BinaryOp _op = BinaryOp::NONE, Reg* _sd = nullptr,
        Reg* _sn = nullptr, Reg* _sm = nullptr, int immed = -1, float _fpconst = -1, std::vector<Reg*> _reg_list = {});

public:
    static ArmInstr create_binary_calc(Cond cond, BinaryOp op, bool s, Reg* rd, Reg* rn, Operand2* operand2);
    static ArmInstr create_binary_calc(Cond cond, BinaryOp op, bool s, Reg* rd, Reg* rd_hi, Reg* rn, Operand2* operadn2);
    static ArmInstr create_move(Cond cond, bool s, Reg* rd, Operand2* operand2, bool is_mvn = false);
    static ArmInstr create_branch(Cond cond, bool is_l, std::string label, bool is_x = false);
    static ArmInstr create_load(Cond cond, Reg* rd, Reg* rn, int imm);
    static ArmInstr create_load(Cond cond, Reg* rd, Reg* rn, Operand2* operand2);
    static ArmInstr create_load(Cond cond, Reg* rd, Reg* rn);
    static ArmInstr create_store(Cond cond, Reg* rd, Reg* rn, int imm);
    static ArmInstr create_store(Cond cond, Reg* rd, Reg* rn, Operand2* operand2);
    static ArmInstr create_store(Cond cond, Reg* rd, Reg* rn);
    static ArmInstr create_ldrPseudo(Cond cond, Reg* rd, int imm);
    static ArmInstr create_ldrPseudo(Cond cond, Reg* rd, std::string label);
    static ArmInstr create_push(Cond cond, std::vector<Reg*> reg_list);
    static ArmInstr create_pop(Cond cond, std::vector<Reg*> reg_list);

    static ArmInstr create_float_binary_calc(Cond cond, BinaryOp op, Reg* sd, Reg* sn, Reg* sm);
    static ArmInstr create_float_move(Cond cond, Reg* sd, Reg* sm);
    static ArmInstr create_float_move(Cond cond, Reg* sd, float fpconst);
    static ArmInstr create_float_to_int(Cond cond, Reg* sd, Reg* sm);
    static ArmInstr create_int_to_float(Cond cond, Reg* sd, Reg* sm);
    static ArmInstr create_cmp_with_0(Cond cond, Reg* sd);
    static ArmInstr create_float_load(Cond cond, Reg* sd, Reg* rn, int immed);
    static ArmInstr create_float_load(Cond cond, Reg* sd, Reg* rn);
    static ArmInstr create_float_store(Cond cond, Reg* sd, Reg* rn, int immed);
    static ArmInstr create_float_store(Cond cond, Reg* sd, Reg* rn);
    static ArmInstr create_float_push(Cond cond, std::vector<Reg*> reg_list);
    static ArmInstr create_float_pop(Cond cond, std::vector<Reg*> reg_list);

    // 在诸如 FPU 比较之类的指令之后，需要将浮点 N、Z、C 和 V 标志移至 APSR N、Z、C 和 V 标志。
    static ArmInstr create_move_flags(Cond cond);
    

    void gen_asm(std::ostream& os);
    void GenAsm_binary_calc(std::ostream& os);
    void GenAsm_move(std::ostream& os);
    void GenAsm_branch(std::ostream& os);
    void GenAsm_load(std::ostream& os);
    void GenAsm_store(std::ostream& os);
    void GenAsm_ldrPseudo(std::ostream& os);
    void GenAsm_push(std::ostream& os);
    void GenAsm_pop(std::ostream& os);

    void GenAsm_float_binary_calc(std::ostream& os);
    void GenAsm_float_move(std::ostream& os);
    void GenAsm_float_to_int(std::ostream& os);
    void GenAsm_int_to_float(std::ostream& os);
    void GenAsm_cmp_with_0(std::ostream& os);
    void GenAsm_float_load(std::ostream& os);
    void GenAsm_float_store(std::ostream& os);
    void GenAsm_float_push(std::ostream& os);
    void GenAsm_float_pop(std::ostream& os);

    static bool checkImm12(int imm) { return (imm < 4096) && (imm > -4096); }
    static bool checkImmed(int imm) { return (imm % 4 == 0) && (imm <= 1020); }
    static bool checkFpconst(float imm);

    Cond get_cond()             {return cond;}
    InstrType get_type()        {return type;}
    BinaryOp get_op()           {return op;}
    bool get_s()                {return s;}
    bool get_is_l()             {return is_l;}
    bool get_is_x()             {return is_x;}
    Reg* get_rn()               {return rn;}
    Reg* get_rd()               {return rd;}
    Reg* get_rm()               {return rm;}
    Operand2* get_operand2()    {return operand2;}
    std::string get_label()     {return label;}
    int get_imm()               {return imm;}

    void set_reg_list(std::vector<Reg*> reg_list)   {this->reg_list = reg_list;}
    void set_cond(Cond cond) {this->cond = cond;}
    void set_label(std::string label) {this->label = label;}
};
using ArmInstrList = std::list<ArmInstr>;