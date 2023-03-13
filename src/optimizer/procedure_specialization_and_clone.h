#pragma once
#include <optimizer.h>
#include <cfg.h>
#include <set>
/*
*   过程特殊化和克隆
*   第一个版本只针对返回值未被利用的情况进行特殊化,不包含克隆
*/
class ProcedureSpecializationAndClone final :public Pass
{
private:
    void procedure_specialization_and_clone(IRUnit* unit);
    //初始化
    void init_calling_info(IRUnit* unit);
    //函数数量
    int m_func_count;
    //函数的返回值是否被使用了
    //0：函数未被调用过
    //1：函数的返回值总是被使用
    //2：函数的返回值总是不被使用
    //3: 函数的返回值有时被使用
    std::vector<int> m_return_value_use_info;
    //返回语句列表
    std::vector<std::vector<IRInstr*>> m_return_instrs;
public:
    ProcedureSpecializationAndClone(bool i_emit) :Pass(PassType::ProcedureSpecializationAndClone,PassTarget::CFG, i_emit) {}
    void run();
};