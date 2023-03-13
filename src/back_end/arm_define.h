#pragma once
#include <iostream>
//armv7 寄存器相关信息
/* -------------start-------------- */

enum RegID : int
{
    //函数形参或者普通寄存器或者函数返回值
    r0 = 0,
    //函数形参或者普通寄存器
    r1 = 1,
    r2 = 2,
    r3 = 3,
    //局部变量
    r4 = 4,
    r5 = 5,
    r6 = 6,
    r7 = 7,
    r8 = 8,
    r9 = 9,
    r10 = 10,
    //特殊用途
    r11 = 11,
    r12 = 12,
    r13 = 13,
    r14 = 14,
    r15 = 15,
    // 32位浮点数寄存器
    s0 = 16,
    s1 = 17,
    s2 = 18,
    s3 = 19,
    s4 = 20,
    s5 = 21,
    s6 = 22,
    s7 = 23,
    s8 = 24,
    s9 = 25,
    s10 = 26,
    s11 = 27,
    s12 = 28,
    s13 = 29,
    s14 = 30,
    s15 = 31,
    s16 = 32,
    s17 = 33,
    s18 = 34,
    s19 = 35,
    s20 = 36,
    s21 = 37,
    s22 = 38,
    s23 = 39,
    s24 = 40,
    s25 = 41,
    s26 = 42,
    s27 = 43,
    s28 = 44,
    s29 = 45,
    s30 = 46,
    s31 = 47,
    //特殊用途寄存器别名
    fp = r11,  // 栈基指针寄存器
    ip = r12,  // ipc scratch
    sp = r13,  // 栈指针寄存器
    lr = r14,  // 链接寄存器
    pc = r15,  // 程序计数器
};

enum RegisterUsage { caller_save, callee_save, special };

//通用寄存器组
const int RegCount = 16;

const RegisterUsage REGISTER_USAGE[RegCount] = {
    caller_save, caller_save, caller_save, caller_save,               // r0...r3
    callee_save, callee_save, callee_save, callee_save, callee_save,  // r4...r8
    callee_save,                                                      // r9
    callee_save, callee_save,                       // r10, r11
    caller_save, special, callee_save, special  // r12...r15
};

const int ARGUMENT_REGISTER_COUNT = 4;

const RegID ARGUMENT_REGISTERS[ARGUMENT_REGISTER_COUNT] = {RegID::r0, RegID::r1, RegID::r2, RegID::r3};

const int ALLOCATABLE_REGISTER_COUNT = 14;

const RegID ALLOCATABLE_REGISTERS[ALLOCATABLE_REGISTER_COUNT] = \
    {RegID::r0, RegID::r1, RegID::r2, RegID::r3, RegID::r12, RegID::r14,\
     RegID::r4, RegID::r5, RegID::r6, RegID::r7, RegID::r8, RegID::r9, RegID::r10, RegID::r11};


//单精度浮点数寄存器组
const int RegCount_S = 32;

const RegisterUsage REGISTER_USAGE_S[RegCount_S] = {
    caller_save, caller_save, caller_save, caller_save,               // s0...s3
    caller_save, caller_save, caller_save, caller_save,               // s4...s7
    caller_save, caller_save, caller_save, caller_save,               // s8...s11
    caller_save, caller_save, caller_save, caller_save,               // s12...s15
    callee_save, callee_save, callee_save, callee_save,               // s16...s19
    callee_save, callee_save, callee_save, callee_save,               // s20...s23
    callee_save, callee_save, callee_save, callee_save,               // s24...s27
    callee_save, callee_save, callee_save, callee_save,               // s28...s31
};

const int ARGUMENT_REGISTER_COUNT_S = 16;

const RegID ARGUMENT_REGISTERS_S[ARGUMENT_REGISTER_COUNT_S] = {\
    RegID::s0, RegID::s1, RegID::s2, RegID::s3,RegID::s4, RegID::s5, RegID::s6, RegID::s7,\
    RegID::s8, RegID::s9, RegID::s10, RegID::s11,RegID::s12, RegID::s13, RegID::s14, RegID::s15\
    };

const int ALLOCATABLE_REGISTER_COUNT_S = 32;

const RegID ALLOCATABLE_REGISTERS_S[ALLOCATABLE_REGISTER_COUNT_S] = {\
    RegID::s0, RegID::s1, RegID::s2, RegID::s3,RegID::s4, RegID::s5, RegID::s6, RegID::s7,\
    RegID::s8, RegID::s9, RegID::s10, RegID::s11,RegID::s12, RegID::s13, RegID::s14, RegID::s15,\
    RegID::s16, RegID::s17, RegID::s18, RegID::s19,RegID::s20, RegID::s21, RegID::s22, RegID::s23,\
    RegID::s24, RegID::s25, RegID::s26, RegID::s27,RegID::s28, RegID::s29, RegID::s30, RegID::s31\
    };

/* -------------end-------------- */

enum class InstrType: int
{
    BinaryCalc,     // <OpCode>{S}{Cond} Rd, Rn, <Operand2>
    Move,           // Move: MOV{S}{Cond} Rd, <Operand2>
    Branch,         // B{L}{Cond} <label>
    Load,           // <op>{size} rd, rn {, #<imm12>} OR rd , rn, +/- rm {, <opsh>}
    Store,          // <op>{size} rd, rn {, #<imm12>} OR rd , rn, +/- rm {, <opsh>}
    LdrPseudo,      // LDR伪指令
    Push,           // <op> <reglist>
    Pop,            // <op> <reglist>

    BinaryCalcS,
    MoveS,
    FloatToInt,
    IntToFloat,
    CmpWith0,
    LoadS,
    StoreS,
    PushS,
    PopS,
};

enum class BinaryOp: int
{
    // Caculate
    ADD,    // +
    SUB,    // -
    RSB,    // 反向减法
    MUL,    // *
    SMMUL,  // 有符号高位字乘法
    SMULL,  // 长整数有符号乘法
    MLA,    // 乘加
    SMMLA,


    // Logical
    AND,    // 与
    EOR,    // 异或
    ORR,    // 或
    RON,    // 或非
    BIC,    // 位清零

    SDIV,   // 有符号除法
    NEG,    // 取反，用于浮点

    // Compare
    CMP,  // 比较
    CMN,  // 与负数比较
    TST,  // 测试
    TEQ,  // 相等测试

    NONE,   // 不是二元运算
};

enum class ShiftOp: int
{
    ASR,  // arithmetic right
    LSL,  // logic left
    LSR,  // logic right
    ROR,  // rotate right
    RRX   // rotate right one bit with extend
};

enum class Cond: int
{
    AL,  // 无条件执行,通常省略
    EQ,
    NE,
    GT,
    GE,
    LT,
    LE,
};

std::string CondToString(Cond cond);
Cond GetOppositeCond(Cond cond);