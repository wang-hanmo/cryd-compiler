#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <type_define.h>
#include <ir_define.h>
#include <unordered_map>
#include <arm_define.h>
class IRBlock;
struct PhiParam
{
    IRSymbol* sym;  //指向参数的指针
    IRBlock* from;  //指向来源块的指针
};
//IR符号
class IRSymbol
{
private:
    IRSymbolKind    m_kind{ IRSymbolKind::Unused };     //内存类型
    BasicType       m_basic_type{BasicType::None};      //基本值类型
    int             m_array_length{ IRArrayLength::IR_NONE_ARRAY }; 
                                                        //非数组则为-1，指向数组首地址的指针为0，长度已知则>0
    union {
        Symbol*     m_global_sym{ nullptr };            //对于global var,function，指向其所在的AST符号
        BasicValue  m_value;                            //常量值
        int         m_index;                            //对于temp,local,param变量，描述其下标（即名字）
    };
    IRSymbol*       m_def_sym{nullptr};                 //SSA形式下指向原始变量定义的指针(不包括phi函数),一般形式下指向自己               
    int             m_ssa_index{-1};                    //SSA形式下额外增加的下标,-1表示没有参与SSA，或是原始变量定义符号
    std::list<PhiParam>  m_phi_param{};                 //phi函数的参数列表
    int m_tag{-1};                                      //附加信息，在各种图论算法使用，离开算法后无效
    std::vector<IRSymbol*> m_fparam_reg;
    std::vector<IRSymbol*> m_rparam_reg;
    IRSymbol* m_ret_reg{nullptr};
public:
    IRSymbol(IRSymbolKind i_kind, BasicType i_basic_type, Symbol* i_global_sym, int i_array_length) :
        m_kind(i_kind), m_basic_type(i_basic_type), m_array_length(i_array_length) {
        m_global_sym = i_global_sym;
    }
    IRSymbol(IRSymbolKind i_kind, BasicType i_basic_type, BasicValue i_value) :
        m_kind(i_kind), m_basic_type(i_basic_type),m_array_length(IRArrayLength::IR_NONE_ARRAY) {
        m_value = i_value;
    }
    IRSymbol(IRSymbolKind i_kind, BasicType i_basic_type, int i_index, int i_array_length) :
        m_kind(i_kind), m_basic_type(i_basic_type), m_array_length(i_array_length){
        m_index = i_index;
    }
    IRSymbol(IRSymbolKind i_kind, int i_index):
        m_kind(i_kind), m_index(i_index){
    }
    //get函数
    int index()const { return m_index; }
    int int_value()const { return m_value.int_value; }
    float float_value()const { return m_value.float_value; }
    BasicValue value()const { return m_value; }
    Symbol* global_sym()const { return m_global_sym; }
    BasicType basic_type()const { return m_basic_type; }
    IRSymbolKind kind()const { return m_kind; }
    int array_length()const { return m_array_length; }
    IRSymbol* def_sym()const { return m_def_sym; }
    int ssa_index()const { return m_ssa_index; }
    std::list<PhiParam>& phi_params() { return m_phi_param; }
    bool is_value()const { return m_kind == IRSymbolKind::Value; }
    bool is_global()const { return m_kind == IRSymbolKind::Global; }
    bool is_non_array()const { return m_array_length == IRArrayLength::IR_NONE_ARRAY; }
    bool is_array_or_pointer()const { return m_array_length >=0; }
    bool is_array()const { return m_array_length > 0; }
    bool is_pointer()const { return m_array_length == 0; }
    //set函数
    void set_ssa_index(int ssa_index) { m_ssa_index = ssa_index; }
    void set_index(int index) {m_index = index;}
    void set_def_sym(IRSymbol* def_sym) { m_def_sym = def_sym; }
    void add_phi_param(IRSymbol* param, IRBlock* from) { m_phi_param.push_back({ param,from }); }
    void set_basic_type(BasicType basic_type) { m_basic_type = basic_type; }
    void set_int_value(int value) {m_value.int_value = value;}
    void set_float_value(float value) {m_value.float_value = value;}
    void set_tag(int tag) { m_tag = tag; }
    void add_fparam_reg(IRSymbol* i_fparam_reg)   {m_fparam_reg.push_back(i_fparam_reg);}
    void add_rparam_reg(IRSymbol* i_rparam_reg)   {m_rparam_reg.push_back(i_rparam_reg);}
    void set_ret_reg(IRSymbol* i_ret_reg)       {m_ret_reg = i_ret_reg;}
    std::vector<IRSymbol*>& get_fparam_reg()     {return m_fparam_reg;}
    std::vector<IRSymbol*>& get_rparam_reg()     {return m_rparam_reg;}
    IRSymbol* get_ret_reg()                     {return m_ret_reg;}
    int get_tag() { return m_tag; }//attach info
    bool is_value_0()const { return m_kind == IRSymbolKind::Value && (m_basic_type == BasicType::Int ? (m_value.int_value == 0) : (m_value.float_value == 0.0f)); };
    bool is_value_1()const { return m_kind == IRSymbolKind::Value && (m_basic_type == BasicType::Int ? (m_value.int_value == 1) : (m_value.float_value == 1.0f)); };
    bool is_value_m1()const { return m_kind == IRSymbolKind::Value && (m_basic_type == BasicType::Int ? (m_value.int_value == -1) : (m_value.float_value == -1.0f)); };
    std::string get_string();
};
//IR符号表
class IRSymbolTable
{
private:
    std::deque<IRSymbol> m_table;                 //符号表
public:
    IRSymbol* add_sym(const IRSymbol& symbol);    //向符号表中加入一个符号,返回指向它的指针
    //向符号表中加入某种类型的符号，并返回指向它的指针
    IRSymbol* create_value(BasicType basic_type,BasicValue value);
    IRSymbol* create_temp(BasicType basic_type, int index, int array_length = IRArrayLength::IR_NONE_ARRAY);
    IRSymbol* create_local(BasicType basic_type, int index,int array_length = IRArrayLength::IR_NONE_ARRAY);
    IRSymbol* create_param(BasicType basic_type, int index, int array_length = IRArrayLength::IR_NONE_ARRAY);
    IRSymbol* create_global(Symbol* global_sym);
    IRSymbol* create_phi_func(BasicType basic_type);
    IRSymbol* create_ssa(IRSymbol* def_sym,int ssa_index);
    IRSymbol* create_register(int index, bool is_float = false);
    IRSymbol* create_memory(int index);
    IRSymbol* create_value_0(BasicType basic_type);
    IRSymbol* create_value_1(BasicType basic_type);
    IRSymbol* create_value_2(BasicType basic_type);
    IRSymbol* create_value_m1(BasicType basic_type);
    //void print_table();                           //打印整个表
};