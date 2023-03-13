#include <constant_fold_ast.h>
#include <string>
#include <iostream>
#include <cassert>
void ConstantFoldAST::run()
{
    std::cout << "Running pass: Constant Fold on AST" << std::endl;
    if (m_ast == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    fold(m_ast);
}
void ConstantFoldAST::fold(ASTNode* node)
{
    ASTNode* child;
    for (int i = 0; i < node->get_child().size(); i++) {
        child = node->get_child(i);
        assert(child != nullptr);
        if (child->is_literal() && 
            !(child->get_type() == ASTType::ArrayInitVal) && 
            !child->get_value_type().is_array() ) {
            if (child->get_type() != ASTType::ConstValue) {
                BasicValue value = child->get_literal_value();
                ASTNode* constant_child = ASTNode::create_const_value(child->get_value_type(), value);
                ASTNode::destory(child);
                node->set_child(i, constant_child);
            }
        } else {
            fold(child);
        }
    }
}