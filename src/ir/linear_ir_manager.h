#pragma once
#include<ir_instr.h>
/*
    保证性质：
    1、每个函数局部变量、形参的index从0开始编号，并且连续。
    2、临时变量从最后一个局部变量、形参index的下一个数字开始，并且连续
    3、标号从0开始，使用与局部变量、形参、临时变量不同的一套编号机制
    4、对局部变量、形参、全局变量、函数的定义、引用语句，其中的r/a/b均指向同一个IRSymbol。
*/
class LinearIRManager
{
private:
    static int s_var_num;//(下一个该使用的)临时变量序号
    static int s_lbl_num;//(下一个该使用的)标号序号
    static IRSymbolTable* s_ir_sym_table;//IR符号表
    static SymbolTable* s_ast_sym_table;//AST符号表
    /*******************************
        以下函数返回一个或多个操作数
    ********************************/
    /*
        对右值子树生成IR，目标节点可以为以下几类之一
        全局变量、局部变量、字面值、双目运算符、单目运算符
        返回操作数类型为temp、value、local、param、global之一
    */
    static std::pair<IRInstrList*, IRSymbol*> gen_rvalue_expand_array_visit(ASTNode* node, IRSymbol* res_oprand);
    static std::pair<IRInstrList*, IRSymbol*> gen_rvalue_expand(ASTNode* node);
    //与上一个函数功能相同，但返回类型只能为temp
    static std::pair<IRInstrList*, IRSymbol*> gen_rvalue_noexpand(ASTNode* node);
    /*
        对左值子树生成IR，目标节点可以为以下几类之一
        全局变量 (返回一个操作数)
        局部变量 (返回一个操作数)
        数组访问（返回两个操作数，第一个表示基地址，第二个表示偏移量）
    */
    static std::tuple<IRInstrList*, IRSymbol*, IRSymbol*> gen_lvalue(ASTNode* node);
    static std::pair<IRInstrList*, IRSymbol*> gen_unary_op(ASTNode* node);     //返回操作数类型为temp
    static std::pair<IRInstrList*, IRSymbol*> gen_binary_op(ASTNode* node);    //返回操作数类型为temp
    static IRSymbol* gen_binary_op_sub(ASTNode* node, IRInstrList* res_list, IRSymbol* oprand_lchild, IRSymbol* oprand_rchild);
    static std::pair<IRInstrList*, IRSymbol*> gen_ident(ASTNode* node);        //返回操作数类型为temp
    static std::pair<IRInstrList*, IRSymbol*> gen_const_value(ASTNode* node);  //返回操作数类型为temp
    /*
        返回值
        <0> 地址计算语句
        <1> 未处理的各维度长度列表(逆序)
        <2> 未处理的各维度长度乘积
        <3> 数组的符号
        <4> 本节点计算出的偏移量
    */
    static std::tuple<IRInstrList*, std::vector<size_t>, size_t, Symbol*, IRSymbol*> gen_array_visit(ASTNode* node);
    static std::pair<IRInstrList*, IRSymbol*> gen_func_call(ASTNode* node, bool with_return_symbol);     //返回操作数类型为temp(函数有返回值)或者nullptr(函数无返回值)
    static std::pair<IRInstrList*, IRSymbol*> gen_implicit_cast(ASTNode* node); //返回操作数类型为temp

    /*******************************
        以下函数不返回操作数
    ********************************/
    static IRInstrList* gen_comp_unit(ASTNode* node);
    static IRInstrList* gen_decl_stmt(ASTNode* node, bool gen_var_decl);
    static IRInstrList* gen_var_decl(ASTNode* node);
    static IRInstrList* gen_if_stmt(ASTNode* node, int continue_lbl = -1, int break_lbl = -1);
    static IRInstrList* gen_switch_stmt(ASTNode* node, int continue_lbl = -1, int break_lbl = -1);
    static IRInstrList* gen_while_stmt(ASTNode* node);
    static IRInstrList* gen_do_while_stmt(ASTNode* node);
    static IRInstrList* gen_return_stmt(ASTNode* node);
    static IRInstrList* gen_break_stmt(ASTNode* node, int break_lbl = -1);
    static IRInstrList* gen_continue_stmt(ASTNode* node, int continue_lbl = -1);
    /*
        对if或while内的条件子树生成IR，使用参数true_lbl和false_lbl传入跳转目标
    */
    static IRInstrList* gen_condition(ASTNode* node, int true_lbl, int false_lbl);
    static IRInstrList* gen_func_def(ASTNode* node);
    //生成子树中所有局部变量的定义语句，忽略其他节点
    static IRInstrList* gen_local_var_def(ASTNode* node);
    static IRInstrList* gen_array_init(ASTNode* node, Symbol* array_sym);
    //枚举生成数组初始化语句，递归函数，调用时请保证传入的offset=0
    static IRInstrList* gen_array_init_enum(ASTNode* node, int current_dim, const std::vector<std::size_t>& length, IRSymbol* array_base, int& offset, bool gen_zero);
    //获取初始化信息函数，返回含有初值的数量,以及第一个省略的0值的offset为多少
    static std::pair<int,int> get_array_init_info(ASTNode* node, int current_dim, const std::vector<std::size_t>& length,int& offset);
    static IRInstrList* gen_local_var_init(ASTNode* node);
    static IRInstrList* gen_func_r_params(ASTNode* node);
    static IRInstrList* gen_func_f_params(ASTNode* node);
    static IRInstrList* gen_func_f_param(ASTNode* node);
    static IRInstrList* gen_block(ASTNode* node,int continue_lbl=-1,int break_lbl=-1);
    
public:
    //为生成器绑定一个IR符号表。生成的IR符号都会存在表里
    static void bind_ir_symbol_table(IRSymbolTable* ir_symbol_table);
    //为生成器绑定一个AST符号表。生成中会从该符号表中读取一些必要信息
    static void bind_symbol_table(SymbolTable* ast_symbol_table);
    //为不同的AST节点生成IR,在外部调用时，只有第一个参数有效
    static IRInstrList* gen_ir(ASTNode* node,int continue_lbl=-1,int break_lbl=-1,bool gen_var_decl=false);
    //打印IR
    static void print_ir_list(const IRInstrList& program,std::ostream& os=std::cout,const std::string& prefix="");
    //打印一个符号
    static void print_ir_symbol(IRSymbol* sym,std::ostream& os=std::cout);
    //打印一条IR语句
    static void print_ir_instr(const IRInstr& instr,std::ostream& os=std::cout,const std::string& prefix="");
    //将rhs的元素彻底清空，全部加入lhs中，并delete rhs
    static inline void splice_ir_instr_list(IRInstrList* lhs,IRInstrList* rhs);

};