#pragma once
#include<cfg_manager.h>
#include<symbol_table.h>
#include<iostream>
#include<type_define.h>
class IRTestManager
{
private:
    //生成形参定义字段，非数组变量length为0。index表示IR中的编号
    static void gen_fparam(BasicType type,int index,bool is_array,std::ostream& os);
    //生成局部变量定义语句，非数组变量length为0。index表示IR中的编号
    static void gen_local_var_def(BasicType type,int index,int length,std::ostream& os,std::string prefix="    ");
    //生成函数定义语句，只包含函数头，左半大括号和所有局部变量定义
    static void gen_function_def_head(const IRInstrList& list,std::ostream& os);
    //生成函数定义结束
    static void gen_function_def_tail(const IRInstrList& list,std::ostream& os);
    //对CFG的全局变量定义单元生成C语句
    static void gen_global_var_def_unit(const IRInstrList& list,std::ostream& os);
    //对控制流图生成C语句
    static void gen_cfg(IRBlock* entry,std::ostream& os);
    //对基本块生成C语句
    static void gen_block(IRBlock* block,std::ostream& os);
public:
    static void gen_c_source(const IRProgram& prog,std::ostream& os);
};
