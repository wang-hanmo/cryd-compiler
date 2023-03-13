#pragma once
#include <cfg.h>
#include <optimizer.h>
class ConstantFoldAST final :public Pass
{
private:
    void fold(ASTNode* node);
public:
    ConstantFoldAST(bool i_emit) :Pass(PassType::ConstantFoldAST,PassTarget::AST, i_emit) {}
    void run();
};