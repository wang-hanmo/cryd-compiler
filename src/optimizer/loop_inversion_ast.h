#pragma once
#include <cfg.h>
#include <optimizer.h>
class LoopInversionAST final :public Pass
{
private:
    void work(ASTNode* node);
public:
    LoopInversionAST(bool i_emit) :Pass(PassType::LoopInversionAST,PassTarget::AST, i_emit) {}
    void run();
};