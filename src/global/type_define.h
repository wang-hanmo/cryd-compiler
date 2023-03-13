#pragma once
#include <vector>
#include <sstream>
#define SIZE_OF_BASIC_TYPE 4
//基本值类型
enum class BasicType:int
{
    None,       //节点不存在类型
    Void,       //void型，仅用于函数返回值
    Int,
    Float,
    Uncertain
};
//基本值
union BasicValue
{
    //float
    float float_value;
    //int
    int int_value;
    //pointer
    void* pointer;
    BasicValue(){}
    BasicValue(int value);
    BasicValue(float value);
    BasicValue(void* pointer);
    static BasicValue create_int(int value);
    static BasicValue create_float(float value);
    static BasicValue create_pointer(void* pointer);
    static BasicValue zero();
};
//变量的作用域类型
enum class VarKind:int
{
    Global, //全局变量
    Local,  //局部变量
    Param,  //函数形参
    Func,   //全局函数
    Null,   //什么也不是
};

//变量类型
class ValueType
{
private:
    BasicType m_basic_type;
    std::vector<std::size_t> m_length;//每一个维度的大小,如果不是数组则为空
public:
    std::string get_string()const;
    bool is_array()const;
    ValueType(BasicType i_basic_type,const std::vector<std::size_t>& i_length={}):m_basic_type(i_basic_type),m_length(i_length){}
    bool operator==(const ValueType& rhs);
    bool operator!=(const ValueType& rhs);
    BasicType basic() const;
    void set_basic_type(BasicType type);
    std::size_t length(int dimension) const;         //获取指定维度长度
    
    std::vector<std::size_t> get_dimension() const; //获取m_length
    void set_dimension(std::vector<std::size_t> i_length);   //设置m_length
    void add_dimension(std::size_t length);         //添加m_length成员
    size_t total_length() const;                    //数组在当前维度下的总长度
    size_t top_unit_length()const;                  //数组第一维一个元素的长度
    void set_array_length(int dimension,int len);   //设置指定维度长度
    void pop_dimension();                           //降低数组维数
};
std::ostream& operator <<(std::ostream& os,BasicType vtype);

//AST节点类型
enum class ASTType:int
{
    CompUnit,       //编译单元
    DeclStmt,       //声明语句，可以有多个并列子语句
    VarDecl,        //变量声明
    ConstDecl,      //常量声明
    ConstValue,     //常量值
    IfStmt,         //if语句
    SwitchStmt,     //switch语句
    Case,           //switch语句中不同条件的值
    Default,        //switch语句中default
    WhileStmt,      //while语句
    DoWhileStmt,    //do-while语句
    ReturnStmt,     //返回语句
    BreakStmt,      //break语句
    ContinueStmt,   //continue语句
    NullStmt,       //空语句
    UnaryOp,        //单目运算符
    BinaryOp,       //双目运算符
    FuncDef,        //函数定义
    Block,          //语句块
    FuncRParams,    //实参表
    FuncFParams,    //形参表
    FuncFParam,     //形参
    Ident,          //符号
    ImplicitCast,   //隐式类型转换
    FuncCall,       //函数调用
    ArraySize,      //数组各维度大小
    ArrayInitVal,   //数组初值
    ArrayVisit,     //数组访问
    Unknown
};

//AST节点的功能类型
enum class BinaryOpFunc:int
{
    Assign,     //
    Add,        //+
    Sub,        //-
    Mul,        //*
    Div,        ///
    Mod,        //%
    Equal,      //==
    NotEqual,   //!=
    Great,      //>
    Less,       //<
    GreatEqual, //>=
    LessEqual,  //<=
    And,        //&&
    Or,         //||
};
enum class UnaryOpFunc:int
{
    Positive,   //正号+
    Negative,   //负号-
    Not,        //取非!
    Paren,      //圆括号()
};
enum class ImplicitCastFunc:int
{
    LValueToRValue,//左值到右值
    IntToFloat,    //整数到浮点数
    FloatToInt    //浮点数到整数
};
//获得各种类型的字符串表示，以便输出中间过程信息
std::string get_basic_type_string(BasicType type);
std::string get_var_kind_string(VarKind kind);
std::string get_binary_op_string(BinaryOpFunc func);
std::string get_unary_op_string(UnaryOpFunc func);
std::string get_implicit_cast_string(ImplicitCastFunc func);

enum ErrorCode: int
{
    LEXICAL_ERROR = 160,        //词法错误, 160
    SYNTAX_ERROR = 161,         //语法错误, 161
    AST_ERROR = 162,            //抽象语法树错误, 162
    SEMANTIC_ERROR = 163,       //语义错误, 163~189
    SYMBOL_ERROR = 190,         //符号错误, 190
    IR_ERROR = 191,             //中间表示错误, 191
    CFG_ERROR = 192,            //控制流图错误, 192~204
    OPTIMIZER_ERROR = 205,      //优化错误, 205
    LIVE_ERROR = 206,           //数据活跃分析错误, 206~207
    REGISTER_ERROR = 208,       //寄存器错误, 208~220
    ARM_ERROR = 221             //汇编错误, 221~250
};