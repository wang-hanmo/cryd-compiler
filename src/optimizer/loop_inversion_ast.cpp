#include <loop_inversion_ast.h>
#include <string>
#include <iostream>
#include <cassert>
void LoopInversionAST::run()
{
    std::cout << "Running pass: Loop Inversion" << std::endl;
    if (m_ast == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    work(m_ast);
}
void LoopInversionAST::work(ASTNode* node)
{
    ASTNode* child;
    for (int i = 0; i < node->get_child().size(); i++) {
        child = node->get_child(i);
        work(child);
        if (child->get_type() == ASTType::WhileStmt) {
            ASTNode* cond= ASTNode::copy_condition(child->get_child(0));
            child->rewrite_type(ASTType::DoWhileStmt);
            ASTNode* if_stmt = ASTNode::create_if_stmt(cond, child);
            node->set_child(i, if_stmt);
        }
    }
}