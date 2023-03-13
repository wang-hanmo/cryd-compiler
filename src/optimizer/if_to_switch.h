#pragma once
#include <optimizer.h>
#include <../ir/cfg.h>

class IfToSwitch final :public Pass
{
private:
    // 分支数量大于4时就将if-else转化为switch
    std::vector<ASTNode*> branch_val;
    std::vector<ASTNode*> branch_block;
    ASTNode* default_block;
    Symbol* ident;
    int case_max;
    int case_min;
    void convert(ASTNode* node);
    // 多个并列的if结构转化为switch
    void if_to_switch(ASTNode* node);
    bool if_else_to_switch(ASTNode* node);
    // 满足以下条件才能if|if-else转化为switch
    // 1、条件必须是 Ident == ConstIntValue的形式
    // 2、对于多个平级if，Ident的值在语句块中未被改变过(简单起见，限定Ident不能是全局变量，否则需要考虑函数调用)
    // 3、case的条件Max-Min <= 50
    bool check(ASTNode* node);
    void reset();
public:
    IfToSwitch(bool i_emit) :Pass(PassType::IfToSwitch,PassTarget::AST, i_emit) {}
    void run();
};