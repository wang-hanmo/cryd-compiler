#include <constant_array_visit_substitution.h>

using namespace std;

void ConstantArrayVisitSubstitutionAST::run()
{
    std::cout << "Running pass: If To Switch on AST" << std::endl;
    if (m_ast == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    find_array_undef(m_ast);
    substitution(m_ast);
}

void ConstantArrayVisitSubstitutionAST::find_array_undef(ASTNode* node)
{
    if (node->get_type() == ASTType::BinaryOp && (BinaryOpFunc)node->get_func() == BinaryOpFunc::Assign) {
        if (node->get_child(0)->get_type() == ASTType::ArrayVisit) {
            Symbol* arr = find_array_symbol_for_visit(node->get_child(0));
            array_def.emplace(arr);
        }
    } else if (node->get_type() == ASTType::FuncCall && node->get_child().size() > 0) {
        auto params = node->get_child(0);
        for (auto param: params->get_child()) {
            auto arr = find_array_symbol_for_func(param);
            if (arr != nullptr && arr->is_array())
                array_def.emplace(arr);
        }
    }
    for (int i = 0; i < node->get_child().size(); ++i) {
        find_array_undef(node->get_child(i));
    }
}

std::tuple<std::vector<size_t>, size_t, int> ConstantArrayVisitSubstitutionAST::get_offset(ASTNode* node)
{
    Symbol* array_ident_sym;
    std::vector<size_t> dim_size = {};
    size_t length = 1;
    int offset = -1;
    if (node->get_type() == ASTType::Ident) {
        array_ident_sym = node->get_symbol();
        assert(array_ident_sym != nullptr);
        const auto &size_vector = array_ident_sym->get_val_type().get_dimension();
        //逆序加入数组长度vector的所有元素,并计算其大小
        for (auto iter = size_vector.rbegin(); iter != size_vector.rend(); ++iter) {
            dim_size.push_back(*iter);
            //如果是形参，第一维长度未知，不能乘进去
            if ((*iter) != 0)
                length = length * (*iter);
        }
    } else {
        auto res_lchild = get_offset(node->get_child(0));
        dim_size = std::get<0>(res_lchild);
        length = std::get<1>(res_lchild);
        int offset_l = std::get<2>(res_lchild);
        if (node->get_child(0)->get_type() != ASTType::Ident && offset_l == -1)
            return make_tuple(dim_size, length, -1);
        if (node->get_child(1)->get_type() != ASTType::ConstValue)
            return make_tuple(dim_size, length, -1);
        int index = node->get_child(1)->get_int_value();
        if (dim_size.back() != 0) {
            length /= dim_size.back();
        }
        dim_size.pop_back();
        // 计算本层新增的偏移量
        int offset_r = length * index;
        //如果左子节点不是叶子，加上左子节点已经计算出的部分
        if (node->get_child(0)->get_type() != ASTType::Ident)
            offset = offset_l + offset_r;
        else
            offset = offset_r;
    }
    return make_tuple(dim_size, length, offset);
}

Symbol* ConstantArrayVisitSubstitutionAST::find_array_symbol_for_func(ASTNode* node)
{
    Symbol* ident = nullptr;
    if (node->get_type() == ASTType::Ident)
        return node->get_symbol();
    
    for (auto child: node->get_child()) {
        auto temp = find_array_symbol_for_func(child);
        if (temp != nullptr && temp->is_array())
            ident = temp;
    }
    // return find_array_symbol(node->get_child(0));
    return ident;
}

Symbol* ConstantArrayVisitSubstitutionAST::find_array_symbol_for_visit(ASTNode* node)
{
    Symbol* ident = nullptr;
    if (node->get_type() == ASTType::Ident)
        return node->get_symbol();
    return find_array_symbol_for_visit(node->get_child(0));}

void ConstantArrayVisitSubstitutionAST::substitution(ASTNode* node)
{
    for (int i = 0; i < node->get_child().size(); ++i) {
        auto node_child = node->get_child(i);
        if (node_child->get_type() == ASTType::ImplicitCast) {
            auto child = node_child->get_child(0);
            if (child->get_type() == ASTType::ArrayVisit) {
                Symbol* arr = find_array_symbol_for_visit(child->get_child(0));
                // 形参
                if (arr->get_val_type().length(0) == 0)
                    continue;
                if (array_def.find(arr) == array_def.end()) {
                    // 计算偏移
                    auto res = get_offset(child);
                    auto length = get<1>(res);
                    int offset = get<2>(res);
                    if (length != 1 || offset == -1)
                        continue;
                    // 直接替换成对应的值
                    node->set_child(i, search_init_val(m_ast, arr, offset));
                    ASTNode::destory(node_child);
                }
            }
        }
        substitution(node->get_child(i));
    }
}

ASTNode* ConstantArrayVisitSubstitutionAST::search_init_val(ASTNode* node, Symbol* arr, int offset)
{
    if (node->get_type() == ASTType::VarDecl && node->get_symbol() == arr) {
        // 未初始化
        if (node->get_child().size() == 1) {
            ValueType type = arr->get_val_type();
            BasicValue value = BasicValue::create_int(0);
            ASTNode* const_val = ASTNode::create_const_value(type, value);
            return const_val;
        } else {
            auto init_val = node->get_child(1);
            int offset_start = 0;
            auto val = get_init_val(init_val, 1, arr->get_val_type().get_dimension(), offset_start, offset, arr);
            return val;
        }
    }

    ASTNode* init_val = nullptr;
    for (int i = 0; i < node->get_child().size(); ++i) {
        if (search_init_val(node->get_child(i), arr, offset) != nullptr)
            init_val = search_init_val(node->get_child(i), arr, offset);
    }
    return init_val;
}

ASTNode* ConstantArrayVisitSubstitutionAST::get_init_val(ASTNode* node, int cur_dim, const std::vector<std::size_t>& length, int& offset, int target_offset, Symbol* arr)
{
    int value_count = 0;
    int unit_count = 0;
    int unit_size = 1;
    for (int d = length.size()-1; d >= cur_dim; d--)
        unit_size *= length[d];
    int unit_length = length[cur_dim - 1];
    for (auto& child : node->get_child()) {
        if (child->get_type() == ASTType::ArrayInitVal) {
            if (value_count != 0) {
                for (; value_count < unit_size; value_count++) {
                    if (offset == target_offset) {
                        ValueType type = arr->get_val_type();
                        BasicValue value = BasicValue::create_int(0);
                        ASTNode* const_val = ASTNode::create_const_value(type, value);
                        return const_val;
                    }
                    ++offset;
                }
                value_count = 0;
                unit_count++;
                if (unit_count == unit_length)
                    break;
            }
            auto init_val = get_init_val(child, cur_dim + 1, length, offset, target_offset, arr);
            if (init_val != nullptr)
                return init_val;
            unit_count++;
        } else {
            if (offset == target_offset)
                return child;
            ++offset;
            value_count++;
            if (value_count == unit_size) {
                value_count = 0;
                unit_count++;
            }
        }
        if (unit_count == unit_length)
            break;
    }
    while (unit_count < unit_length) {
        if (offset == target_offset) {
            ValueType type = arr->get_val_type();
            BasicValue value = BasicValue::create_int(0);
            ASTNode* const_val = ASTNode::create_const_value(type, value);
            return const_val;
        }
        ++offset;
        value_count++;
        if (value_count == unit_size) {
            value_count = 0;
            unit_count++;
        }
    }
    return nullptr;
}
