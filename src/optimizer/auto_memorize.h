#pragma once

#include <optimizer.h>
#include <../ir/cfg.h>
/*
*   为黑盒递归函数自动引入记忆化搜索
*/
class AutoMemorize final :public Pass
{
private:
    int max_temp_index{0};
    std::set<IRBlock*> visited;
    // 缓存递归结果的全局数组
    std::vector<IRSymbol*> Cache;
    std::vector<IRSymbol*> CheckCache;
    //std::vector<IRSymbol*> new_local;
    void rewrite_return(IRBlock* block, IRSymbol* return_sym);
    void create_func_cache(IRSymbol* func);
    bool check_memorize(IRSymbol* func);
    void ir_list_find_index(IRInstrList& program);
    void find_max_index(IRUnit* unit);
    void insert_return_statement(IRUnit* unit, IRSymbol* param, IRSymbol* return_sym);
    void insert_use_cache_statement(IRUnit* unit, IRSymbol* param, IRSymbol* return_sym);

    bool check_memorize_2_param(IRSymbol* func);
    void insert_return_statement_2_param(IRUnit* unit, IRSymbol* param1, IRSymbol* param2, IRSymbol* return_sym);
    void insert_use_cache_statement_2_param(IRUnit* unit, IRSymbol* param1, IRSymbol* param2, IRSymbol* return_sym);
public:
    AutoMemorize(bool i_emit) :Pass(PassType::AutoMemorize, PassTarget::CFG, i_emit) {}
    void run();
};