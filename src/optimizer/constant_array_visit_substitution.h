#pragma once
#include <optimizer.h>
#include <../ir/cfg.h>

class ConstantArrayVisitSubstitutionAST final :public Pass
{
private:
    std::set<IRBlock*> visited;
    // 被定值过的数组(初始化不算)
    std::set<Symbol*> array_def;
    Symbol* find_array_symbol_for_visit(ASTNode* node);
    Symbol* find_array_symbol_for_func(ASTNode* node);
    void find_array_undef(ASTNode* node);
    void substitution(ASTNode* node);
    ASTNode* search_init_val(ASTNode* node, Symbol* arr, int offset);
    ASTNode* get_init_val(ASTNode* node, int cur_dim, const std::vector<std::size_t>& length, int& offset, int target_offset, Symbol* arr);
    std::tuple<std::vector<size_t>, size_t, int> get_offset(ASTNode* node);
public:
    ConstantArrayVisitSubstitutionAST(bool i_emit) :Pass(PassType::BlockSimplification,PassTarget::CFG, i_emit) {}
    void run();
};