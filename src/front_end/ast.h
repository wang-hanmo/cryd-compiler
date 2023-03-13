#pragma once
#include <iostream>
#include <string>
#include <symbol_table.h>
#include <type_define.h>
#include <vector>
class ASTNode
{
private:
    //对应源程序的行号
    int m_line_no{-1};
    //子节点
    std::vector<ASTNode *> m_child{{}};
    //基本类型
    ASTType m_type{ASTType::Unknown};
    //值类型 int或float
    ValueType m_value_type{ValueType(BasicType::None)};
    //是否为左值
    bool m_is_lvalue{false};
    //是否为字面值
    bool m_is_literal{false};
    //常数值
    BasicValue m_value;
    //具体功能类型,有一些节点没有功能类型就是0
    int m_func{0};
    //变量名(如果是Decl节点)
    std::string m_var_name{""};
    //对应符号在符号表中的项
    Symbol *m_symbol{nullptr};
    
    //构造函数，外部不可访问。在外部创建节点的话用create
    ASTNode(const ASTType i_type,
            const std::vector<ASTNode *> &i_child_list,
            const BasicValue i_value,
            const int i_func,
            std::string i_var_name,
            const ValueType i_value_type,
            const bool i_is_lvalue,
            const bool i_is_literal);

public:
    //通用节点构建函数
    //使用实例： $$=ASTNode::create(ASTType::Block,{$1,$2,$3});
    static ASTNode *create(const ASTType type,
                           const std::vector<ASTNode *> &child_list,
                           const BasicValue value,
                           const int func = 0,
                           std::string var_name = "",
                           const ValueType value_type = ValueType(BasicType::None),
                           const bool is_lvalue = false,
                           const bool is_literal = false);
    //构造各种节点的静态函数
    static ASTNode *create_comp_unit();                                                                                                    //建立编译单元
    static ASTNode *create_decl_stmt(ValueType type);                                                                                      //建立一个空declstmt结点
    static ASTNode *create_var_decl(ValueType type, std::string var_name, bool is_array, std::vector<ASTNode *> children);                 //建立一个变量定义节点
    static ASTNode *create_const_decl(ValueType type, std::string var_name, bool is_array, std::vector<ASTNode *> children);               //建立一个常量定义节点
    static ASTNode *create_func_def(ValueType type, std::string name);                                                                     //建立一个函数定义结点
    static ASTNode *create_block();                                                                                                        //建立一个空block结点
    static ASTNode *create_const_value(ValueType type, BasicValue value);                                                                  //建立一个字面值节点
    static ASTNode *create_binary_op(BinaryOpFunc func, ValueType value_type, ASTNode *left_child, ASTNode *right_child);                  //建立一个双目运算符节点
    static ASTNode *create_unary_op(UnaryOpFunc func, ValueType value_type, ASTNode *child, bool is_lvalue);                               //建立一个单目运算符节点
    static ASTNode *create_if_stmt(ASTNode *condition, ASTNode *true_stmt, ASTNode *false_stmt = nullptr);                                 //建立一个if结点
    static ASTNode *create_switch_stmt(std::vector<ASTNode*> children);                                                                    //建立一个switch结点
    static ASTNode *create_case(BasicValue value, ASTNode* child);                                                                         //建立一个case结点
    static ASTNode *create_default(ASTNode* child);                                                                                        //建立一个default结点
    static ASTNode *create_while_stmt(ASTNode *condition, ASTNode *stmt);                                                                  //建立一个while结点
    static ASTNode *create_do_while_stmt(ASTNode* condition, ASTNode* stmt);                                                               //建立一个do-while结点
    static ASTNode *create_break_stmt();                                                                                                   //建立一个break结点
    static ASTNode *create_continue_stmt();                                                                                                //建立一个continue结点
    static ASTNode *create_null_stmt();                                                                                                    //建立一个空语句节点
    static ASTNode *create_return_stmt(ASTNode *return_value_node = nullptr);                                                              //建立一个return结点
    static ASTNode *create_ident(ValueType type, std::string name, bool is_lvalue);                                                        //建立一个标识符节点
    static ASTNode *create_func_call(std::string name);                                                                                    //建立一个函数调用节点
    static ASTNode *create_func_r_params();                                                                                                //建立一个空的rparams节点
    static ASTNode *create_func_f_params();                                                                                                //建立一个空的fparams节点
    static ASTNode *create_func_f_param(ValueType type, std::string name, bool is_array, bool is_lvalue, std::vector<ASTNode *> children); //建立一个fparam节点
    static ASTNode *create_implicit_cast(ValueType type_after_cast, ImplicitCastFunc func, ASTNode *child);                                //建立一个隐式类型转换节点
    static ASTNode *create_array_size();                                                                                                   //建立一个ArraySize节点
    static ASTNode *create_array_init_val(ValueType type);
    static ASTNode *create_array_visit(ValueType type, ASTNode *ident, ASTNode *subscript); //建立一个ArrayInitVal节点
    static ASTNode *copy_condition(ASTNode* node); //复制整个条件子树，包括所有child
    //工具函数
    //判断操作类型的返回结果是否为布尔值
    static bool is_condition_operation_binary(BinaryOpFunc binary_op_func);
    static bool is_condition_operation_unary(UnaryOpFunc unary_op_func);
    // get函数
    ASTType get_type();
    int get_func();
    Symbol *get_symbol();
    ValueType &get_value_type();
    //获取值
    BasicValue get_value();
    //获取变量名
    std::string get_var_name();
    //获得子节点列表
    const std::vector<ASTNode *>& get_child();
    std::vector<ASTNode*>& get_child_not_const();
    //获得单个子节点指针
    ASTNode *get_child(int index);
    //是否为左值
    bool is_lvalue();
    //是否为字面常量
    bool is_literal();
    // 获取整型值
    int get_int_value();
    //获取浮点型值
    float get_float_value();
    //获取节点对应源程序行号
    int get_line_no();
    // set函数
    void set_value_type(ValueType type);
    void set_var_name(std::string name);
    void set_symbol(Symbol *symbol);
    void set_is_lvalue(bool is_lvalue);
    void set_is_literal(bool is_literal);
    void set_child(int index, ASTNode *child);
    void set_int_value(int value);
    void set_float_value(float value);
    void set_value(BasicValue value);
    void clear_child();
    //重写类型（只重写类型，不考虑其他）
    void rewrite_type(ASTType type);
    void add_child(ASTNode *child);                  //子节点列表最后加入一个新的子节点
    void add_child(std::vector<ASTNode *> children); //加入一些新的子节点
    //void remove_first_child();
    
    //销毁包括自己在内的整个子树
    static void destory(ASTNode *target);

    //打印抽象语法树
    void printNode(int tier = 0, std::ostream &os = std::cout);
    //获取字面值，如果是数组，自动根据计算出的偏移量取值
    BasicValue get_literal_value(); 
    //默认构造函数
    ASTNode() {}
    //默认析构函数
    ~ASTNode(){};
};
extern ASTNode *g_ast_root;