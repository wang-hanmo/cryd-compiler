#pragma once
#include<vector>
#include<list>
#include<iostream>
#include<type_define.h>
#include<symbol_table.h>
#include<ast.h>
#include<tuple>
enum IRArrayLength :int
{
    IR_NONE_ARRAY = -1,
    IR_ARRAY_POINTER = 0,
};
enum class IROper : int
{
    /*
    * MIR ops
    */
    Null,       //Null是不正确的Operator，出现则说明程序出现bug
    //calculate
    AddI,AddF,        //+
    SubI,SubF,        //-
    MulI,MulF,        //*
    DivI,DivF,        ///
    ModI,             //%
    NegI,NegF,        //单目负号
    //cast
    IToF,             //类型转换 int to float
    FToI,             //类型转换 float to int
    //relation
    EqualI,EqualF,              //==
    NotEqualI,NotEqualF,        //!=
    GreaterI,GreaterF,          //>
    LessI,LessF,                //<
    GreaterEqualI, GreaterEqualF,   //>=
    LessEqualI, LessEqualF,     //<=
    //logic
    NotI,NotF,        //!
    
    /*
    * ARMv7 LIR ops
    */
    //移位指令和附带移位操作的指令
    ShiftI,      //r = b <sop> c         注意，类型是Ternary calc
    ShiftAddI,   //r = a + (b <sop> c)
    ShiftSubI,   //r = a - (b <sop> c)
    ShiftRsbI,   //r = (b <sop> c) - a
    SignedLargeMulI,   //r(hi), c(lo) = a * b   类型是Binary Calc
    RsbI,        // r = b - a
    BitAnd,      //r = a & b
    BitOr,       //r = a | b
    BitXor,      //r = a xor b
    BitClear,    //r = a & (~b)
    SmmulI,      //r = (a * b)[63:32]
    MulAddI,     //r = a + (b * c)
    SignedLargeMulAddI, // r = a + (b * c)([63:32])
    MulAddF,     //r = r + (a * b)
};
bool is_relation_oper(IROper oper);
IROper opposite_relation_oper(IROper oper);
enum class IRShiftOper : int
{
    Null,
    LSL,        //<<    逻辑左移
    LSR,        //>>>   逻辑右移
    ASR,        //>>    算术右移
};
enum class IRSymbolKind :int
{
    Unused = 0,
    Temp,
    Value,
    Local,
    Param,
    Global,
    PhiFunc,
    Register,   //前n个参数,使用寄存器符号
    Memory      //其余参数,使用栈空间符号
};
enum class IRType : int
{
    //线性IR
    Label,
    BinaryCalc,
    UnaryCalc,
    Assign,
    ArrayLoad,
    ArrayStore,
    Goto,
    CondGoto, 
    Switch,
    Call,
    CallWithRet,
    Return,
    ValReturn,
    FuncDef,
    FuncEnd,
    FParam,
    RParam,
    GlobalDecl,
    LocalDecl,
    Load,
    Store,
    
    //控制流图相关
    BlockGoto,       //uncond goto
    BlockCondGoto,   //cond goto <ta>
    PhiFunc,         //<lr> = <phifunc>   <phifunc> = phi(<l>,<l>,.....)
    MemoryConvergeMark,//memory converge <la> 
    //LIR
    TernaryCalc,
    BlockBinaryGoto,
    BlockUnaryGoto,
};
using IRJumpTarget = int;