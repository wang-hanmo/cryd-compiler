#include <iostream>
#include <sstream>
#include <string>
#include <symbol_table.h>
#include <type_define.h>
using namespace std;

size_t SymbolTable::get_stack_size() const
{
    return m_sym_stack.size();
}
//向符号表中加入一个符号，push进符号栈，并让hash表中对应项指向这个新符号
Symbol *SymbolTable::add_sym(const Symbol &symbol)
{
    m_table.push_back(symbol);
    Symbol *sym_ptr = &(m_table.back());
    m_sym_stack.push_back(sym_ptr);
    //查找hash表中是否存了这个名字
    auto iter = m_current_sym.find(sym_ptr->name());
    //名字是第一次出现
    if (iter == m_current_sym.end())
        sym_ptr->set_next_homonym(nullptr);
    else //名字已经出现过
        sym_ptr->set_next_homonym(iter->second);
    m_current_sym[sym_ptr->name()] = sym_ptr;
    return sym_ptr;
}
std::vector<Symbol*> SymbolTable::add_runtime_functions()
{
    std::vector<Symbol*> runtime_functions;
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "memset", {ValueType(BasicType::Void,{0}),ValueType(BasicType::Int),ValueType(BasicType::Int)})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Int), "getint", {})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Int), "getch", {})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Float), "getfloat", {})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Int), "getarray", {ValueType(BasicType::Int, {0})})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Int), "getfarray", {ValueType(BasicType::Float, {0})})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "putint", {ValueType(BasicType::Int)})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "putch", {ValueType(BasicType::Int)})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "putfloat", {ValueType(BasicType::Float)})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "putarray", {ValueType(BasicType::Int), ValueType(BasicType::Int, {0})})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "putfarray", {ValueType(BasicType::Int), ValueType(BasicType::Float, {0})})));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "_sysy_starttime", { ValueType(BasicType::Int) })));
    runtime_functions.push_back(this->add_sym(Symbol(ValueType(BasicType::Void), "_sysy_stoptime", { ValueType(BasicType::Int) })));
    return runtime_functions;
}
//栈顶符号离开作用域时使用。从栈中删除它，并让hash表中对应项指向上一级同名符号
void SymbolTable::delete_top_sym()
{
    Symbol *target_sym = m_sym_stack.back();
    if (target_sym->get_next_homonym() == nullptr)
        m_current_sym.erase(target_sym->name());
    else
        m_current_sym[target_sym->name()] = target_sym->get_next_homonym();
    m_sym_stack.pop_back();
}
Symbol *SymbolTable::get_current_sym(std::string name)
{
    return m_current_sym[name];
}
void SymbolTable::print_table(std::ostream &os)
{
    for (auto &symbol : m_table)
        os << symbol.get_string() << endl;
}

void Symbol::add_int_value(int value, int index)
{
    m_init_value.push_back({ index,BasicValue::create_int(value) });
}
void Symbol::add_float_value(float value, int index)
{
    m_init_value.push_back({ index,BasicValue::create_float(value) });
}
string Symbol::get_string()
{
    stringstream res;
    res << m_name << " " << get_var_kind_string(m_var_kind) << " " << (m_is_const ? "Const " : "") << m_val_type.get_string();
    if (is_literally_initialized()) {
        res << " Init_value: ";
        for (auto &init_unit : m_init_value) {
            if (init_unit.pos == -1) {
                if (m_val_type.basic() == BasicType::Float)
                    res << init_unit.val.float_value;
                else
                    res  << init_unit.val.int_value;
            }
            else {
                if (m_val_type.basic() == BasicType::Float)
                    res << "<"<<init_unit.pos<<"> "<< init_unit.val.float_value << ",";
                else
                    res << "<" << init_unit.pos << "> " << init_unit.val.int_value << ",";
            }
            
        }
    }
    if (m_var_kind == VarKind::Func) {
        if (m_param_type.size() > 0) {
            res << " Parameter: ";
            for (auto &param : m_param_type) {
                res << param.get_string() << ", ";
            }
        }
    }
    return res.str();
}
int Symbol::get_array_int_value(int index)
{
    if (index >= m_val_type.total_length())
        symbol_error("Index is out of range.");
    auto res = std::lower_bound(m_init_value.begin(), m_init_value.end(), InitializeUnit(index, 0 ));
    if (res == m_init_value.end() || res->pos != index)
        return 0;
    else return res->val.int_value;
}
float Symbol::get_array_float_value(int index)
{
    if (index >= m_val_type.total_length())
        symbol_error("Index is out of range.");
    auto res = std::lower_bound(m_init_value.begin(), m_init_value.end(), InitializeUnit(index, 0));
    if (res == m_init_value.end() || res->pos != index)
        return 0;
    else return res->val.float_value;
}

void Symbol::symbol_error(const std::string &&error_msg)
{
    std::cerr << "[Symbol Error] : " << error_msg << std::endl;
    exit(ErrorCode::SYMBOL_ERROR);
}