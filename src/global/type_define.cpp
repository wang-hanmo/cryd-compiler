#include <type_define.h>
#include <sstream>
using namespace std;
std::string get_basic_type_string(BasicType type)
{
    switch(type){
        case BasicType::Int: return "Int";
        case BasicType::Float:return "Float";
        case BasicType::Void: return "Void";
        case BasicType::None: return "None";
        case BasicType::Uncertain: return "Uncertain";
        default: return "Unknown";break;
    }
}
std::string get_var_kind_string(VarKind kind)
{
    switch(kind){
        case VarKind::Func:return "Func";
        case VarKind::Global: return "Global";
        case VarKind::Local: return "Local";
        case VarKind::Param: return "Param";
        case VarKind::Null: return "Null";
        default: return "Unknown";break;
    } 
}
string ValueType::get_string()const
{
    stringstream res;
    res<<get_basic_type_string(m_basic_type);
    if(this->is_array()){
        res<<" ";
        for(int i = 0; i < m_length.size(); i++)
            if(m_length[i] == 0)
                res << "[]";
            else
                res << "[" << m_length[i] << "]";
    }
    return res.str();
}
std::ostream& operator <<(std::ostream& os,BasicType vtype)
{
    static const std::string table[]={"","void","i32","f32","?"};
    return os << table[(int)vtype];
}
bool ValueType::is_array()const
{
    return !m_length.empty();
}
bool ValueType::operator==(const ValueType& rhs)
{
    if(m_basic_type!=rhs.m_basic_type)
        return false;
    if(m_length.size()!=rhs.m_length.size())
        return false;
    for(int i=0;i<m_length.size();++i)
        if(m_length[i]!=rhs.m_length[i])
            return false;
    return true;
}
bool ValueType::operator!=(const ValueType& rhs)
{
    if(m_basic_type!=rhs.m_basic_type)
        return true;
    if(m_length.size()!=rhs.m_length.size())
        return true;
    for(int i=0;i<m_length.size();++i)
        if(m_length[i]!=rhs.m_length[i])
            return true;
    return false;
}
BasicType ValueType::basic()const
{
    return m_basic_type;
}
std::size_t ValueType::length(int dimension)const
{
    return m_length[dimension];
}
std::vector<std::size_t> ValueType::get_dimension()const
{
    return m_length;
}
void ValueType::set_dimension(std::vector<std::size_t> i_length)
{
    m_length = i_length;
}
void ValueType::add_dimension(std::size_t length)
{
    m_length.push_back(length);
}
size_t ValueType::total_length()const
{
    size_t len = 1;
    for(int i = 0; i < m_length.size(); i++)
        len=len*m_length[i];
    return len;
}
size_t ValueType::top_unit_length()const
{
    size_t len = 1;
    for (int i = 1; i < m_length.size(); i++)
        len = len * m_length[i];
    return len;
}
void ValueType::set_basic_type(BasicType type)
{
    m_basic_type = type;
}
void ValueType::set_array_length(int dimension,int len)
{
    m_length[dimension] = len;
}
void ValueType::pop_dimension()
{
    m_length.erase(m_length.begin());
}
BasicValue::BasicValue(int value)
{
    int_value=value;
}
BasicValue::BasicValue(float value)
{
    float_value=value;
}
BasicValue::BasicValue(void* pointer)
{
    this->pointer=pointer;
}
BasicValue BasicValue::create_int(int value)
{
    BasicValue res;
    res.int_value=value;
    return res;
}
BasicValue BasicValue::create_float(float value)
{
    BasicValue res;
    res.float_value=value;
    return res;
}
BasicValue BasicValue::create_pointer(void* pointer)
{
    BasicValue res;
    res.pointer=pointer;
    return res;
}
BasicValue BasicValue::zero()
{
    BasicValue res;
    res.int_value=0;
    return res;
}


std::string get_binary_op_string(BinaryOpFunc func)
{
    switch(func){
        case BinaryOpFunc::Assign: return "Assign";break;
        case BinaryOpFunc::Add: return "Add";break;
        case BinaryOpFunc::Sub: return "Sub";break;
        case BinaryOpFunc::Mul: return "Mul";break;
        case BinaryOpFunc::Div: return "Div";break;
        case BinaryOpFunc::Mod: return "Mod";break;
        case BinaryOpFunc::Equal: return "Equal";break;
        case BinaryOpFunc::NotEqual: return "NotEqual";break;
        case BinaryOpFunc::Less: return "Less";break;
        case BinaryOpFunc::Great: return "Great";break;
        case BinaryOpFunc::LessEqual: return "LessEqual";break;
        case BinaryOpFunc::GreatEqual: return "GreatEqual";break;
        case BinaryOpFunc::And: return "And";break;
        case BinaryOpFunc::Or: return "Or";break;
        default: return "Unknown";break;
    }
}
std::string get_unary_op_string(UnaryOpFunc func)
{
    switch(func){
        case UnaryOpFunc::Positive: return "Positive";break;
        case UnaryOpFunc::Negative: return "Negative";break;
        case UnaryOpFunc::Not: return "Not";break;
        case UnaryOpFunc::Paren: return "Paren";break;
        default: return "Unknown";break;
    }
}
std::string get_implicit_cast_string(ImplicitCastFunc func)
{
    switch(func){
        case ImplicitCastFunc::IntToFloat : return "IntToFloat";break;
        case ImplicitCastFunc::FloatToInt : return "FloatToInt";break;
        case ImplicitCastFunc::LValueToRValue : return "LValueToRValue";break;
        default: return "Unknown";break;
    }
}
