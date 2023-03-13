#include <if_to_switch.h>
#include <limits.h>
#include <math.h>

using namespace std;

void IfToSwitch::run()
{
    std::cout << "Running pass: If To Switch on AST" << std::endl;
    if (m_ast == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    reset();
    convert(m_ast);
}

void IfToSwitch::convert(ASTNode* node)
{
    assert(node != nullptr);
    ASTNode* child;
    for (int i = 0; i < node->get_child().size(); i++) {
        child = node->get_child(i);
        if_else_to_switch(child);
        reset();
        if_to_switch(child);
        reset();
        convert(child);
    }
}

bool IfToSwitch::if_else_to_switch(ASTNode* node)
{
    assert(node != nullptr);
    if (node->get_type() == ASTType::IfStmt) {
        auto& children = node->get_child();
        auto cond = children[0];
        if ((BinaryOpFunc)cond->get_func() != BinaryOpFunc::Equal)
            return false;
        // 注意Equal的子节点存在左值->右值的隐式类型转换
        ASTNode* lchild;
        if (cond->get_child(0)->get_type() == ASTType::ImplicitCast)
            lchild = cond->get_child(0)->get_child(0);
        else
            lchild = cond->get_child(0);
        if (lchild->get_type() != ASTType::Ident || cond->get_child(1)->get_type() != ASTType::ConstValue ||
            cond->get_child(1)->get_value_type().basic() != BasicType::Int)
            return false;
        if (ident == nullptr) {
            ident = lchild->get_symbol();
        } else if (ident != lchild->get_symbol())
            return false;
        case_max = max(cond->get_child(1)->get_int_value(), case_max);
        case_min = min(cond->get_child(1)->get_int_value(), case_min);
        if (case_max > case_min && case_max - case_min > 50)
            return false;
        branch_val.emplace_back(cond->get_child(1));
        branch_block.emplace_back(children[1]);
        if (children.size() == 3) {
            if (children[2]->get_type() != ASTType::IfStmt) {
                default_block = children[2];
            } else {
                if (!if_else_to_switch(children[2]))
                    return false;
            }
        }
        // 第一层else
        if (branch_val[0] == cond->get_child(1)) {
            if (branch_val.size() >= 4) {
                // auto ident_node = lchild;
                // 删除原来的结点
                node->clear_child();
                // 构造switch结点
                node->rewrite_type(ASTType::SwitchStmt);
                node->add_child(cond->get_child(0));
                for (int i = 0; i < branch_val.size(); ++i) {
                    auto case_node = ASTNode::create_case(branch_val[i]->get_value(), branch_block[i]);
                    node->add_child(case_node);
                }
                auto default_node = ASTNode::create_default(default_block);
                node->add_child(default_node);
                std::cerr << "convert to switch" << std::endl;
                return true;
            }
            return false;
        } else {
            // 如果不是第一层且需要转换成switch，就将自身销毁
            if (branch_val.size() >= 4) {
                delete(node);
                return true;
            }
            return false;
        }
    }
    return false;
}

void IfToSwitch::if_to_switch(ASTNode* node)
{
    assert(node != nullptr);
    ASTNode* lchild;
    // 寻找多个并列的if结构
    auto& node_child = node->get_child_not_const();
    for (auto iter = node_child.begin(); iter != node_child.end(); ++iter) {
        auto child = *iter;
        assert(child != nullptr);
        if (child->get_type() == ASTType::IfStmt) {
            auto& children = child->get_child();
            auto cond = children[0];
            if ((BinaryOpFunc)cond->get_func() != BinaryOpFunc::Equal) {
                reset();
                continue;
            }
            // 注意Equal的子节点存在左值->右值的隐式类型转换
            if (cond->get_child(0)->get_type() == ASTType::ImplicitCast)
                lchild = cond->get_child(0)->get_child(0);
            else
                lchild = cond->get_child(0);
            if (lchild->get_type() != ASTType::Ident || cond->get_child(1)->get_type() != ASTType::ConstValue ||
            cond->get_child(1)->get_value_type().basic() != BasicType::Int) {
                reset();
                continue;
            }
            if (ident == nullptr) {
                ident = lchild->get_symbol();
            } else if (ident != lchild->get_symbol()) {
                reset();
                // 把这个if语句块当作开始，重新搜索
                ident = lchild->get_symbol();
            }
            if (!check(children[1])) {
                reset();
                continue;
            }
            if (children.size() == 3) {
                reset();
                continue;
            }
            case_max = max(cond->get_child(1)->get_int_value(), case_max);
            case_min = min(cond->get_child(1)->get_int_value(), case_min);
            if (case_max > case_min && case_max - case_min > 50) {
                reset();
                continue;
            }
            branch_val.emplace_back(cond->get_child(1));
            branch_block.emplace_back(children[1]);
        } else {
            if (branch_val.size() >= 4) {
                // 构造switch结点
                // auto ident_node = lchild;
                std::vector<ASTNode*> switch_children;
                switch_children.emplace_back(child->get_child(0));
                for (int i = 0; i < branch_val.size(); ++i) {
                    auto case_node = ASTNode::create_case(branch_val[i]->get_value(), branch_block[i]);
                    switch_children.emplace_back(case_node);
                }
                auto default_node = ASTNode::create_default(nullptr);
                switch_children.emplace_back(default_node);
                auto switch_node = ASTNode::create_switch_stmt(switch_children);
                for (int i = 0; i < branch_val.size(); ++i) {
                    // 删除了被代替的if结点，并将switch结点插入
                    --iter;
                    iter = node_child.erase(iter);
                }
                iter = node_child.insert(iter, switch_node);
                std::cerr << "convert to switch" << std::endl;
            }
            reset();
        }
    }
}

bool IfToSwitch::check(ASTNode* node)
{
    assert(node != nullptr);
    ASTNode* child;
    bool flag = true;
    // 若if cond 为 i == a，则语句块内不能对i有赋值
    for (int i = 0; i < node->get_child().size(); ++i) {
        child = node->get_child(i);
        if (child->get_type() == ASTType::BinaryOp && (BinaryOpFunc)child->get_func() == BinaryOpFunc::Assign) {
            if (((child->get_child())[0])->get_symbol() == ident)
                return false;
        }
        if (!check(child))
            flag = false;
    }
    return flag;
}

void IfToSwitch::reset()
{
    default_block = nullptr;
    ident = nullptr;
    branch_block.clear();
    branch_val.clear();
    case_max = INT_MIN;
    case_min = INT_MAX;
}