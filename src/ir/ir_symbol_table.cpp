#include <iostream>
#include <sstream>
#include <string>
#include <ir_symbol_table.h>
#include <type_define.h>
#include <cassert>
using namespace std;
//向符号表中加入一个符号，push进符号栈，并让hash表中对应项指向这个新符号
IRSymbol* IRSymbolTable::add_sym(const IRSymbol& symbol)
{
    m_table.push_back(symbol);
    return &(m_table.back());
}
IRSymbol* IRSymbolTable::create_value(BasicType basic_type, BasicValue value)
{
    m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type,value));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_value_2(BasicType basic_type)
{
    assert(basic_type == BasicType::Int || basic_type == BasicType::Float);
    if (basic_type == BasicType::Int)m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, 2));
    else m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, 2.0f));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_value_1(BasicType basic_type)
{
    assert(basic_type == BasicType::Int || basic_type == BasicType::Float);
    if(basic_type==BasicType::Int)m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, 1));
    else m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, 1.0f));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_value_0(BasicType basic_type)
{
    assert(basic_type == BasicType::Int || basic_type == BasicType::Float);
    if (basic_type == BasicType::Int)m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, 0));
    else m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, 0.0f));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_value_m1(BasicType basic_type)
{
    assert(basic_type == BasicType::Int || basic_type == BasicType::Float);
    if (basic_type == BasicType::Int)m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, -1));
    else m_table.push_back(IRSymbol(IRSymbolKind::Value, basic_type, -1.0f));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_local(BasicType basic_type, int index, int array_length)
{
    m_table.push_back(IRSymbol(IRSymbolKind::Local,basic_type,index, array_length));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_param(BasicType basic_type, int index, int array_length)
{
    m_table.push_back(IRSymbol(IRSymbolKind::Param, basic_type, index, array_length));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_temp(BasicType basic_type, int index, int array_length)
{
    m_table.push_back(IRSymbol(IRSymbolKind::Temp, basic_type, index, array_length));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_global(Symbol* global_sym)
{
    int array_length = global_sym->is_array() ? global_sym->get_val_type().total_length() : IRArrayLength::IR_NONE_ARRAY;
    m_table.push_back(IRSymbol(IRSymbolKind::Global, global_sym->get_val_type().basic(), global_sym, array_length));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_phi_func(BasicType basic_type)
{
    m_table.push_back(IRSymbol(IRSymbolKind::PhiFunc, basic_type, 0));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_ssa(IRSymbol* def_sym, int ssa_index)
{
    m_table.push_back(IRSymbol(def_sym->kind(), def_sym->basic_type(), def_sym->index(), def_sym->array_length()));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(def_sym);
    res->set_ssa_index(ssa_index);
    return res;
}
IRSymbol* IRSymbolTable::create_register(int index, bool is_float)
{
    if(is_float)
    {
        m_table.push_back(IRSymbol(IRSymbolKind::Register, RegCount + index));
    }
    else
    {
        m_table.push_back(IRSymbol(IRSymbolKind::Register, index));
    }
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
IRSymbol* IRSymbolTable::create_memory(int index)
{
    m_table.push_back(IRSymbol(IRSymbolKind::Memory, index));
    IRSymbol* res = &(m_table.back());
    res->set_def_sym(res);
    return res;
}
std::string IRSymbol::get_string()
{
    stringstream res;
    res << m_basic_type<<" ";
    switch (m_kind) {
    case IRSymbolKind::Global:
        res << "@" << m_global_sym->name();
        break;
    case IRSymbolKind::Local:
        // res << "l" << m_index <<"_"<<m_ssa_index;
        res << "l" << m_index;
        break;
    case IRSymbolKind::Param:
        res << "l" << m_index;
        break;
    case IRSymbolKind::Temp:
        res << "t" << m_index;
        break;
    case IRSymbolKind::Value: {
        if (m_basic_type == BasicType::Float)
            res << m_value.float_value;
        res << m_value.int_value;
        break;
    }
    case IRSymbolKind::PhiFunc: {
        res << "phi(";
        bool first = true;
        for (auto param : m_phi_param) {
            if (first)first = false;
            else res << ", ";
            res << param.sym;
        }
        res << ")";
        break;
    }
    case IRSymbolKind::Register: {
        if(m_index >= RegCount)
            res << "s" << (m_index - RegCount);
        else
            res << "r" << m_index;
        break;
    }
    case IRSymbolKind::Memory:
        res << "mem(sp)" << m_index;
        break;
    default:
        break;
    }
    return res.str();
}
/*
void SymbolTable::print_table()
{
    for (auto &symbol : m_table)
        cout << symbol.get_string() << endl;
}

void Symbol::symbol_error(const std::string &&error_msg)
{
    std::cerr << "[Symbol Error] : " << error_msg << std::endl;
    exit(255);
}*/