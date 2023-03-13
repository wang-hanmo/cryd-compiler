#pragma once
#include <iostream>
#include <queue>
#include <string>
#include <type_define.h>
#include <unordered_map>
#include <vector>
class IRSymbol;
//初始化单元
struct InitializeUnit
{
    int pos;
    BasicValue val;
    InitializeUnit() {}
    InitializeUnit(int pos_, BasicValue val_):pos(pos_),val(val_) {}
    bool operator <(const InitializeUnit& rhs)const
    {
        return pos < rhs.pos;
    }
};
//由有序的初始化单元组成，不描述隐含的0。如果非数组，则只有唯一的一个单元，且pos为-1
using Initializer= std::vector<InitializeUnit>;
//符号
class Symbol
{
private:
    ValueType m_val_type;                 //值类型
    VarKind m_var_kind;                   //作用域类型
    std::string m_name;                   //符号的名字
    Symbol *m_next_homonym;               //指向作用范围更大的同名变量
    bool m_is_const;                      //是否常量
    bool m_is_internal_function{false};   //是否为内部函数
    IRSymbol* m_ir_sym{ nullptr };        //在IR符号表中对应的符号
    Initializer m_init_value;             //符号初值列表。如果非数组则只有唯一的一项
    std::vector<ValueType> m_param_type;  //形参类型
public:
    Symbol(const ValueType i_val_type, const VarKind i_var_kind, const std::string &i_name, const bool i_is_const = false) : m_val_type(i_val_type), m_var_kind(i_var_kind), m_name(i_name), m_is_const(i_is_const)
    {
    }
    //库函数用构造函数,禁止其他用途的调用
    Symbol(const ValueType i_ret_val_type, const std::string &i_name, const std::vector<ValueType> &i_param_type) : m_val_type(i_ret_val_type), m_var_kind(VarKind::Func), m_name(i_name), m_is_const(false), m_param_type(i_param_type)
    {
        m_is_internal_function = true;
    }
    VarKind get_kind() { return m_var_kind; }
    const ValueType &get_val_type() { return m_val_type; }
    bool is_array() { return m_val_type.is_array(); }
    std::string name() const { return m_name; }
    void set_next_homonym(Symbol *symbol) { m_next_homonym = symbol; }
    //是否为内部函数
    bool is_internal_function()const { return m_is_internal_function; };
    //获取高层作用域的同名符号
    Symbol *get_next_homonym() { return m_next_homonym; }
    std::string get_string();
    std::string get_name() { return m_name; }
    void set_ir_sym(IRSymbol* ir_sym) { m_ir_sym=ir_sym; }
    IRSymbol* get_ir_sym() { return m_ir_sym; }
    bool is_const() { return m_is_const; }
    //是否被字面值初始化
    bool is_literally_initialized() { return !m_init_value.empty(); }
    void add_int_value(int value,int index=-1);
    void add_float_value(float value,int index=-1);
    int get_int_value() { return m_init_value[0].val.int_value; }
    float get_float_value() { return m_init_value[0].val.float_value; }
    const Initializer& get_init_value() { return m_init_value; }
    int get_array_int_value(int index);
    float get_array_float_value(int index);
    void add_param_type(ValueType type) { m_param_type.push_back(type); }
    std::vector<ValueType> get_param_type() { return m_param_type; }
    ValueType get_param_type(int index) { return m_param_type[index]; }
    void symbol_error(const std::string &&error_msg);
};
//符号表
class SymbolTable
{
private:
    std::unordered_map<std::string, Symbol *> m_current_sym; //当前位置所有名字对应的符号
    std::vector<Symbol*> m_sym_stack;                       //符号栈
    std::deque<Symbol> m_table;                              //符号表
public:
    //加入运行时库函数
    std::vector<Symbol*> add_runtime_functions();
    size_t get_stack_size() const;
    //这两个函数在遍历抽象语法树时用
    Symbol *add_sym(const Symbol &symbol);     //向符号表中加入一个符号，push进符号栈，并让hash表中对应项指向这个新符号
    void delete_top_sym();                     //栈顶符号离开作用域时使用。从栈中删除它，并让hash表中对应项指向上一级同名符号
    Symbol *get_current_sym(std::string name); //获得当前名字对应的符号
    void print_table(std::ostream &os = std::cout); //打印整个表
};