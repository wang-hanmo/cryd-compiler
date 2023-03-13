#pragma once
#include <ast.h>
#include <stack>
enum class SemanticAnalysisState:int
{
    BeforeRecursion = -1,
    AfterRecursion = 0,
};
class SemanticAnalysis
{
private:
    static bool in_global_area;         //�Ƿ���ȫ����
    static BasicType return_type;   //��������ֵ����
    static int in_while_stmt;          //�Ƿ���whileѭ����
    static SymbolTable* sym_table;     //���ű�
    //��ǰ���������ջ�����ڷǵݹ�������ڶ���������״̬����ÿ������������ʹ��������״̬
    static std::stack<std::pair<ASTNode*, int>>* current_stack;
    static void semantic_error(const std::string&& error_msg, int line_no, int error_code);
    static void  semantic_warning(const std::string&& error_msg, int line_no, int error_code);
    //����ֵ��ʾ�Ƿ����г�ʼ�����Ϊ����
    static void array_init_search(ASTNode* node, int dimension, std::vector<std::size_t> length, BasicType type, Symbol* symbol,int& offset);
    static void analysis_(ASTNode* node);
    static void analysis_func_def(ASTNode* node);
    static void analysis_ident(ASTNode* node);
    static void analysis_func_call(ASTNode* node);
    static void analysis_implicit_cast(ASTNode* node);
    static void analysis_var_decl(ASTNode* node);
    static void analysis_const_decl(ASTNode* node);
    static void analysis_array_init_val(ASTNode* node);
    static void analysis_while_stmt(ASTNode* node);
    static void analysis_if_stmt(ASTNode* node);
    static void analysis_break_stmt(ASTNode* node);
    static void analysis_continue_stmt(ASTNode* node);
    static void analysis_func_f_param(ASTNode* node);
    static void analysis_func_f_params(ASTNode* node);
    static void analysis_binary_op(ASTNode* node,int state);
    static void analysis_unary_op(ASTNode* node,int state);
    static void analysis_func_r_params(ASTNode* node);
    static void analysis_array_visit(ASTNode* node);
    static void analysis_block(ASTNode* node, int state);
    static void analysis_return_stmt(ASTNode* node);
    static void semantic_analysis_recursion(ASTNode* node);
    static void push_node(ASTNode* node);
public:
    static void semantic_analysis(ASTNode* node,SymbolTable* sym_table);

};