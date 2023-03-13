#include <cassert>
#include <iostream>
#include <semantic_analysis.h>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

bool SemanticAnalysis::in_global_area = true;
BasicType SemanticAnalysis::return_type = BasicType::None;
int SemanticAnalysis::in_while_stmt = 0;
SymbolTable* SemanticAnalysis::sym_table = nullptr;
std::stack<std::pair<ASTNode*, int>>* SemanticAnalysis::current_stack = nullptr;
void SemanticAnalysis::semantic_analysis(ASTNode* node, SymbolTable* sym_table)
{
    SemanticAnalysis::sym_table = sym_table;
    semantic_analysis_recursion(node);
}
void SemanticAnalysis::push_node(ASTNode* node)
{
    current_stack->push({ node,(int)SemanticAnalysisState::BeforeRecursion});
}
void SemanticAnalysis::semantic_analysis_recursion(ASTNode* node)
{
    stack<pair<ASTNode*, int>> analysis_stack;//语义分析栈
    current_stack = &analysis_stack;
    assert(analysis_stack.empty());
    push_node(node);
    while (!analysis_stack.empty()) {
        auto [node,child_index] = analysis_stack.top();
        //cout << now.second << endl;
        analysis_stack.pop();
        switch (node->get_type()) {
        case ASTType::FuncDef:
            analysis_func_def(node);
            break;
        case ASTType::Ident:
            analysis_ident(node);
            break;
        case ASTType::FuncCall:
            analysis_func_call(node);
            break;
        case ASTType::ImplicitCast:
            analysis_implicit_cast(node);
            break;
        case ASTType::VarDecl:
            analysis_var_decl(node);
            break;
        case ASTType::ConstDecl:
            analysis_const_decl(node);
            break;
        case ASTType::ArrayInitVal:
            analysis_array_init_val(node);
            break;
        case ASTType::WhileStmt:
            analysis_while_stmt(node);
            break;
        case ASTType::IfStmt:
            analysis_if_stmt(node);
            break;
        case ASTType::BreakStmt:
            analysis_break_stmt(node);
            break;
        case ASTType::ContinueStmt:
            analysis_continue_stmt(node);
            break;
        case ASTType::FuncFParam:
            analysis_func_f_param(node);
            break;
        case ASTType::FuncFParams:
            analysis_func_f_params(node);
            break;
        case ASTType::BinaryOp:
            analysis_binary_op(node, child_index);
            break;
        case ASTType::UnaryOp:
            analysis_unary_op(node, child_index);
            break;
        case ASTType::FuncRParams:
            analysis_func_r_params(node);
            break;
        case ASTType::ArrayVisit:
            analysis_array_visit(node);
            break;
        case ASTType::Block:
            analysis_block(node, child_index);
            break;
        case ASTType::ReturnStmt:
            analysis_return_stmt(node);
            break;
        default:
            analysis_(node);
            break;
        }
        current_stack = &analysis_stack;
    }
}
void SemanticAnalysis::analysis_(ASTNode *node)
{
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
}
void SemanticAnalysis::analysis_func_def(ASTNode *node)
{
    size_t size_of_stack = sym_table->get_stack_size();
    Symbol *new_symbol = sym_table->add_sym(Symbol(node->get_value_type(), VarKind::Func, node->get_var_name()));
    node->set_symbol(new_symbol);
    //函数名不从符号栈中删除
    return_type = node->get_value_type().basic();
    in_global_area = false;
    size_of_stack++;
    //传递函数名给func_f_params和block
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        child->set_symbol(node->get_symbol());
    }
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    //给函数体末尾补return语句
    ASTNode *block_child = node->get_child(node->get_child().size() - 1);
    ASTNode *last_child_of_block_child;
    if (block_child->get_child().size() > 0) {
        last_child_of_block_child = block_child->get_child(block_child->get_child().size() - 1);
        if (last_child_of_block_child->get_type() != ASTType::ReturnStmt) {
            ASTNode *return_stmt;
            if (return_type == BasicType::Int)
                return_stmt = ASTNode::create_return_stmt(ASTNode::create_const_value(ValueType(BasicType::Int), BasicValue::create_int(0)));
            else if (return_type == BasicType::Float)
                return_stmt = ASTNode::create_return_stmt(ASTNode::create_const_value(ValueType(BasicType::Float), BasicValue::create_float(0)));
            else
                return_stmt = ASTNode::create_return_stmt();
            block_child->add_child(return_stmt);
        }
    } else {
        ASTNode *return_stmt;
        if (return_type == BasicType::Int)
            return_stmt = ASTNode::create_return_stmt(ASTNode::create_const_value(ValueType(BasicType::Int), BasicValue::create_int(0)));
        else if (return_type == BasicType::Float)
            return_stmt = ASTNode::create_return_stmt(ASTNode::create_const_value(ValueType(BasicType::Float), BasicValue::create_float(0)));
        else
            return_stmt = ASTNode::create_return_stmt();
        block_child->add_child(return_stmt);
    }
    //从符号栈中删除形参
    while (sym_table->get_stack_size() > size_of_stack) {
        sym_table->delete_top_sym();
    }
    return_type = BasicType::None;
    in_global_area = true;
}
void SemanticAnalysis::analysis_ident(ASTNode *node)
{
    Symbol *m_symbol = sym_table->get_current_sym(node->get_var_name());
    if (m_symbol != nullptr) {
        //设置结点属性
        node->set_symbol(m_symbol);
        node->set_value_type(m_symbol->get_val_type());
        node->set_is_lvalue(true);
        node->set_is_literal(m_symbol->is_const());
        //赋值
        if (node->is_literal()) {
            if (node->get_value_type().is_array()) {
                //数组偏移下标
                node->set_int_value(0);
            } else {
                if (node->get_value_type().basic() == BasicType::Float)
                    node->set_float_value(m_symbol->get_float_value());
                else if (node->get_value_type().basic() == BasicType::Int)
                    node->set_int_value(m_symbol->get_int_value());
            }
        }
    } else {
        stringstream error;
        error << "Undefined Symbol \'" << node->get_var_name() << "\' ." << std::endl;
        semantic_error(error.str(), node->get_line_no(), 0);
    }
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
}
void SemanticAnalysis::analysis_func_call(ASTNode *node)
{
    //特殊处理starttime和stoptime宏定义
    if (node->get_var_name() == string("starttime")|| node->get_var_name() == string("stoptime")) {
        if (!node->get_child().empty())
            semantic_error("Bad use of library function", node->get_line_no(), 1);
        if (node->get_var_name() == string("starttime"))
            node->set_var_name("_sysy_starttime");
        else node->set_var_name("_sysy_stoptime");
        auto params= ASTNode::create_func_r_params();
        auto child = ASTNode::create_const_value(ValueType(BasicType::Int),BasicValue(node->get_line_no()));
        params->add_child(child);
        node->add_child(params);
    }
    Symbol* m_symbol = sym_table->get_current_sym(node->get_var_name());
    if (m_symbol != nullptr) {
        //设置节点属性
        node->set_symbol(m_symbol);
        node->set_value_type(m_symbol->get_val_type());
    } else {
        stringstream error;
        error << "Undefined Function \'" << node->get_var_name() << "\' ." << std::endl;
        semantic_error(error.str(), node->get_line_no(), 2);
    }
    //检查函数无实参但有形参的特殊情况
    assert(node->get_symbol() != nullptr);
    if (node->get_child().size() == 0 && node->get_symbol()->get_param_type().size() != 0) {
        semantic_error("The FuncRParams of FuncCall does not conform to the FuncFParams of FuncDef.", node->get_line_no(), 3);
    }
    //传递函数名symbol给func_r_params
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        child->set_symbol(node->get_symbol());
    }
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
}
void SemanticAnalysis::analysis_implicit_cast(ASTNode *node)
{

    ASTNode *child = node->get_child(0);
    BasicValue value = child->get_literal_value();
    switch ((ImplicitCastFunc)node->get_func()) {
    case ImplicitCastFunc::IntToFloat:
        if (child->get_value_type().is_array())
            node->set_value(value);
        else
            node->set_float_value((float)value.int_value);
        node->set_value_type(child->get_value_type());
        node->get_value_type().set_basic_type(BasicType::Float);
        node->set_is_lvalue(child->is_lvalue());
        node->set_is_literal(child->is_literal());
        node->set_var_name(child->get_var_name());
        node->set_symbol(child->get_symbol());
        break;
    case ImplicitCastFunc::FloatToInt:
        if (child->get_value_type().is_array())
            node->set_value(value);
        else
            node->set_int_value((int)value.float_value);
        node->set_value_type(child->get_value_type());
        node->get_value_type().set_basic_type(BasicType::Int);
        node->set_is_lvalue(child->is_lvalue());
        node->set_is_literal(child->is_literal());
        node->set_var_name(child->get_var_name());
        node->set_symbol(child->get_symbol());
        break;
    case ImplicitCastFunc::LValueToRValue:
        node->set_value(value);
        node->set_value_type(child->get_value_type());
        node->set_is_lvalue(false);
        node->set_is_literal(child->is_literal());
        node->set_var_name(child->get_var_name());
        node->set_symbol(child->get_symbol());
        break;
    default:
        break;
    }
}
void SemanticAnalysis::analysis_var_decl(ASTNode *node)
{
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        if (child->get_type() == ASTType::ArrayInitVal)
            child->get_value_type().set_basic_type(node->get_value_type().basic());
    }
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    ASTNode *child;
    //如果是数组，根据子节点中各维度的大小，把类型信息补充完整
    if (node->get_value_type().is_array()) {
        child = node->get_child(0);
        int dimension = 0;
        for (auto &child_of_child : child->get_child()) {
            if (!child_of_child->is_literal() || child_of_child->get_value_type().basic() != BasicType::Int) {
                semantic_error("Invalid size for the dimensions of array.", node->get_line_no(), 4);
            }
            int v = child_of_child->get_int_value();
            if (v <= 0) {
                semantic_error("Invalid size for the dimensions of array.", node->get_line_no(), 5);
            }
            node->get_value_type().set_array_length(dimension, v);
            dimension++;
        }
        //完成数组维度长度分析，删除arraysize子结点
        // node->remove_first_child();
    }
    node->set_symbol(sym_table->add_sym(Symbol(node->get_value_type(), in_global_area ? VarKind::Global : VarKind::Local, node->get_var_name())));
    //初值
    if (node->get_child().size() > 1 && node->get_value_type().is_array()) {
        //数组
        child = node->get_child(1);
        if (child->is_literal()) {
            int offset = 0;
            array_init_search(child, 1, node->get_value_type().get_dimension(), node->get_value_type().basic(), node->get_symbol(), offset);
        }
        assert(node->get_symbol() != nullptr);
        //完成变量初始化，删除子结点 (有内存泄漏，而且先不要删，有用)
        // node->remove_first_child();
    } 
    else if (node->get_child().size() > 0 && !node->get_value_type().is_array()){
        child = node->get_child(0);
        //非数组
        if (child->is_lvalue()) {
            //插入左值转右值的隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(child->get_value_type(), ImplicitCastFunc::LValueToRValue, child);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(0);
        }
        if (node->get_value_type().basic() == BasicType::Float && child->get_value_type().basic() == BasicType::Int) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Float), ImplicitCastFunc::IntToFloat, child);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(0);
        } else if (node->get_value_type().basic() == BasicType::Int && child->get_value_type().basic() == BasicType::Float) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Int), ImplicitCastFunc::FloatToInt, child);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(0);
        }
        if (!child->is_literal())
            return;
        assert(node->get_symbol() != nullptr);
        if (node->get_value_type().basic() == BasicType::Float) {
            node->set_float_value(child->get_literal_value().float_value);
            node->get_symbol()->add_float_value(node->get_float_value());
        } else {
            node->set_int_value(child->get_literal_value().int_value);
            node->get_symbol()->add_int_value(node->get_int_value());
        }
        //完成变量初始化，删除子结点(有内存泄漏，而且先不要删，有用)
        // node->remove_first_child();
    }
}
void SemanticAnalysis::analysis_const_decl(ASTNode *node )
{

    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        if (child->get_type() == ASTType::ArrayInitVal)
            child->get_value_type().set_basic_type(node->get_value_type().basic());
    }
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    ASTNode *child;
    //如果是数组，根据子节点中各维度的大小，把类型信息补充完整
    if (node->get_value_type().is_array()) {
        child = node->get_child(0);
        int dimension = 0;
        for (auto &child_of_child : child->get_child()) {
            if (!child_of_child->is_literal() || child_of_child->get_value_type().basic() != BasicType::Int)
                semantic_error("Invalid size for the dimensions of array.", node->get_line_no(), 6);
            int v = child_of_child->get_int_value();
            if (v <= 0)
                semantic_error("Invalid size for the dimensions of array.", node->get_line_no(), 7);
            node->get_value_type().set_array_length(dimension, v);
            dimension++;
        }
        //完成数组维度长度分析，删除arraysize子结点
        // child->destory();
        // node->remove_first_child();
    }
    node->set_symbol(sym_table->add_sym(Symbol(node->get_value_type(), in_global_area ? VarKind::Global : VarKind::Local, node->get_var_name(), true)));
    //初值
    if (node->get_value_type().is_array()) {
        //数组
        child = node->get_child(1);
        int offset = 0;
        array_init_search(child, 1, node->get_value_type().get_dimension(), node->get_value_type().basic(), node->get_symbol(),offset);
        assert(node->get_symbol() != nullptr);
        //完成变量初始化，删除子结点
        // node->remove_first_child();
    } else {
        child = node->get_child(0);
        //非数组
        if (child->is_lvalue()) {
            //插入左值转右值的隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(child->get_value_type(), ImplicitCastFunc::LValueToRValue, child);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(0);
        }
        if (node->get_value_type().basic() == BasicType::Float && child->get_value_type().basic() == BasicType::Int) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Float), ImplicitCastFunc::IntToFloat, child);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(0);
        } else if (node->get_value_type().basic() == BasicType::Int && child->get_value_type().basic() == BasicType::Float) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Int), ImplicitCastFunc::FloatToInt, child);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(0);
        }
        assert(node->get_symbol() != nullptr);
        if (node->get_value_type().basic() == BasicType::Float) {
            node->set_float_value(child->get_literal_value().float_value);
            node->get_symbol()->add_float_value(node->get_float_value());
        } else {
            node->set_int_value(child->get_literal_value().int_value);
            node->get_symbol()->add_int_value(node->get_int_value());
        }
        //完成变量初始化，删除子结点
        // node->remove_first_child();
    }
}
void SemanticAnalysis::analysis_array_init_val(ASTNode *node )
{
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        if (child->get_type() == ASTType::ArrayInitVal)
            child->get_value_type().set_basic_type(node->get_value_type().basic());
    }
    bool is_literal = true;
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
        is_literal = is_literal && child->is_literal();
    }
    for (int i = 0; i < node->get_child().size(); i++) {
        ASTNode *child = node->get_child(i);
        if (child->get_type() != ASTType::ArrayInitVal) {
            if (child->is_lvalue()) {
                //插入左值转右值隐式类型转换结点
                ASTNode* implicit_cast_node = ASTNode::create_implicit_cast(node->get_value_type().basic(),ImplicitCastFunc::LValueToRValue, child);
                node->set_child(i, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                child = node->get_child(i);
            }
            if (node->get_value_type().basic() == BasicType::Float && child->get_value_type().basic() == BasicType::Int) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Float), ImplicitCastFunc::IntToFloat, child);
                node->set_child(i, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                child = node->get_child(i);
            } else if (node->get_value_type().basic() == BasicType::Int && child->get_value_type().basic() == BasicType::Float) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Int), ImplicitCastFunc::FloatToInt, child);
                node->set_child(i, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                child = node->get_child(i);
            }
        }
    }
    node->set_is_literal(is_literal);
}
void SemanticAnalysis::analysis_if_stmt(ASTNode* node)
{
    //递归
    for (auto child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    auto condition = node->get_child(0);
    assert(condition != nullptr);
    if (condition->is_lvalue()) {
        ASTNode* implicit_cast_node = ASTNode::create_implicit_cast(condition->get_value_type(), ImplicitCastFunc::LValueToRValue, condition);
        semantic_analysis_recursion(implicit_cast_node);
        node->set_child(0, implicit_cast_node);
        condition = implicit_cast_node;
    }
}
void SemanticAnalysis::analysis_while_stmt(ASTNode *node )
{
    in_while_stmt++;
    //递归
    for (auto child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    auto condition = node->get_child(0);
    assert(condition != nullptr);
    //如果目标不是右值，插入隐式类型转换结点
    if (condition->is_lvalue()) {
        ASTNode* implicit_cast_node = ASTNode::create_implicit_cast(condition->get_value_type(), ImplicitCastFunc::LValueToRValue, condition);
        semantic_analysis_recursion(implicit_cast_node);
        node->set_child(0,implicit_cast_node);
        condition = implicit_cast_node;
    }
    if (condition->get_value_type().basic() == BasicType::Float) {
        //插入隐式类型转换结点
        ASTNode* implicit_cast_node = ASTNode::create_implicit_cast(BasicType::Int, ImplicitCastFunc::FloatToInt, condition);
        node->set_child(0, implicit_cast_node);
        semantic_analysis_recursion(implicit_cast_node);
        condition = node->get_child(0);
    }
    in_while_stmt--;
}
void SemanticAnalysis::analysis_break_stmt(ASTNode *node )
{
    if (!in_while_stmt)
        semantic_error("Break statement must be in the while statement.", node->get_line_no(), 9);
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
}
void SemanticAnalysis::analysis_continue_stmt(ASTNode *node )
{
    if (!in_while_stmt)
        semantic_error("Continue statement must be in the while statement.", node->get_line_no(), 10);
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
}
void SemanticAnalysis::analysis_func_f_param(ASTNode *node )
{
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    if (node->get_value_type().is_array()) {
        int dimension = 1;
        for (auto &child : node->get_child()) {
            assert(child != nullptr);
            if (!child->is_literal() || child->get_value_type().basic() != BasicType::Int) {
                semantic_error("Invalid size for the dimensions of array.", node->get_line_no(), 11);
            }
            int v = child->get_int_value();
            if (v <= 0) {
                semantic_error("Invalid size for the dimensions of array.", node->get_line_no(), 12);
            }
            node->get_value_type().set_array_length(dimension, v);
            dimension++;
        }
        /*
        int child_num = node->get_child().size();
        for (int i = 0; i < child_num; i++)
            node->remove_first_child();*/
    }
    node->set_symbol(sym_table->add_sym(Symbol(node->get_value_type(), VarKind::Param, node->get_var_name())));
}
void SemanticAnalysis::analysis_func_f_params(ASTNode *node )
{
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    //将函数形参类型添加到函数名符号的m_param_type中
    assert(node->get_symbol() != nullptr);
    for (auto &child : node->get_child()) {
        node->get_symbol()->add_param_type(child->get_symbol()->get_val_type());
    }
}

void SemanticAnalysis::analysis_binary_op(ASTNode *node,int state)
{
    //非递归分析
    if (state == (int)SemanticAnalysisState::BeforeRecursion) {
        current_stack->push({ node, (int)SemanticAnalysisState::AfterRecursion });
        for (auto iter = node->get_child().rbegin(); iter != node->get_child().rend(); iter++) {
            assert(*iter != nullptr);
            push_node(*iter);
        }
        return;
    }
    
    ASTNode* lchild = node->get_child(0);
    ASTNode* rchild = node->get_child(1);
    bool is_array_offset_calc = false;
    if (lchild->get_value_type().is_array() || rchild->get_value_type().is_array())
    {
        is_array_offset_calc = true;
        // semantic_error("Operand is an array.", node->get_line_no(), 13);
        if ((BinaryOpFunc)node->get_func() != BinaryOpFunc::Add) {
            semantic_error("Unsupported Operator between arrays/array and offset", node->get_line_no(), 13);
        }
        if(lchild->get_value_type().is_array())
            node->get_value_type().set_dimension(lchild->get_value_type().get_dimension());
        else
            node->get_value_type().set_dimension(rchild->get_value_type().get_dimension());
        //强制要求左端是数组
        if (rchild->get_value_type().is_array() && !lchild->get_value_type().is_array()) {
            node->set_child(1, lchild);
            node->set_child(0, rchild);
            swap(lchild, rchild);
        }
    }
    //判断BasicType
    if ((BinaryOpFunc)node->get_func() == BinaryOpFunc::Assign) {
        assert(lchild->get_symbol() != nullptr);
        if (lchild->get_symbol()->is_const())
            semantic_error("Constant cannot be assigned again.", node->get_line_no(), 14);
        if (rchild->is_lvalue()) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(rchild->get_value_type(), ImplicitCastFunc::LValueToRValue, rchild);
            node->set_child(1, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            rchild = node->get_child(1);
        }
        if (lchild->get_value_type().basic() == BasicType::Float) {
            node->get_value_type().set_basic_type(BasicType::Float);
            if (rchild->get_value_type().basic() == BasicType::Int) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Float), ImplicitCastFunc::IntToFloat, rchild);
                node->set_child(1, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                rchild = node->get_child(1);
            }
        } else {
            node->get_value_type().set_basic_type(BasicType::Int);
            if (rchild->get_value_type().basic() == BasicType::Float) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Int), ImplicitCastFunc::FloatToInt, rchild);
                node->set_child(1, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                rchild = node->get_child(1);
            }
        }
    } else {
        if (lchild->is_lvalue()) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(lchild->get_value_type(), ImplicitCastFunc::LValueToRValue, lchild);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            lchild = node->get_child(0);
        }
        if (rchild->is_lvalue()) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(rchild->get_value_type(), ImplicitCastFunc::LValueToRValue, rchild);
            node->set_child(1, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            rchild = node->get_child(1);
        }
        if(is_array_offset_calc)
            node->set_value_type(lchild->get_value_type());
        else if (lchild->get_value_type().basic() == BasicType::Int && rchild->get_value_type().basic() == BasicType::Int) {
            node->get_value_type().set_basic_type(BasicType::Int);
        } else if (lchild->get_value_type().basic() == BasicType::Int && rchild->get_value_type().basic() == BasicType::Float) {
            if (ASTNode::is_condition_operation_binary((BinaryOpFunc)node->get_func()))
                node->get_value_type().set_basic_type(BasicType::Int);
            else node->get_value_type().set_basic_type(BasicType::Float);
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Float), ImplicitCastFunc::IntToFloat, lchild);
            node->set_child(0, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            lchild = node->get_child(0);
            
        } else if (lchild->get_value_type().basic() == BasicType::Float && rchild->get_value_type().basic() == BasicType::Int) {
            if (ASTNode::is_condition_operation_binary((BinaryOpFunc)node->get_func()))
                node->get_value_type().set_basic_type(BasicType::Int);
            else node->get_value_type().set_basic_type(BasicType::Float);
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(ValueType(BasicType::Float), ImplicitCastFunc::IntToFloat, rchild);
            node->set_child(1, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            rchild = node->get_child(1);
        } else {//子节点均为float
            if (ASTNode::is_condition_operation_binary((BinaryOpFunc)node->get_func()))
                node->get_value_type().set_basic_type(BasicType::Int);
            else node->get_value_type().set_basic_type(BasicType::Float);
        }
    }
    //计算literal
    if (lchild->is_literal() && rchild->is_literal()) {
        node->set_is_literal(true);
        float&& lchild_float_value = lchild->get_literal_value().float_value;
        float&& rchild_float_value = rchild->get_literal_value().float_value;
        int&& lchild_int_value = lchild->get_literal_value().int_value;
        int&& rchild_int_value = rchild->get_literal_value().int_value;
        switch ((BinaryOpFunc)node->get_func()) {
        case BinaryOpFunc::Add:
            if (node->get_value_type().basic() == BasicType::Float)
                node->set_float_value(lchild_float_value + rchild_float_value);
            else
                node->set_int_value(lchild_int_value + rchild_int_value);
            break;
        case BinaryOpFunc::Sub:
            if (node->get_value_type().basic() == BasicType::Float)
                node->set_float_value(lchild_float_value - rchild_float_value);
            else
                node->set_int_value(lchild_int_value - rchild_int_value);
            break;
        case BinaryOpFunc::Mul:
            if (node->get_value_type().basic() == BasicType::Float)
                node->set_float_value(lchild_float_value * rchild_float_value);
            else
                node->set_int_value(lchild_int_value * rchild_int_value);
            break;
        case BinaryOpFunc::Div:
            if (node->get_value_type().basic() == BasicType::Float)
                node->set_float_value(lchild_float_value / rchild_float_value);
            else
                node->set_int_value(lchild_int_value / rchild_int_value);
            break;
        case BinaryOpFunc::Mod:
            if (node->get_value_type().basic() == BasicType::Float)
                semantic_error("Mod operation of float type operands.", node->get_line_no(), 15);
                // node->set_float_value(lchild_float_value % rchild_float_value);
                // node->set_is_literal(false);
            else
                node->set_int_value(lchild_int_value % rchild_int_value);
            break;
        case BinaryOpFunc::Equal:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value == rchild_float_value);
            else
                node->set_int_value(lchild_int_value == rchild_int_value);
            break;
        case BinaryOpFunc::NotEqual:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value != rchild_float_value);
            else
                node->set_int_value(lchild_int_value != rchild_int_value);
            break;
        case BinaryOpFunc::Great:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value > rchild_float_value);
            else
                node->set_int_value(lchild_int_value > rchild_int_value);
            break;
        case BinaryOpFunc::GreatEqual:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value >= rchild_float_value);
            else
                node->set_int_value(lchild_int_value >= rchild_int_value);
            break;
        case BinaryOpFunc::Less:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value < rchild_float_value);
            else
                node->set_int_value(lchild_int_value < rchild_int_value);
            break;
        case BinaryOpFunc::LessEqual:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value <= rchild_float_value);
            else
                node->set_int_value(lchild_int_value <= rchild_int_value);
            break;
        case BinaryOpFunc::And:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value && rchild_float_value);
            else
                node->set_int_value((int)(lchild_int_value && rchild_int_value));
            break;
        case BinaryOpFunc::Or:
            if (lchild->get_value_type().basic() == BasicType::Float)
                node->set_int_value(lchild_float_value || rchild_float_value);
            else
                node->set_int_value((int)(lchild_int_value || rchild_int_value));
            break;
        default:
            break;
        }
    }
}
void SemanticAnalysis::analysis_unary_op(ASTNode *node,int state)
{
    //非递归分析
    if (state == (int)SemanticAnalysisState::BeforeRecursion) {
        current_stack->push({ node, (int)SemanticAnalysisState::AfterRecursion });
        for(auto iter=node->get_child().rbegin();iter!=node->get_child().rend();iter++){
            assert(*iter != nullptr);
            push_node(*iter);
        }
        return;
    }
    ASTNode *child = node->get_child(0);
    if (child->get_value_type().is_array())
        semantic_error("Operand is an array.", node->get_line_no(), 16);
    //判断BasicType
    if (child->get_value_type().basic() == BasicType::Int)
        node->get_value_type().set_basic_type(BasicType::Int);
    else {
        if (ASTNode::is_condition_operation_unary((UnaryOpFunc)node->get_func()))
            node->get_value_type().set_basic_type(BasicType::Int);
        else node->get_value_type().set_basic_type(BasicType::Float);
    }
    if (child->is_lvalue()) {
        //插入隐式类型转换结点
        ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(child->get_value_type(), ImplicitCastFunc::LValueToRValue, child);
        node->set_child(0, implicit_cast_node);
        semantic_analysis_recursion(implicit_cast_node);
        child = node->get_child(0);
    }
    //计算literal
    if (child->is_literal()) {
        node->set_is_literal(true);
        float child_float_value = child->get_literal_value().float_value;
        int child_int_value = child->get_literal_value().int_value;
        switch ((UnaryOpFunc)node->get_func()) {
        case UnaryOpFunc::Positive:
            if (node->get_value_type().basic() == BasicType::Float)
                node->set_float_value(child_float_value);
            else
                node->set_int_value(child_int_value);
            break;
        case UnaryOpFunc::Negative:
            if (node->get_value_type().basic() == BasicType::Float)
                node->set_float_value(-child_float_value);
            else
                node->set_int_value(-child_int_value);
            break;
        case UnaryOpFunc::Not:
            if (child->get_value_type().basic() == BasicType::Float)
                node->set_int_value(!child_float_value);
            else
                node->set_int_value(!child_int_value);
            break;
        default:
            break;
        }
    }
}
void SemanticAnalysis::analysis_func_r_params(ASTNode *node )
{
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    //检查函数实参和形参数量是否匹配
    assert(node->get_symbol() != nullptr);
    if (node->get_child().size() != node->get_symbol()->get_param_type().size()) {
        semantic_error("The FuncRParams of FuncCall does not conform to the FuncFParams of FuncDef.", node->get_line_no(), 17);
    }
    for (int i = 0; i < node->get_child().size(); i++) {
        ASTNode *child = node->get_child(i);
        ValueType value_type = node->get_symbol()->get_param_type(i);
        if (child->is_lvalue()) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(child->get_value_type(), ImplicitCastFunc::LValueToRValue, child);
            node->set_child(i, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(i);
        }
        //检查函数实参和形参维度是否匹配
        if (child->get_value_type().get_dimension().size() != value_type.get_dimension().size()) {
            semantic_warning("The Dimension count of FuncRParam does not conform to the definition of FuncFParam.", node->get_line_no(), 18);
        }
        //检查函数实参和形参各维度数量是否相同
        for (int i = 0; i < value_type.get_dimension().size(); i++) {
            if (i == 0)
                //第一维不检查
                continue;
            if (child->get_value_type().length(i) != value_type.length(i)) {
                semantic_warning("A Dimension of FuncRParam does not conform to the definition of FuncFParam.", node->get_line_no(), 19);
            }
        }
        if (child->get_value_type().basic() == BasicType::Int && value_type.basic() == BasicType::Float) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(BasicType::Float, ImplicitCastFunc::IntToFloat, child);
            node->set_child(i, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(i);
        } else if (child->get_value_type().basic() == BasicType::Float && value_type.basic() == BasicType::Int) {
            //插入隐式类型转换结点
            ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(BasicType::Int, ImplicitCastFunc::FloatToInt, child);
            node->set_child(i, implicit_cast_node);
            semantic_analysis_recursion(implicit_cast_node);
            child = node->get_child(i);
        }
    }
}
void SemanticAnalysis::analysis_array_visit(ASTNode *node )
{
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    Symbol *m_symbol = sym_table->get_current_sym(node->get_var_name());
    ASTNode *base = node->get_child(0);
    ASTNode *offset = node->get_child(1);
    if (offset->get_value_type().basic() != BasicType::Int)
        semantic_error("Array index must be integer.", node->get_line_no(), 20);
    //子节点1必须为右值，插入隐式类型转换节点
    if (offset->is_lvalue()) {
        //插入隐式类型转换结点
        ASTNode* implicit_cast_node = ASTNode::create_implicit_cast(offset->get_value_type(), ImplicitCastFunc::LValueToRValue, offset);
        node->set_child(1, implicit_cast_node);
        semantic_analysis_recursion(implicit_cast_node);
        offset = node->get_child(1);
    }
    int index; //该维度下标
    int unit;  //该维度大小
    if (base->get_value_type().get_dimension().size() == 0) {
        semantic_error("Too many dimensions for array.", node->get_line_no(), 21);
    }
    if (m_symbol != nullptr) {
        //设置节点属性
        node->set_symbol(m_symbol);
        node->set_value_type(base->get_value_type());
        node->set_value(base->get_value());
        node->set_is_lvalue(true);
        node->set_is_literal(base->is_literal() && offset->is_literal());
        index = offset->get_int_value();
        if (node->is_literal()) {
            unit = node->get_value_type().total_length() / node->get_value_type().length(0);
            node->set_int_value(node->get_int_value() + index * unit);
        }
        //完成该维度计算，用0表示
        node->get_value_type().pop_dimension();
    } else {
        stringstream error;
        error << "Undefined Array \'" << node->get_var_name() << "\' .";
        semantic_error(error.str(), node->get_line_no(), 22);
    }
}
void SemanticAnalysis::analysis_block(ASTNode *node,int state)
{
    //非递归分析
    if (state == (int)SemanticAnalysisState::BeforeRecursion) {
        current_stack->push({ node, (int)sym_table->get_stack_size() });
        for (auto iter = node->get_child().rbegin(); iter != node->get_child().rend(); iter++) {
            assert(*iter != nullptr);
            push_node(*iter);
        }
        return;
    }
    int size_of_stack = state;
    //从符号栈中删除局部变量
    while ((int)sym_table->get_stack_size() > size_of_stack) {
        sym_table->delete_top_sym();
    }
}
void SemanticAnalysis::analysis_return_stmt(ASTNode *node)
{
    //递归
    for (auto &child : node->get_child()) {
        assert(child != nullptr);
        semantic_analysis_recursion(child);
    }
    if (node->get_child().size() > 0) {
        ASTNode *child = node->get_child(0);
        if (return_type == BasicType::Void) {
            semantic_error("The return value of function does not conform to the definition.", node->get_line_no(), 23);
        } else {
            if (child->is_lvalue()) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(child->get_value_type(), ImplicitCastFunc::LValueToRValue, child);
                node->set_child(0, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                child = node->get_child(0);
            }
            if (child->get_value_type().basic() == BasicType::Int && return_type == BasicType::Float) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(BasicType::Float, ImplicitCastFunc::IntToFloat, child);
                node->set_child(0, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                child = node->get_child(0);
            } else if (child->get_value_type().basic() == BasicType::Float && return_type == BasicType::Int) {
                //插入隐式类型转换结点
                ASTNode *implicit_cast_node = ASTNode::create_implicit_cast(BasicType::Int, ImplicitCastFunc::FloatToInt, child);
                node->set_child(0, implicit_cast_node);
                semantic_analysis_recursion(implicit_cast_node);
                child = node->get_child(0);
            }
        }
    } else {
        if (return_type != BasicType::Void)
            semantic_error("The return value of function does not conform to the definition.", node->get_line_no(), 24);
    }
}

void SemanticAnalysis::semantic_error(const std::string &&error_msg, int line_no, int error_code)
{
    std::cerr << "[Semantic error] line " << line_no << " : " << error_msg << std::endl;
    exit(ErrorCode::SEMANTIC_ERROR + error_code);
}
void SemanticAnalysis::semantic_warning(const std::string&& error_msg, int line_no, int error_code)
{
    std::cerr << "[Semantic warning] line " << line_no << " : " << error_msg << std::endl;
}
void SemanticAnalysis::array_init_search(ASTNode *node, int dimension, std::vector<std::size_t> length, BasicType type, Symbol *symbol,int& offset)
{
    assert(symbol != nullptr);
    int counter = 0;
    int unit_size = 1;
    int unit_counter = 0;
    int unit_length;
    int d;
    int dimensions = length.size();
    for (d = dimensions; d > dimension; d--)
        unit_size *= length[d - 1];
    unit_length = length[d - 1];
    if (dimension > dimensions)
        semantic_error("Too many array dimensions.", node->get_line_no(), 25);
    for (auto &child : node->get_child()) {
        if (child->get_type() == ASTType::ArrayInitVal) {
            if (counter != 0) {
                counter = 0;
                offset += unit_size - counter;
                unit_counter++;
                if (unit_counter == unit_length)
                    break;
            }
            array_init_search(child, dimension + 1, length, type, symbol,offset);
            unit_counter++;
        } else {
            if (!child->is_literal()){
                if(symbol->is_const()){
                    //常量数组必须用字面值初始化
                    semantic_error("Invalid initial value of array.", node->get_line_no(), 26);
                }else{
                    //遇到非字面值初始化变量数组，此情况下本函数不应该被调用
                    assert(false);
                }
            }else{
                if (type == BasicType::Float)
                    symbol->add_float_value(child->get_literal_value().float_value, offset++);
                else
                    symbol->add_int_value(child->get_literal_value().int_value, offset++);
            }
            counter++;
            if (counter == unit_size) {
                counter = 0;
                unit_counter++;
            }
        }
        if (unit_counter == unit_length)
            break;
        /*
        if (unit_counter > unit_length || unit_counter == unit_length && counter != 0) {
            // semantic_error("Too many array elements.", node->get_line_no(), 27);
            break;
        }*/
    }
    //先补全一个unit
    offset += unit_size - counter;
    unit_counter++;
    //再补全所有的unit
    offset += (unit_length - unit_counter) * unit_size;
}