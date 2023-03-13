#pragma once
#include <optimizer.h>
#include <cfg.h>
#include <set>
/*
* 基本块简化优化，包含不可达块删除、空块删除和相邻块合并
*/
class BlockSimplification final :public Pass
{
private:
    std::set<IRBlock*> visited;
    bool is_block_empty(IRBlock* block);
    void unreachable_block_elimination(IRBlock* block, IRBlock* entry);
    void empty_block_elimination(IRBlock* block);
    void block_merge(IRBlock* block);
public:
    BlockSimplification(bool i_emit) :Pass(PassType::BlockSimplification,PassTarget::CFG, i_emit) {}
    void run();
};
