#include <ast.h>
#include <cassert>
#include <iostream>
#include <symbol_table.h>
using namespace std;
extern int line_no; //全局行号变量
ASTNode *g_ast_root;
ASTType ASTNode::get_type()
{
    return m_type;
}
Symbol *ASTNode::get_symbol()
{
    return m_symbol;
}
int ASTNode::get_func()
{
    return m_func;
}
ValueType &ASTNode::get_value_type()
{
    return m_value_type;
}
BasicValue ASTNode::get_value()
{
    return m_value;
}
int ASTNode::get_line_no()
{
    return m_line_no;
}
//设置值类型
void ASTNode::set_value_type(ValueType type)
{
    m_value_type = type;
}
const std::vector<ASTNode*>& ASTNode::get_child()
{
    return m_child;
}
std::vector<ASTNode*>& ASTNode::get_child_not_const()
{
    return m_child;
}
ASTNode *ASTNode::get_child(int index)
{
    return m_child[index];
}
std::string ASTNode::get_var_name()
{
    return m_var_name;
}
//设置var_name
void ASTNode::set_var_name(std::string name)
{
    m_var_name = name;
}
bool ASTNode::is_lvalue()
{
    return m_is_lvalue;
}

void ASTNode::set_symbol(Symbol *symbol)
{
    m_symbol = symbol;
}
void ASTNode::set_child(int index, ASTNode *child)
{
    m_child[index] = child;
}
bool ASTNode::is_literal()
{
    return m_is_literal;
}
void ASTNode::set_is_lvalue(bool is_lvalue)
{
    m_is_lvalue = is_lvalue;
}
void ASTNode::set_is_literal(bool is_literal)
{
    m_is_literal = is_literal;
}

int ASTNode::get_int_value()
{
    return m_value.int_value;
}
float ASTNode::get_float_value()
{
    return m_value.float_value;
}
void ASTNode::set_int_value(int value)
{
    m_value.int_value = value;
}
void ASTNode::set_float_value(float value)
{
    m_value.float_value = value;
}
void ASTNode::set_value(BasicValue value)
{
    m_value = value;
}
bool ASTNode::is_condition_operation_binary(BinaryOpFunc binary_op_func)
{
     return binary_op_func == BinaryOpFunc::Equal ||
         binary_op_func == BinaryOpFunc::NotEqual ||
         binary_op_func == BinaryOpFunc::Less ||
         binary_op_func == BinaryOpFunc::LessEqual ||
         binary_op_func == BinaryOpFunc::Great ||
         binary_op_func == BinaryOpFunc::GreatEqual||
         binary_op_func == BinaryOpFunc::And ||
         binary_op_func == BinaryOpFunc::Or;
}
bool ASTNode::is_condition_operation_unary(UnaryOpFunc unary_op_func)
{
    return unary_op_func == UnaryOpFunc::Not;
}
/*
void ASTNode::remove_first_child()
{
    m_child.erase(std::begin(m_child));
}*/
ASTNode::ASTNode(const ASTType i_type,
                 const std::vector<ASTNode *> &i_child_list,
                 const BasicValue i_value,
                 const int i_func,
                 std::string i_var_name,
                 const ValueType i_value_type,
                 const bool i_is_lvalue,
                 const bool i_is_literal) : m_type(i_type), m_child(i_child_list), m_func(i_func), m_value_type(i_value_type),
                                            m_is_lvalue(i_is_lvalue), m_var_name(i_var_name), m_value(i_value), m_is_literal(i_is_literal),
                                            m_line_no(line_no)
{
}
ASTNode *ASTNode::create(const ASTType type,
                         const std::vector<ASTNode *> &child_list,
                         const BasicValue value,
                         const int func,
                         std::string var_name,
                         const ValueType value_type,
                         const bool lvalue,
                         const bool is_literal)
{
    return new ASTNode(type, child_list, value, func, var_name, value_type, lvalue, is_literal);
}
//建立编译单元
ASTNode* ASTNode::create_comp_unit()
{
    return new ASTNode(ASTType::CompUnit, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
//建立一个空的declstmt结点
ASTNode* ASTNode::create_decl_stmt(ValueType type)
{
    return new ASTNode(ASTType::DeclStmt, {}, BasicValue::zero(), 0, "", type, false, false);
}
//建立常量、变量声明节点
ASTNode* ASTNode::create_var_decl(ValueType type, std::string var_name, bool is_array, std::vector<ASTNode *> children)
{
    ASTNode *var_decl = new ASTNode(ASTType::VarDecl, children, BasicValue::zero(), 0, var_name, type, false, false);
    if (is_array) {
        // children[0]是ArraySize,children[1]是ArrayInitVal
        for (auto &dimension : children[0]->get_child())
            var_decl->m_value_type.add_dimension(0);
    }
    return var_decl;
}
ASTNode* ASTNode::create_const_decl(ValueType type, std::string var_name, bool is_array, std::vector<ASTNode *> children)
{
    ASTNode *const_decl = new ASTNode(ASTType::ConstDecl, children, BasicValue::zero(), 0, var_name, type, false, false);
    if (is_array) {
        for (auto &dimension : children[0]->get_child())
            const_decl->m_value_type.add_dimension(0);
    }
    return const_decl;
}
//建立一个函数定义结点
ASTNode* ASTNode::create_func_def(ValueType type, std::string name)
{
    return new ASTNode(ASTType::FuncDef, {}, BasicValue::zero(), 0, name, type, false, false);
}
//建立一个没有子节点的block结点
ASTNode* ASTNode::create_block()
{
    return new ASTNode(ASTType::Block, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_const_value(ValueType type, BasicValue value)
{
    return new ASTNode(ASTType::ConstValue, {}, value, 0, "", type, false, true);
}
ASTNode* ASTNode::create_binary_op(BinaryOpFunc func, ValueType value_type, ASTNode *left_child, ASTNode *right_child)
{ /*
  对于赋值操作，如果右孩子类型与左孩子不同，则做隐式类型转换。如果有Uncertain则不处理，结点值类型保持与左孩子相同。
  对于其他操作，如果左右为一个Int一个Float，则向Float转换，结点值类型为Float。如果有Uncertain则不处理，结点值类型为Uncertain。
  */
    //暂无IntToFloat隐式类型转换
    return new ASTNode(ASTType::BinaryOp, {left_child, right_child}, BasicValue::zero(), (int)func, "", value_type, false, false);
}
ASTNode* ASTNode::create_unary_op(UnaryOpFunc func, ValueType value_type, ASTNode *child, bool is_lvalue)
{
    return new ASTNode(ASTType::UnaryOp, {child}, BasicValue::zero(), (int)func, "", value_type, is_lvalue, false);
}
ASTNode* ASTNode::create_if_stmt(ASTNode *condition, ASTNode *true_stmt, ASTNode *false_stmt)
{
    if (false_stmt == nullptr)
        return new ASTNode(ASTType::IfStmt, {condition, true_stmt}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
    return new ASTNode(ASTType::IfStmt, {condition, true_stmt, false_stmt}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_switch_stmt(std::vector<ASTNode*> children)
{
    return new ASTNode(ASTType::SwitchStmt, children, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_case(BasicValue value, ASTNode* child)
{
    return new ASTNode(ASTType::Case, {child}, value, 0, "", ValueType(BasicType::Int), false, true);
}
ASTNode* ASTNode::create_default(ASTNode* child)
{
    if (child == nullptr)
        return new ASTNode(ASTType::Default, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
    return new ASTNode(ASTType::Default, {child}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_while_stmt(ASTNode *condition, ASTNode *stmt)
{
    return new ASTNode(ASTType::WhileStmt, {condition, stmt}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_do_while_stmt(ASTNode* condition, ASTNode* stmt)
{
    return new ASTNode(ASTType::DoWhileStmt, { condition, stmt }, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_break_stmt()
{
    return new ASTNode(ASTType::BreakStmt, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_continue_stmt()
{
    return new ASTNode(ASTType::ContinueStmt, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_return_stmt(ASTNode *return_value_node)
{
    if (return_value_node == nullptr)
        return new ASTNode(ASTType::ReturnStmt, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
    return new ASTNode(ASTType::ReturnStmt, {return_value_node}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_null_stmt()
{
    return new ASTNode(ASTType::NullStmt, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
} //建立一个空语句节点
ASTNode* ASTNode::create_ident(ValueType type, std::string name, bool is_lvalue)
{
    return new ASTNode(ASTType::Ident, {}, BasicValue::zero(), 0, name, type, is_lvalue, false);
}
ASTNode* ASTNode::create_func_call(std::string name)
{
    return new ASTNode(ASTType::FuncCall, {}, BasicValue::zero(), 0, name, ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_func_r_params()
{
    return new ASTNode(ASTType::FuncRParams, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_func_f_params()
{
    return new ASTNode(ASTType::FuncFParams, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_func_f_param(ValueType type, std::string name, bool is_array, bool is_lvalue, std::vector<ASTNode *> children)
{
    ASTNode *func_f_param = new ASTNode(ASTType::FuncFParam, children, BasicValue::zero(), 0, name, type, is_lvalue, false);
    if (is_array) {
        //第一个维度是隐式声明的
        func_f_param->m_value_type.add_dimension(0);
        for (auto &dimension : children)
            func_f_param->m_value_type.add_dimension(0);
    }
    return func_f_param;
}
ASTNode* ASTNode::create_implicit_cast(ValueType type_after_cast, ImplicitCastFunc func, ASTNode *child)
{
    return new ASTNode(ASTType::ImplicitCast, {child}, BasicValue::zero(), (int)func, "", type_after_cast, false, false);
}
ASTNode* ASTNode::create_array_size()
{
    return new ASTNode(ASTType::ArraySize, {}, BasicValue::zero(), 0, "", ValueType(BasicType::None), false, false);
}
ASTNode* ASTNode::create_array_init_val(ValueType type)
{
    return new ASTNode(ASTType::ArrayInitVal, {}, BasicValue::zero(), 0, "", type, false, false);
}
ASTNode* ASTNode::create_array_visit(ValueType type, ASTNode *ident, ASTNode *subscript)
{
    return new ASTNode(ASTType::ArrayVisit, {ident, subscript}, BasicValue::zero(), 0, ident->get_var_name(), type, false, false);
}
void ASTNode::add_child(ASTNode *child)
{
    m_child.push_back(child);
}
void ASTNode::add_child(std::vector<ASTNode *> children)
{
    m_child.insert(m_child.end(), children.begin(), children.end());
}
ASTNode* ASTNode::copy_condition(ASTNode* node)
{
    assert(node != nullptr);
    assert(node->m_type != ASTType::DeclStmt);
    ASTNode* res = new ASTNode();
    res->m_child = {};
    res->m_type = node->m_type;
    res->m_line_no = node->m_line_no;
    res->m_value_type = node->m_value_type;
    res->m_is_lvalue = node->m_is_lvalue;
    res->m_is_literal = node->m_is_literal;
    res->m_value = node->m_value;
    res->m_func = node->m_func;
    res->m_var_name = node->m_var_name;
    res->m_symbol = node->m_symbol;//指向同一个符号
    //复制子树
    for (auto child : node->get_child()) {
        assert(child!= nullptr);
        res->add_child(copy_condition(child));
    }
    return res;
}
void ASTNode::rewrite_type(ASTType type)
{
    m_type = type;
}
//销毁自身以及全部子节点
void ASTNode::destory(ASTNode *target)
{
    for (auto &child : target->get_child()) {
        destory(child);
    }
    delete target;
}

void ASTNode::clear_child()
{
    m_child.clear();
}

void ASTNode::printNode(int tier, ostream &os)
{
    for (int i = 0; i < tier; i++)
        os << "--";
    switch (m_type) {
    case ASTType::CompUnit:
        os << "CompUnit" << endl;
        break;
    case ASTType::DeclStmt:
        os << "DeclStmt" << endl;
        break;
    case ASTType::VarDecl:
        os << "VarDecl  " << m_var_name << " " << m_value_type.get_string() << " ";
        if (m_symbol && m_symbol->is_literally_initialized())
            os << "Literally Initialized";
        os << endl;
        break;
    case ASTType::ConstDecl:
        os << "ConstDecl  " << m_var_name << " " << m_value_type.get_string() << " ";
        if (m_symbol && m_symbol->is_literally_initialized())
            os << "Literally Initialized";
        os << endl;
        break;
    case ASTType::Block:
        os << "Block" << endl;
        break;
    case ASTType::FuncDef:
        os << "FuncDef  " << m_var_name << " " << m_value_type.get_string() << endl;
        break;
    case ASTType::ConstValue:
        os << "ConstValue  ";
        if (m_value_type.basic() == BasicType::Int)
            os << m_value.int_value << " Int" << endl;
        else if (m_value_type.basic() == BasicType::Float)
            os << m_value.float_value << " Float" << endl;
        else
            os << m_value.float_value << " <<Unknown Type>>" << endl;
        break;
    case ASTType::Ident:
        os << "Ident  " << m_var_name << " " << m_value_type.get_string() << " ";
        if (m_is_literal) {
            if (m_value_type.is_array()) {
                os << "offset: " << m_value.int_value << endl;
            } else {
                if (m_value_type.basic() == BasicType::Int)
                    os << m_value.int_value << endl;
                else
                    os << m_value.float_value << endl;
            }
        } else
            os << endl;
        break;
    case ASTType::BinaryOp:
        os << "BinaryOp  " << get_binary_op_string((BinaryOpFunc)m_func) << " " << m_value_type.get_string() << " ";
        if (m_is_literal) {
            if (m_value_type.basic() == BasicType::Int)
                os << m_value.int_value << endl;
            else
                os << m_value.float_value << endl;
        } else
            os << endl;
        break;
    case ASTType::UnaryOp:
        os << "UinaryOp  " << get_unary_op_string((UnaryOpFunc)m_func) << " " << m_value_type.get_string() << " ";
        if (m_is_literal) {
            if (m_value_type.basic() == BasicType::Int)
                os << m_value.int_value << endl;
            else
                os << m_value.float_value << endl;
        } else
            os << endl;
        break;
    case ASTType::IfStmt:
        os << "IfStmt" << endl;
        break;
    case ASTType::SwitchStmt:
        os << "SwitchStmt" << endl;
        break;
    case ASTType::Case:
        os << "Case: " << m_value.int_value << endl; 
        break;
    case ASTType::Default:
        os << "Default: " << endl;
        break;
    case ASTType::WhileStmt:
        os << "WhileStmt" << endl;
        break;
    case ASTType::DoWhileStmt:
        os << "DoWhileStmt" << endl;
        break;
    case ASTType::BreakStmt:
        os << "BreakStmt" << endl;
        break;
    case ASTType::ContinueStmt:
        os << "ContinueStmt" << endl;
        break;
    case ASTType::ReturnStmt:
        os << "ReturnStmt" << endl;
        break;
    case ASTType::NullStmt:
        os << "NullStmt" << endl;
        break;
    case ASTType::FuncCall:
        os << "FuncCall  " << m_var_name << endl;
        break;
    case ASTType::FuncRParams:
        os << "FuncRParams" << endl;
        break;
    case ASTType::FuncFParams:
        os << "FuncFParams" << endl;
        break;
    case ASTType::FuncFParam:
        os << "FuncFParam  " << m_var_name << " " << m_value_type.get_string() << endl;
        break;
    case ASTType::ImplicitCast:
        os << "ImplicitCast  " << get_implicit_cast_string((ImplicitCastFunc)m_func) << " " << m_value_type.get_string() << " ";
        if (m_is_literal) {
            if (m_value_type.is_array()) {
                os << "offset: " << m_value.int_value << endl;
            } else {
                if (m_value_type.basic() == BasicType::Int)
                    os << m_value.int_value << endl;
                else
                    os << m_value.float_value << endl;
            }
        } else
            os << endl;
        break;
    case ASTType::ArraySize:
        os << "ArraySize" << endl;
        break;
    case ASTType::ArrayInitVal:
        os << "ArrayInitVal  " << m_value_type.get_string();
        if (m_is_literal) {
            os << " Literal";
        }
        os << endl;
        break;
    case ASTType::ArrayVisit:
        if (m_is_literal)
            os << "ArrayVisit  " << m_var_name << " " << m_value_type.get_string() << " offset: " << m_value.int_value << endl;
        else
            os << "ArrayVisit  " << m_var_name << " " << m_value_type.get_string() << endl;
        break;
    default:
        os << "Unknown" << endl;
        assert(false);
        break;
    }
    for (auto &child : m_child) {
        // assert(child!=nullptr);
        if (child == nullptr) {
            for (int i = 0; i <= tier; i++)
                os << "--";
            os << "[NULLPTR!]" << endl;
        } else
            child->printNode(tier + 1);
    }
}

BasicValue ASTNode::get_literal_value()
{
    if (m_is_literal) {
        if (m_symbol && m_symbol->is_array() && !m_value_type.is_array() && m_type == ASTType::ArrayVisit) {
            //由偏移量得出数组元素的常量值
            if (m_value_type.basic() == BasicType::Float)
                return BasicValue::create_float(m_symbol->get_array_float_value(m_value.int_value));
            else
                return BasicValue::create_int(m_symbol->get_array_int_value(m_value.int_value));
        } else {
            return m_value;
        }
    } else {
        return BasicValue::zero();
    }
}
