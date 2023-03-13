#include <cassert>
#include <linear_ir_manager.h>
#include <limits.h>
#include <cfg_manager.h>
//将rhs的元素清空，全部加入lhs中
int LinearIRManager::s_var_num; //变量序号
int LinearIRManager::s_lbl_num; //标号序号
IRSymbolTable* LinearIRManager::s_ir_sym_table;
SymbolTable* LinearIRManager::s_ast_sym_table;
void LinearIRManager::bind_ir_symbol_table(IRSymbolTable* ir_symbol_table)
{
    s_ir_sym_table = ir_symbol_table;
}
void LinearIRManager::bind_symbol_table(SymbolTable* ast_symbol_table)
{
    s_ast_sym_table = ast_symbol_table;
}
void LinearIRManager::splice_ir_instr_list(IRInstrList* lhs, IRInstrList* rhs)
{
    lhs->splice(lhs->end(), *rhs);
    delete rhs;
}
IRInstrList* LinearIRManager::gen_comp_unit(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    //先生成所有全局变量声明，并为函数创建符号
    for (ASTNode *child : node->get_child()) {
        if (child->get_type() == ASTType::DeclStmt) {
            splice_ir_instr_list(res, gen_decl_stmt(child, true));
        } else {
            auto ir_sym=s_ir_sym_table->create_global(child->get_symbol());
            child->get_symbol()->set_ir_sym(ir_sym);
        }
    }
    //然后生成所有函数声明
    for (ASTNode *child : node->get_child()) {
        if (child->get_type() == ASTType::FuncDef) {
            s_var_num = 0; //每个函数的临时变量都单独进行编号
            s_lbl_num = 0;
            splice_ir_instr_list(res, gen_func_def(child));
        }
    }
    return res;
}
IRInstrList* LinearIRManager::gen_decl_stmt(ASTNode *node, bool gen_var_decl_)
{
    IRInstrList* res = new IRInstrList;
    //将子节点所有IR语句合并到一起
    for (ASTNode *child : node->get_child()) {
        splice_ir_instr_list(res, gen_ir(child, -1, -1, gen_var_decl_));
    }
    return res;
}
IRInstrList* LinearIRManager::gen_array_init_enum(ASTNode* node,int cur_dim,const std::vector<std::size_t>& length, IRSymbol* array_base,int& offset,bool gen_zero)
{
    IRInstrList* res = new IRInstrList;
    int value_count = 0;
    int unit_count = 0;
    int unit_size = 1;
    for (int d = length.size()-1; d >= cur_dim; d--)
        unit_size *= length[d];
    int unit_length = length[cur_dim - 1];
    for (auto& child : node->get_child()) {
        if (child->get_type() == ASTType::ArrayInitVal) {
            if (value_count != 0) {
                if (gen_zero) {
                    for (; value_count < unit_size; value_count++) {
                        res->push_back(IRInstr::create_array_store(
                            array_base,
                            s_ir_sym_table->create_value(BasicType::Int, offset++),
                            s_ir_sym_table->create_value(array_base->basic_type(), 0)));
                    }
                } else offset += unit_size - value_count;
                value_count = 0;
                unit_count++;
                if (unit_count == unit_length)
                    break;
            }
            splice_ir_instr_list(res, gen_array_init_enum(child, cur_dim + 1, length, array_base,offset,gen_zero));
            unit_count++;
        } else {
            auto [res_child_list,res_child_oprand] = gen_rvalue_expand(child);
            splice_ir_instr_list(res, res_child_list);
            res->push_back(IRInstr::create_array_store(
                array_base,
                s_ir_sym_table->create_value(BasicType::Int, offset++),
                res_child_oprand));
            value_count++;
            if (value_count == unit_size) {
                value_count = 0;
                unit_count++;
            }
        }
        if (unit_count == unit_length)
            break;
        //先假定不允许出现初始化器超过数组长度的情况
        // assert(!(unit_count > unit_length || unit_count == unit_length && value_count != 0));
    }
    //补全unit个数
    if (gen_zero) {
        while (unit_count < unit_length) {
            res->push_back(IRInstr::create_array_store(
                array_base,
                s_ir_sym_table->create_value(BasicType::Int, offset++),
                s_ir_sym_table->create_value(array_base->basic_type(), 0)));
            value_count++;
            if (value_count == unit_size) {
                value_count = 0;
                unit_count++;
            }
        }
    } else {
        //先补全一个unit
        offset += unit_size - value_count;
        unit_count++;
        //再补全所有的unit
        offset += (unit_length - unit_count) * unit_size;
    }
    return res;
}
std::pair<int, int> LinearIRManager::get_array_init_info(ASTNode* node, int cur_dim, const std::vector<std::size_t>& length, int& offset)
{
    int min_omitted_offset = INT_MAX;
    int initialized_element_count = 0;
    assert(node->get_type() == ASTType::ArrayInitVal);
    int value_count = 0;
    int unit_count = 0;
    int unit_size = 1;
    for (int d = length.size() - 1; d >= cur_dim; d--)
        unit_size *= length[d];
    int unit_length = length[cur_dim - 1];
    for (auto& child : node->get_child()) {
        if (child->get_type() == ASTType::ArrayInitVal) {
            if (value_count != 0) {
                min_omitted_offset = std::min(min_omitted_offset, offset);
                offset += unit_size - value_count;
                value_count = 0;
                unit_count++;
                if (unit_count == unit_length)
                    break;
            }
            auto [sub_count,sub_offset] = get_array_init_info(child, cur_dim + 1, length, offset);
            min_omitted_offset = std::min(min_omitted_offset, sub_offset);
            initialized_element_count += sub_count;
            unit_count++;
        } else {
            value_count++;
            offset++;
            if (value_count == unit_size) {
                value_count = 0;
                unit_count++;
            }
        }
        if (unit_count == unit_length)
            break;
    }
    min_omitted_offset = std::min(min_omitted_offset, offset);
    //先补全一个unit
    offset += unit_size - value_count;
    unit_count++;
    //再补全所有的unit
    offset += (unit_length - unit_count) * unit_size;
    return { initialized_element_count, min_omitted_offset };
}
IRInstrList* LinearIRManager::gen_array_init(ASTNode* init_node, Symbol* array_sym)
{
    IRInstrList* res = new IRInstrList;
    int offset = 0;
    auto [initialized_element_count, min_omitted_offset] = get_array_init_info(init_node, 1, array_sym->get_val_type().get_dimension(), offset);
    auto total_length = array_sym->get_val_type().total_length();
    assert(total_length < 536870912);
    offset = 0;
    //总长度小于16，则枚举式初始化
    if (total_length < 16) {
        splice_ir_instr_list(res,gen_array_init_enum(init_node, 1, array_sym->get_val_type().get_dimension(), array_sym->get_ir_sym(), offset, true));
    } else if (initialized_element_count >= total_length/2) {//如果初始化的元素数达到了一半，则枚举式初始化
        splice_ir_instr_list(res, gen_array_init_enum(init_node, 1, array_sym->get_val_type().get_dimension(), array_sym->get_ir_sym(), offset, true));
    } else {  //否则先全部memset为0再枚举式初始化
        Symbol* memset_func=s_ast_sym_table->get_current_sym("memset");
        IRSymbol* ir_memset_func= memset_func->get_ir_sym();
        if (ir_memset_func == nullptr) {
            ir_memset_func = s_ir_sym_table->create_global(memset_func);
            memset_func->set_ir_sym(ir_memset_func);
        }else ir_memset_func = memset_func->get_ir_sym();
        IRSymbol* memset_start_addr;
        res->push_back(IRInstr::create_binary_calc(
            IROper::AddI,
            memset_start_addr=s_ir_sym_table->create_temp(array_sym->get_val_type().basic(),s_var_num++, IRArrayLength::IR_ARRAY_POINTER),
            array_sym->get_ir_sym(),
            s_ir_sym_table->create_value(BasicType::Int,min_omitted_offset)
        ));
        res->push_back(IRInstr::create_r_param(memset_start_addr));
        res->push_back(IRInstr::create_r_param(s_ir_sym_table->create_value(BasicType::Int, 0)));
        res->push_back(IRInstr::create_r_param(s_ir_sym_table->create_value(BasicType::Int, ((int)total_length - min_omitted_offset) * 4)));
        res->push_back(IRInstr::create_call(ir_memset_func, s_ir_sym_table->create_value(BasicType::Int, 3)));
        splice_ir_instr_list(res, gen_array_init_enum(init_node, 1, array_sym->get_val_type().get_dimension(), array_sym->get_ir_sym(), offset, false));
    }
    return res;
}
//生成局部变量初始化语句
IRInstrList* LinearIRManager::gen_local_var_init(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    assert(node->get_type() == ASTType::ConstDecl || node->get_type() == ASTType::VarDecl);
    Symbol *sym = node->get_symbol();
    //判断变量是否被初始化了
    if (node->get_child().size() > (sym->is_array() ? 1 : 0)) {
        if (sym->is_array()) {
            splice_ir_instr_list(res, gen_array_init(node->get_child(1),sym));
        } else {
            auto process_right_child = gen_rvalue_expand(node->get_child(0));
            splice_ir_instr_list(res, process_right_child.first);
            res->push_back(IRInstr::create_assign(sym->get_ir_sym(), process_right_child.second));
        }
    }
    return res;
}
IRInstrList* LinearIRManager::gen_var_decl(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    Symbol *ast_sym = node->get_symbol();
    switch (ast_sym->get_kind()) {
    case VarKind::Global: {
        IRSymbol* ir_sym = s_ir_sym_table->create_global(ast_sym);
        node->get_symbol()->set_ir_sym(ir_sym); //符号表中对应项指向IR符号表
        res->push_back(IRInstr::create_global_decl(ir_sym));
        break;
    }
    case VarKind::Local: {
        int array_length = (ast_sym->is_array()) ? (ast_sym->get_val_type().total_length()) : -1;
        IRSymbol* ir_sym = s_ir_sym_table->create_local(ast_sym->get_val_type().basic(), s_var_num++, array_length);
        node->get_symbol()->set_ir_sym(ir_sym); //符号表中对应项指向IR符号表
        res->push_back(IRInstr::create_local_decl(ir_sym));
        break;
    }
    default:
        assert(false);
        break;
    }
    return res;
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_const_value(ASTNode *node)
{
    IRInstrList* res_list = new IRInstrList;
    IRSymbol* res_oprand = nullptr;
    res_list->push_back(IRInstr::create_assign(
        res_oprand = s_ir_sym_table->create_temp(node->get_value_type().basic(), s_var_num++),
        s_ir_sym_table->create_value(node->get_value_type().basic(),node->get_value())
    ));
    return { res_list,res_oprand };
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_ident(ASTNode *node)
{
    IRInstrList* res_list = new IRInstrList;
    Symbol *sym = node->get_symbol();
    assert(sym != nullptr);
    IRSymbol* res_oprand = s_ir_sym_table->create_temp(sym->get_ir_sym()->basic_type(), s_var_num++);
    return { res_list,res_oprand };
}
std::tuple<IRInstrList*, std::vector<size_t>, size_t, Symbol*,IRSymbol*> LinearIRManager::gen_array_visit(ASTNode *node)
{
    IRInstrList* res_list = new IRInstrList;
    std::vector<size_t> dim_size = {};
    Symbol* array_ident_sym = nullptr;
    IRSymbol* res_oprand = nullptr;
    size_t length = 1;
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
        auto res_lchild = gen_array_visit(node->get_child(0));
        auto res_rchild = gen_rvalue_expand(node->get_child(1));
        //从左子节点获得已经计算完成的地址
        splice_ir_instr_list(res_list, std::get<0>(res_lchild));
        dim_size = std::get<1>(res_lchild);
        length = std::get<2>(res_lchild);
        array_ident_sym = std::get<3>(res_lchild);
        auto oprand_lchild = std::get<4>(res_lchild); //左子节点已经计算出的表达式对应的符号
        assert(!dim_size.empty());
        if (dim_size.back() != 0) {
            length /= dim_size.back();
        }
        dim_size.pop_back();
        //从右子节点获得访问下标
        splice_ir_instr_list(res_list, res_rchild.first);
        //计算本层新增的偏移量
        auto oprand_offset = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++);
        res_list->push_back(IRInstr::create_binary_calc(
            IROper::MulI,
            oprand_offset,
            s_ir_sym_table->create_value(BasicType::Int, (int)length),
            res_rchild.second
        ));
        //如果左子节点不是叶子，加上左子节点已经计算出的部分
        if (node->get_child(0)->get_type() != ASTType::Ident) {
            res_list->push_back(IRInstr::create_binary_calc(
                IROper::AddI,
                res_oprand = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++),
                oprand_offset,
                oprand_lchild
            ));
        } else  res_oprand = oprand_offset;
    }
    return std::make_tuple(res_list, dim_size, length, array_ident_sym, res_oprand);
}
//将条件转为IR，包括短路求值
IRInstrList* LinearIRManager::gen_condition(ASTNode *node, int true_lbl, int false_lbl)
{
    IRInstrList* res = new IRInstrList;
    if (node->get_type() == ASTType::BinaryOp && node->get_func() == (int)BinaryOpFunc::And) {
        int mid_lbl = s_lbl_num++; //子条件A和B中间的label
        splice_ir_instr_list(res, gen_condition(node->get_child(0), mid_lbl, false_lbl));
        res->push_back(IRInstr::create_label(mid_lbl));
        splice_ir_instr_list(res, gen_condition(node->get_child(1), true_lbl, false_lbl));
    }
    else if (node->get_type() == ASTType::BinaryOp && node->get_func() == (int)BinaryOpFunc::Or) {
        int mid_lbl = s_lbl_num++; //子条件A和B中间的label
        splice_ir_instr_list(res, gen_condition(node->get_child(0), true_lbl, mid_lbl));
        res->push_back(IRInstr::create_label(mid_lbl));
        splice_ir_instr_list(res, gen_condition(node->get_child(1), true_lbl, false_lbl));
    } else if (node->get_type() == ASTType::UnaryOp && node->get_func() == (int)UnaryOpFunc::Not) {
        splice_ir_instr_list(res, gen_condition(node->get_child(0), false_lbl, true_lbl));
    } else {
        auto res_child = gen_rvalue_noexpand(node);
        splice_ir_instr_list(res, res_child.first);
        IRSymbol* ir_sym = res_child.second;
        //float类型直接出现在条件里，则加一个与0的比较指令
        if (ir_sym->basic_type()==BasicType::Float) {
            IRSymbol* cast_res;
            res->push_back(IRInstr::create_binary_calc(
                IROper::NotEqualF,
                cast_res = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++),
                ir_sym,
                s_ir_sym_table->create_value(BasicType::Float,(float)0.0)
            ));
            ir_sym = cast_res;
        }
        res->push_back(IRInstr::create_cond_goto(ir_sym, true_lbl, false_lbl));
    }
    return res;
}
IRInstrList* LinearIRManager::gen_switch_stmt(ASTNode* node, int continue_lbl, int break_lbl)
{
    IRInstrList* res = new IRInstrList;
    std::vector<CaseLabel> case_vec;
    for (auto child : node->get_child()){
        assert(child->get_value_type().basic() != BasicType::Float);
        case_vec.push_back(CaseLabel(s_lbl_num++, child->get_value(), child->get_type() == ASTType::Default));
    }
    //只有true stmt
    if (node->get_child().size() == 2) {
        int lbl_true = s_lbl_num++;
        int lbl_end = s_lbl_num++;
        splice_ir_instr_list(res, gen_condition(node->get_child(0), lbl_true, lbl_end));
        res->push_back(IRInstr::create_label(lbl_true));
        splice_ir_instr_list(res, gen_ir(node->get_child(1), continue_lbl, break_lbl));
        res->push_back(IRInstr::create_label(lbl_end));
    }
    else { // true和false stmt都有
        int lbl_true = s_lbl_num++;
        int lbl_false = s_lbl_num++;
        int lbl_end = s_lbl_num++;
        splice_ir_instr_list(res, gen_condition(node->get_child(0), lbl_true, lbl_false));
        res->push_back(IRInstr::create_label(lbl_true));
        splice_ir_instr_list(res, gen_ir(node->get_child(1), continue_lbl, break_lbl));
        res->push_back(IRInstr::create_goto(lbl_end));
        res->push_back(IRInstr::create_label(lbl_false));
        splice_ir_instr_list(res, gen_ir(node->get_child(2), continue_lbl, break_lbl));
        res->push_back(IRInstr::create_label(lbl_end));
    }
    return res;
}
IRInstrList* LinearIRManager::gen_if_stmt(ASTNode *node, int continue_lbl, int break_lbl)
{
    IRInstrList* res = new IRInstrList;
    //只有true stmt
    if (node->get_child().size() == 2) {
        int lbl_true = s_lbl_num++;
        int lbl_end = s_lbl_num++;
        splice_ir_instr_list(res, gen_condition(node->get_child(0), lbl_true, lbl_end));
        res->push_back(IRInstr::create_label(lbl_true));
        splice_ir_instr_list(res, gen_ir(node->get_child(1), continue_lbl, break_lbl));
        res->push_back(IRInstr::create_label(lbl_end));
    } else { // true和false stmt都有
        int lbl_true = s_lbl_num++;
        int lbl_false = s_lbl_num++;
        int lbl_end = s_lbl_num++;
        splice_ir_instr_list(res, gen_condition(node->get_child(0), lbl_true, lbl_false));
        res->push_back(IRInstr::create_label(lbl_true));
        splice_ir_instr_list(res, gen_ir(node->get_child(1), continue_lbl, break_lbl));
        res->push_back(IRInstr::create_goto(lbl_end));
        res->push_back(IRInstr::create_label(lbl_false));
        splice_ir_instr_list(res, gen_ir(node->get_child(2), continue_lbl, break_lbl));
        res->push_back(IRInstr::create_label(lbl_end));
    }
    return res;
}
IRInstrList* LinearIRManager::gen_while_stmt(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    int lbl_cond = s_lbl_num++;
    int lbl_stmt = s_lbl_num++;
    int lbl_end = s_lbl_num++;
    res->push_back(IRInstr::create_label(lbl_cond));
    splice_ir_instr_list(res, gen_condition(node->get_child(0), lbl_stmt, lbl_end));
    res->push_back(IRInstr::create_label(lbl_stmt));
    splice_ir_instr_list(res, gen_ir(node->get_child(1), lbl_cond, lbl_end));
    res->push_back(IRInstr::create_goto(lbl_cond));
    res->push_back(IRInstr::create_label(lbl_end));
    return res;
}
IRInstrList* LinearIRManager::gen_do_while_stmt(ASTNode* node)
{
    IRInstrList* res = new IRInstrList;
    int lbl_cond = s_lbl_num++;
    int lbl_stmt = s_lbl_num++;
    int lbl_end = s_lbl_num++;
    res->push_back(IRInstr::create_label(lbl_stmt));
    splice_ir_instr_list(res, gen_ir(node->get_child(1), lbl_cond, lbl_end));
    res->push_back(IRInstr::create_label(lbl_cond));
    splice_ir_instr_list(res, gen_condition(node->get_child(0), lbl_stmt, lbl_end));
    res->push_back(IRInstr::create_label(lbl_end));
    return res;
}
IRInstrList* LinearIRManager::gen_return_stmt(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    //无返回值
    if (node->get_child().empty()) {
        res->push_back(IRInstr::create_return());
    } else { //有返回值
        auto res_child=gen_rvalue_noexpand(node->get_child(0));
        splice_ir_instr_list(res, res_child.first);
        res->push_back(IRInstr::create_value_return(res_child.second));
    }
    return res;
}
IRInstrList* LinearIRManager::gen_break_stmt(ASTNode *node, int break_lbl)
{
    IRInstrList* res = new IRInstrList;
    res->push_back(IRInstr::create_goto(break_lbl));
    return res;
}
IRInstrList* LinearIRManager::gen_continue_stmt(ASTNode *node, int continue_lbl)
{
    IRInstrList* res = new IRInstrList;
    res->push_back(IRInstr::create_goto(continue_lbl));
    return res;
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_unary_op(ASTNode *node)
{
    IRInstrList* res_list = new IRInstrList;
    IRSymbol* res_oprand = nullptr;
    IRSymbol* oprand_child;
    auto process_child = gen_rvalue_expand(node->get_child(0));
    splice_ir_instr_list(res_list, process_child.first);
    oprand_child = process_child.second;
    switch((UnaryOpFunc)node->get_func()){
    case UnaryOpFunc::Negative:
        res_list->push_back(IRInstr::create_unary_calc(
            (oprand_child->basic_type()==BasicType::Int ? IROper::NegI: IROper::NegF),
            res_oprand = s_ir_sym_table->create_temp(oprand_child->basic_type(), s_var_num++),
            oprand_child
        ));
        break;
    case UnaryOpFunc::Not:
        //float类型直接出现在条件里，则加一个与0的比较指令
        if (oprand_child->basic_type() == BasicType::Float) {
            IRSymbol* cast_res;
            res_list->push_back(IRInstr::create_binary_calc(
                IROper::NotEqualF,
                cast_res = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++),
                oprand_child,
                s_ir_sym_table->create_value(BasicType::Float, (float)0.0)
            ));
            oprand_child = cast_res;
        }
        res_list->push_back(IRInstr::create_unary_calc(
            IROper::NotI,
            res_oprand = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++),
            oprand_child
        ));
        break;
    //什么也不做
    case UnaryOpFunc::Paren:
    case UnaryOpFunc::Positive:
        res_list->push_back(IRInstr::create_assign(
            res_oprand = s_ir_sym_table->create_temp(oprand_child->basic_type(), s_var_num++),
            oprand_child
        ));
        break;
    default:
        assert(false);
    }
    return {res_list,res_oprand};
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_rvalue_expand_array_visit(ASTNode* node, IRSymbol* res_oprand)
{
    IRInstrList* res_list = new IRInstrList;
    ASTNode* target_node = node->get_child(0);
    //从子节点获得已经计算完成的地址以及地址计算语句
    const auto& res_child = gen_array_visit(target_node);
    splice_ir_instr_list(res_list, std::get<0>(res_child));
    Symbol* sym = std::get<3>(res_child);
    IRSymbol* oprand_offset = std::get<4>(res_child);
    assert(oprand_offset != nullptr);
    //判断返回结果是不是数组
    if (node->get_value_type().is_array()) {
        //如果返回结果也是数组，只计算偏移量
        res_list->push_back(IRInstr::create_binary_calc(
            IROper::AddI,   //偏移量是整数相加
            res_oprand = s_ir_sym_table->create_temp(node->get_value_type().basic(), s_var_num++, 0),
            sym->get_ir_sym(),
            oprand_offset
        ));
    } else {
        //如果返回不是数组，则直接load
        res_list->push_back(IRInstr::create_array_load(
            res_oprand = s_ir_sym_table->create_temp(node->get_value_type().basic(), s_var_num++),
            sym->get_ir_sym(),
            oprand_offset
        ));
    }
    return { res_list,res_oprand };
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_rvalue_expand(ASTNode *node)
{
    IRSymbol* res_oprand = nullptr;
    if (node->get_type() == ASTType::ConstValue) {
        res_oprand = s_ir_sym_table->create_value(node->get_value_type().basic(), node->get_value());
    } else if (node->get_type() == ASTType::ImplicitCast && node->get_child(0)->get_type() == ASTType::Ident) {
        res_oprand = node->get_child(0)->get_symbol()->get_ir_sym();
        assert(res_oprand != nullptr);
    } else if (node->get_type() == ASTType::ImplicitCast && node->get_child(0)->get_type() == ASTType::ArrayVisit) { //访问数组
        return gen_rvalue_expand_array_visit(node,res_oprand);
    } else {
        switch (node->get_type()) {
        case ASTType::ConstValue:
            return gen_const_value(node);
        case ASTType::BinaryOp:
            return gen_binary_op(node);
        case ASTType::UnaryOp:
            return gen_unary_op(node);
        case ASTType::FuncCall:
            return gen_func_call(node,true);
        case ASTType::ImplicitCast:
            return gen_implicit_cast(node);
        case ASTType::ArraySize:
            return { new IRInstrList,nullptr };
        default:
            break;
        }
        assert(false);
    }
    return { new IRInstrList,res_oprand };
}
std::tuple<IRInstrList*, IRSymbol*,IRSymbol*> LinearIRManager::gen_lvalue(ASTNode *node)
{
    assert(node->is_lvalue());
    IRInstrList* res_list = new IRInstrList;
    IRSymbol* res_oprand = nullptr;
    IRSymbol* res_oprand_offset = nullptr;
    if (node->get_type() == ASTType::Ident) {
        Symbol *sym = node->get_symbol();
        res_oprand = sym->get_ir_sym();
    } else if (node->get_type() == ASTType::ArrayVisit) {
        //从子节点获得已经计算完成的基地址和偏移量
        const auto& res_child = gen_array_visit(node);
        splice_ir_instr_list(res_list, std::get<0>(res_child));
        Symbol* sym = std::get<3>(res_child);
        res_oprand = sym->get_ir_sym();
        res_oprand_offset = std::get<4>(res_child);
    } else {
        assert(false);
    }
    assert(res_oprand != nullptr);
    return std::make_tuple(res_list, res_oprand,res_oprand_offset);
}
IRSymbol* LinearIRManager::gen_binary_op_sub(ASTNode* node, IRInstrList* res_list, IRSymbol* oprand_lchild, IRSymbol* oprand_rchild)
{
    IRSymbol* res_oprand = nullptr;
    IROper operation = IROper::Null;
    bool operation_is_compare = false;
    switch ((BinaryOpFunc)node->get_func()) {
    case BinaryOpFunc::Assign: {
        assert(node->get_child(0)->is_lvalue());
        //左子节点是左值，右子节点是右值
        const auto& res_rchild = gen_rvalue_expand(node->get_child(1));
        const auto& res_lchild = gen_lvalue(node->get_child(0));
        splice_ir_instr_list(res_list, res_rchild.first);
        splice_ir_instr_list(res_list, std::get<0>(res_lchild));
        oprand_lchild = std::get<1>(res_lchild);
        oprand_rchild = res_rchild.second;
        if (node->get_child(0)->get_type() == ASTType::ArrayVisit) { //左值是数组元素
            auto oprand_offset = std::get<2>(res_lchild);
            res_list->push_back(IRInstr::create_array_store(oprand_lchild, oprand_offset, oprand_rchild));
        }
        else //左值是变量
            res_list->push_back(IRInstr::create_assign(oprand_lchild, oprand_rchild));
        break;
    }
    case BinaryOpFunc::Add: {
        auto lchild = node->get_child(0);
        auto rchild = node->get_child(1);
        //对数组+偏移量的情形进行特殊判定
        if (node->get_value_type().is_array()) {
            assert(!rchild->get_value_type().is_array());
            IRSymbol* last_oprand;
            res_list->push_back(IRInstr::create_binary_calc(
                IROper::MulI,
                last_oprand = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++),
                oprand_rchild,
                s_ir_sym_table->create_value(BasicType::Int, (int)node->get_value_type().top_unit_length())
            ));
            oprand_rchild = last_oprand;
            operation = IROper::AddI;
        }else operation = node->get_value_type().basic() == BasicType::Int ? IROper::AddI: IROper::AddF;
        break;
    }
    case BinaryOpFunc::Sub:
        operation = node->get_value_type().basic() == BasicType::Int ? IROper::SubI : IROper::SubF;
        break;
    case BinaryOpFunc::Mul:
        operation = node->get_value_type().basic() == BasicType::Int ? IROper::MulI : IROper::MulF;
        break;
    case BinaryOpFunc::Div:
        operation = node->get_value_type().basic() == BasicType::Int ? IROper::DivI : IROper::DivF;
        break;
    case BinaryOpFunc::Mod:
        operation = IROper::ModI;
        break;
    case BinaryOpFunc::Equal:
        assert(node->get_child(0)->get_value_type().basic() == node->get_child(1)->get_value_type().basic());
        operation = node->get_child(0)->get_value_type().basic() == BasicType::Int ? IROper::EqualI : IROper::EqualF;
        operation_is_compare = true;
        break;
    case BinaryOpFunc::NotEqual:
        assert(node->get_child(0)->get_value_type().basic() == node->get_child(1)->get_value_type().basic());
        operation = node->get_child(0)->get_value_type().basic() == BasicType::Int ? IROper::NotEqualI : IROper::NotEqualF;
        operation_is_compare = true;
        break;
    case BinaryOpFunc::Great:
        assert(node->get_child(0)->get_value_type().basic() == node->get_child(1)->get_value_type().basic());
        operation = node->get_child(0)->get_value_type().basic() == BasicType::Int ? IROper::GreaterI : IROper::GreaterF;
        operation_is_compare = true;
        break;
    case BinaryOpFunc::Less:
        assert(node->get_child(0)->get_value_type().basic() == node->get_child(1)->get_value_type().basic());
        operation = node->get_child(0)->get_value_type().basic() == BasicType::Int ? IROper::LessI : IROper::LessF;
        operation_is_compare = true;
        break;
    case BinaryOpFunc::GreatEqual:
        assert(node->get_child(0)->get_value_type().basic() == node->get_child(1)->get_value_type().basic());
        operation = node->get_child(0)->get_value_type().basic() == BasicType::Int ? IROper::GreaterEqualI : IROper::GreaterEqualF;
        operation_is_compare = true;
        break;
    case BinaryOpFunc::LessEqual:
        assert(node->get_child(0)->get_value_type().basic() == node->get_child(1)->get_value_type().basic());
        operation = node->get_child(0)->get_value_type().basic() == BasicType::Int ? IROper::LessEqualI : IROper::LessEqualF;
        operation_is_compare = true;
        break;
    default:
        assert(false);
        break;
    }
    if (operation != IROper::Null) {
        res_list->push_back(IRInstr::create_binary_calc(
            operation,
            res_oprand = s_ir_sym_table->create_temp(operation_is_compare ? BasicType::Int : node->get_value_type().basic(), s_var_num++, node->get_value_type().is_array()?IRArrayLength::IR_ARRAY_POINTER:IRArrayLength::IR_NONE_ARRAY),
            oprand_lchild,
            oprand_rchild
        ));
    }
    return res_oprand;
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_binary_op(ASTNode *node)
{
    IRInstrList* res_list = new IRInstrList;
    IRSymbol* oprand_lchild = nullptr, * oprand_rchild = nullptr;               //左右子树对应操作数
    //对于除了And、Or、Assign以外的节点，先处理子节点
    if (node->get_func() != (int)BinaryOpFunc::And &&
        node->get_func() != (int)BinaryOpFunc::Or &&
        node->get_func() != (int)BinaryOpFunc::Assign) {
        const auto& res_lchild = gen_rvalue_expand(node->get_child(0));
        splice_ir_instr_list(res_list, res_lchild.first);
        oprand_lchild = res_lchild.second;
        const auto& res_rchild = gen_rvalue_expand(node->get_child(1));
        splice_ir_instr_list(res_list, res_rchild.first);
        oprand_rchild = res_rchild.second;
    }
    return { res_list,gen_binary_op_sub(node,res_list, oprand_lchild, oprand_rchild) };
}
IRInstrList* LinearIRManager::gen_func_def(ASTNode *node)
{
    assert(node->get_type() == ASTType::FuncDef);
    IRInstrList* res = new IRInstrList;
    //无形参
    if (node->get_child().size() == 1) {
        assert(node->get_symbol()->get_ir_sym() != nullptr);
        res->push_back(IRInstr::create_func_def(
            node->get_symbol()->get_ir_sym(),
            s_ir_sym_table->create_value(BasicType::Int, 0)
        ));
        //生成所有局部变量的声明代码
        splice_ir_instr_list(res, gen_local_var_def(node->get_child(0)));
        //生成语句块内其他代码
        splice_ir_instr_list(res, gen_block(node->get_child(0)));
        res->push_back(IRInstr::create_func_end());
    } else { //有形参
        assert(node->get_symbol()->get_ir_sym() != nullptr);
        res->push_back(IRInstr::create_func_def(
            node->get_symbol()->get_ir_sym(),
            s_ir_sym_table->create_value(BasicType::Int, (int)node->get_child(0)->get_child().size())));
        splice_ir_instr_list(res, gen_func_f_params(node->get_child(0)));
        //生成所有局部变量的声明代码
        splice_ir_instr_list(res, gen_local_var_def(node->get_child(1)));
        //生成语句块内其他代码
        splice_ir_instr_list(res, gen_block(node->get_child(1)));
        res->push_back(IRInstr::create_func_end());
    }
    return res;
}
//生成子树中所有局部变量的定义语句,忽略其他节点
IRInstrList* LinearIRManager::gen_local_var_def(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    switch (node->get_type()) {
    case ASTType::DeclStmt:
        delete res;
        res = gen_decl_stmt(node, true);
        break;
    case ASTType::IfStmt:
        splice_ir_instr_list(res, gen_local_var_def(node->get_child(1)));
        if (node->get_child().size() > 2)
            splice_ir_instr_list(res, gen_local_var_def(node->get_child(2)));
        break;
    case ASTType::WhileStmt:
        splice_ir_instr_list(res, gen_local_var_def(node->get_child(1)));
        break;
    case ASTType::DoWhileStmt:
        splice_ir_instr_list(res, gen_local_var_def(node->get_child(1)));
        break;
    case ASTType::Block:
        for (ASTNode *child : node->get_child()) {
            if (child->get_type() == ASTType::DeclStmt ||
                child->get_type() == ASTType::Block ||
                child->get_type() == ASTType::IfStmt ||
                child->get_type() == ASTType::WhileStmt||
                child->get_type() == ASTType::DoWhileStmt) {
                splice_ir_instr_list(res, gen_local_var_def(child));
            }
        }
        break;
    case ASTType::FuncDef:
    case ASTType::ConstValue:
    case ASTType::Ident:
    case ASTType::BinaryOp:
    case ASTType::UnaryOp:
    case ASTType::BreakStmt:
    case ASTType::ContinueStmt:
    case ASTType::ReturnStmt:
    case ASTType::NullStmt:
    case ASTType::FuncCall:
    case ASTType::FuncRParams:
    case ASTType::FuncFParams:
    case ASTType::FuncFParam:
    case ASTType::ImplicitCast:
    case ASTType::ArraySize:
    case ASTType::ArrayInitVal:
    case ASTType::ArrayVisit:
        break;
    case ASTType::VarDecl:
    case ASTType::ConstDecl:
    default:
        assert(false);
        break;
    }
    return res;
}
//注：该函数会忽略所有的局部变量声明，只进行初值赋予
IRInstrList* LinearIRManager::gen_block(ASTNode *node, int continue_lbl, int break_lbl)
{
#ifdef DEBUG_IR
    printf("debug_ir: generating \"block\"\n");
#endif
    IRInstrList* res = new IRInstrList;
    //将子节点所有IR语句合并到一起
    for (ASTNode *child : node->get_child()) {
        splice_ir_instr_list(res, gen_ir(child, continue_lbl, break_lbl));
    }
    return res;
}

IRInstrList* LinearIRManager::gen_func_f_params(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    //将子节点所有IR语句合并到一起
    for (ASTNode *child : node->get_child()) {
        splice_ir_instr_list(res, gen_func_f_param(child));
    }
    return res;
}
IRInstrList* LinearIRManager::gen_func_f_param(ASTNode *node)
{
    assert(node->get_type() == ASTType::FuncFParam);
    IRInstrList* res = new IRInstrList;
    Symbol *symbol = node->get_symbol();
    assert(symbol->get_kind() == VarKind::Param);
    IRSymbol* ir_sym = s_ir_sym_table->create_param(symbol->get_val_type().basic(),s_var_num++, symbol->is_array() ? 0 : -1);
    symbol->set_ir_sym(ir_sym); //在符号表中记录下ir_sym
    res->push_back(IRInstr::create_f_param(ir_sym));
    return res;
}

std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_implicit_cast(ASTNode *node)
{
    IRInstrList* res_list = new IRInstrList;
    IRSymbol* res_oprand = nullptr;
    //左值转右值
    if (node->get_func() == (int)ImplicitCastFunc::LValueToRValue) {
        //变量直接转右值
        if (node->get_child(0)->get_type() == ASTType::Ident) {
            res_list->push_back(IRInstr::create_assign(
                res_oprand = s_ir_sym_table->create_temp(node->get_value_type().basic(), s_var_num++), node->get_child(0)->get_symbol()->get_ir_sym()));
        } else { //数组访问转右值
            const auto& res_child = gen_lvalue(node->get_child(0));
            splice_ir_instr_list(res_list, std::get<0>(res_child));
            auto ir_sym_base = std::get<1>(res_child);
            auto ir_sym_offset = std::get<2>(res_child);
            //根据数组访问结果是否还是数组来决定添加哪一条指令
            if (node->get_value_type().is_array()) {
                res_list->push_back(IRInstr::create_binary_calc(IROper::AddI,
                    res_oprand = s_ir_sym_table->create_temp(node->get_value_type().basic(), s_var_num++, 0), ir_sym_base, ir_sym_offset));
            } else {
                res_list->push_back(IRInstr::create_array_load(
                    res_oprand = s_ir_sym_table->create_temp(node->get_value_type().basic(), s_var_num++), ir_sym_base, ir_sym_offset));
            }
        }
    } else if (node->get_func() == (int)ImplicitCastFunc::FloatToInt) {
        const auto& res_child = gen_rvalue_expand(node->get_child(0));
        splice_ir_instr_list(res_list, res_child.first);
        res_list->push_back(IRInstr::create_unary_calc(IROper::FToI,
            res_oprand = s_ir_sym_table->create_temp(BasicType::Int, s_var_num++), res_child.second));
    } else if (node->get_func() == (int)ImplicitCastFunc::IntToFloat) {
        const auto& res_child = gen_rvalue_expand(node->get_child(0));
        splice_ir_instr_list(res_list, res_child.first);
        res_list->push_back(IRInstr::create_unary_calc(IROper::IToF,
            res_oprand = s_ir_sym_table->create_temp(BasicType::Float, s_var_num++), res_child.second));
    }
    return { res_list,res_oprand };
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_func_call(ASTNode *node,bool with_return_symbol)
{
    IRInstrList* res_list = new IRInstrList;
    IRSymbol* res_oprand = nullptr;
    int param_count = 0;
    //处理参数
    if (!node->get_child().empty()) {
        param_count = node->get_child(0)->get_child().size();
        splice_ir_instr_list(res_list, gen_func_r_params(node->get_child(0)));
    }
    assert(node->get_symbol() != nullptr);
    IRSymbol* func;
    //本文件中没定义的函数为库函数
    if (node->get_symbol()->get_ir_sym() == nullptr) {
        func = s_ir_sym_table->create_global(node->get_symbol());
        node->get_symbol()->set_ir_sym(func);
    }
    else func = node->get_symbol()->get_ir_sym();
    if (node->get_symbol()->get_val_type().basic() == BasicType::Void || !with_return_symbol) {
        res_list->push_back(IRInstr::create_call(func, s_ir_sym_table->create_value(BasicType::Int, param_count)));
    } else {
        res_list->push_back(IRInstr::create_call_with_ret(
            res_oprand = s_ir_sym_table->create_temp(node->get_symbol()->get_val_type().basic(), s_var_num++),
            func,
            s_ir_sym_table->create_value(BasicType::Int, param_count)));
    }
    return  { res_list,res_oprand };
}
IRInstrList* LinearIRManager::gen_func_r_params(ASTNode *node)
{
    IRInstrList* res = new IRInstrList;
    IRInstrList* rparam_instrs = new IRInstrList;
    //计算实参
    for (ASTNode* child : node->get_child()){
        const auto& res_rparam = gen_rvalue_expand(child);
        splice_ir_instr_list(res,res_rparam.first);
        rparam_instrs->push_back(IRInstr::create_r_param(res_rparam.second));
    }
    splice_ir_instr_list(res, rparam_instrs);
    return res;
}
std::pair<IRInstrList*, IRSymbol*> LinearIRManager::gen_rvalue_noexpand(ASTNode* node)
{
    switch (node->get_type()) {
    case ASTType::ConstValue:
        return gen_const_value(node);
    case ASTType::BinaryOp:
        return gen_binary_op(node);
    case ASTType::UnaryOp:
        return gen_unary_op(node);
    case ASTType::FuncCall:
        return gen_func_call(node,true);
    case ASTType::ImplicitCast:
        return gen_implicit_cast(node);
    case ASTType::ArraySize:
        return { new IRInstrList,nullptr};
    default:
        assert(false);
        break;
    }
    return { new IRInstrList,nullptr };
}
IRInstrList* LinearIRManager::gen_ir(ASTNode *node, int continue_lbl, int break_lbl, bool gen_var_decl_)
{
    switch (node->get_type()) {
    case ASTType::CompUnit:
        return gen_comp_unit(node);
    case ASTType::DeclStmt:
        return gen_decl_stmt(node, gen_var_decl_);
    case ASTType::VarDecl:
    case ASTType::ConstDecl:
        if (gen_var_decl_)
            return gen_var_decl(node);
        else
            return gen_local_var_init(node);
    case ASTType::FuncDef:
        return gen_func_def(node);
    case ASTType::BinaryOp:
        return gen_binary_op(node).first;
    case ASTType::UnaryOp:
        return gen_unary_op(node).first;
    case ASTType::FuncCall:
        return gen_func_call(node,false).first;
    case ASTType::Block:
        return gen_block(node, continue_lbl, break_lbl);
    case ASTType::IfStmt:
        return gen_if_stmt(node, continue_lbl, break_lbl);
    case ASTType::SwitchStmt:
        return gen_switch_stmt(node, continue_lbl, break_lbl);
    case ASTType::WhileStmt:
        return gen_while_stmt(node);
    case ASTType::DoWhileStmt:
        return gen_do_while_stmt(node);
    case ASTType::BreakStmt:
        return gen_break_stmt(node, break_lbl);
    case ASTType::ContinueStmt:
        return gen_continue_stmt(node, continue_lbl);
    case ASTType::ReturnStmt:
        return gen_return_stmt(node);
    case ASTType::NullStmt:
        return new IRInstrList;
    case ASTType::FuncRParams:
        return gen_func_r_params(node);
    case ASTType::FuncFParams:
        return gen_func_f_params(node);
    case ASTType::FuncFParam:
        return gen_func_f_param(node);
    case ASTType::ArraySize:
        return new IRInstrList;
    default:
        //未知的指令
        assert(false);
        break;
    }
    return new IRInstrList;
}
static std::string get_lbl_str(int lbl)
{
    std::stringstream res;
    res << ".lbl " << lbl;
    return res.str();
}
static std::ostream& operator<<(std::ostream& os, IROper op)
{
    switch (op) {
    case IROper::Null:return os << "?";
    case IROper::AddI:return os << "+";
    case IROper::AddF:return os << "+";
    case IROper::SubI:return os << "-";
    case IROper::SubF:return os << "-";
    case IROper::MulI:return os << "*";
    case IROper::MulF:return os << "*";
    case IROper::DivI:return os << "/";
    case IROper::DivF:return os << "/";
    case IROper::ModI:return os << "%";
    case IROper::NegI:return os << "-";
    case IROper::NegF:return os << "-";
    case IROper::IToF:return os << "(i32->f32)";
    case IROper::FToI:return os << "(f32->i32)";
    case IROper::EqualI:return os << "==";
    case IROper::EqualF:return os << "==";
    case IROper::NotEqualI:return os << "!=";
    case IROper::NotEqualF:return os << "!=";
    case IROper::GreaterI:return os << ">";
    case IROper::GreaterF:return os << ">";
    case IROper::LessI:return os << "<";
    case IROper::LessF:return os << "<";
    case IROper::GreaterEqualI:return os << ">=";
    case IROper::GreaterEqualF:return os << ">=";
    case IROper::LessEqualI:return os << "<=";
    case IROper::LessEqualF:return os << "<=";
    case IROper::NotI:return os << "!";
    case IROper::NotF:return os << "!";
    case IROper::BitAnd:return os << "&";
    case IROper::BitOr:return os << "|";
    case IROper::BitXor:return os << "xor";
    case IROper::SmmulI:return os << "*";
    case IROper::RsbI:return os << "rsb";
    default:
        assert(false);
        break;
    };
    return os;
}
static std::ostream& operator<<(std::ostream& os, IRShiftOper sop)
{
    switch (sop) {
    case IRShiftOper::Null:return os << "?";
    case IRShiftOper::ASR:return os << ">>";
    case IRShiftOper::LSL:return os << "<<";
    case IRShiftOper::LSR:return os << ">>>";
    default:
        assert(false);
    };
    return os;

}
static std::ostream& print_symbol_without_type(std::ostream& os, IRSymbol* ir_sym);
static std::ostream& operator<<(std::ostream& os, IRSymbol* ir_sym)
{
    os << ir_sym->basic_type();
    if (ir_sym->array_length() == IRArrayLength::IR_ARRAY_POINTER) {
        os << "* ";
    }
    else if (ir_sym->array_length() > 0) {
        os << " array ";
    }
    else os << " ";
    return print_symbol_without_type(os, ir_sym);
}
static std::ostream& print_symbol_without_type(std::ostream& os, IRSymbol* ir_sym)
{
    switch (ir_sym->kind()) {
    case IRSymbolKind::Global:
        return os << "@" << ir_sym->global_sym()->name();
    case IRSymbolKind::Local:
    case IRSymbolKind::Param:
        if (ir_sym->ssa_index() != -1)
            return os << "l" << ir_sym->index() << "_" << ir_sym->ssa_index();
        else return os << "l" << ir_sym->index();
    case IRSymbolKind::Temp:
        return os << "t" << ir_sym->index();
    case IRSymbolKind::Value: {
        if (ir_sym->basic_type() == BasicType::Float)
            return os << ir_sym->value().float_value;
        return os << ir_sym->value().int_value;
    }
    case IRSymbolKind::PhiFunc: {
        os << "phi(";
        bool first = true;
        for (auto param : ir_sym->phi_params()) {
            if (first)first = false;
            else os << ", ";
            os << param.sym << "[B" << param.from->get_index() << "]";
        }
        return os << ")";
    }
    case IRSymbolKind::Register: {
        if(ir_sym->index() >= RegCount)
            os << "s" << (ir_sym->index() - RegCount);
        else
            os << "r" << ir_sym->index();
        return os;
    }
    case IRSymbolKind::Memory:
        os << "mem(sp)" << ir_sym->index();
        return os;
    default:
        break;
    }
    return os << "[Error Oprand]";
}
void LinearIRManager::print_ir_symbol(IRSymbol* sym, std::ostream& os)
{
    os << sym;
}
static void print_symbol_array(std::ostream& os, IRSymbol* ir_sym)
{
    os << ir_sym->basic_type()<<" ";
    print_symbol_without_type(os, ir_sym);
}
void LinearIRManager::print_ir_instr(const IRInstr& instr, std::ostream& os, const std::string& prefix)
{
    switch (instr.type()) {
    case IRType::Label:
        os << get_lbl_str(instr.lbl0()) << ":" << std::endl;
        break;
    case IRType::BinaryCalc:
        if (instr.op() == IROper::SmmulI) {
            os << instr.r() << " = (" << instr.a() << " " << IROper::MulI << " " << instr.b() <<")[63:32]" << std::endl;
        }
        else if (instr.op() == IROper::SignedLargeMulI) {
            os << instr.r()<<" , "<<instr.c() << " = " << instr.a() << " " << IROper::MulI << " " << instr.b() << std::endl;
        }
        else {
            os << instr.r() << " = " << instr.a() << " " << instr.op() << " " << instr.b() << std::endl;
        }
        break;
    case IRType::UnaryCalc:
        os << instr.r() << " = " << instr.op() << " " << instr.a() << std::endl;
        break;
    case IRType::Assign:
        os << instr.r() << " = " << instr.a() << std::endl;
        break;
    case IRType::ArrayLoad: {
        os << instr.r() << " = ";
        print_symbol_array(os, instr.a());
        os << "[" << instr.b() << "]" << std::endl;
        break;
    }
    case IRType::ArrayStore: {
        print_symbol_array(os, instr.r());
        os << "[" << instr.a() << "] = " << instr.b() << std::endl;
        break;
    }
    case IRType::Goto:
        os << "goto " << get_lbl_str(instr.lbl0()) << std::endl;
        break;
    case IRType::CondGoto:
        os << "goto " << instr.a() << " ? " << get_lbl_str(instr.lbl1()) << " : " << get_lbl_str(instr.lbl0()) << std::endl;
        break;
    case IRType::Call:
        os << "call " << instr.a() << std::endl; /* << ", param " << instr.b() << std::endl;*/
        break;
    case IRType::CallWithRet:
        os << instr.r() << " = call " << instr.a() << std::endl;/*<< ", param " << instr.b() << std::endl;*/
        break;
    case IRType::Return:
        os << "return " << std::endl;
        break;
    case IRType::ValReturn:
        os << "return " << instr.a() << std::endl;
        break;
    case IRType::FuncDef:
        os << "func " << instr.a() << ",param " << instr.b() << std::endl;
        break;
    case IRType::FuncEnd:
        os << "end func" << std::endl;
        break;
    case IRType::FParam:
        if (instr.a()->array_length() == IRArrayLength::IR_ARRAY_POINTER)
            os << "fparam array " << instr.a() << std::endl;
        else
            os << "fparam " << instr.a() << std::endl;
        break;
    case IRType::RParam:
        os << "rparam " << instr.a() << std::endl;
        break;
    case IRType::GlobalDecl:
        if (instr.a()->array_length() > 0)
            os << "global " << instr.a() << "[" << instr.a()->array_length() << "]" << std::endl;
        else
            os << "global " << instr.a() << std::endl;
        break;
    case IRType::LocalDecl:
        if (instr.a()->array_length() > 0)
            os << "local " << instr.a() << "[" << instr.a()->array_length() << "]" << std::endl;
        else
            os << "local " << instr.a() << std::endl;
        break;
    case IRType::Load: {
        os << instr.r() << " = [" << instr.a() << "]" << std::endl;;
        break;
    }
    case IRType::Store: {
        os << "[" << instr.r() << "] = " << instr.a() << std::endl;
        break;
    }
    case IRType::BlockGoto:
        os << "uncond goto" << std::endl;
        break;
    case IRType::BlockCondGoto:
        os << "cond goto " << instr.a() << std::endl;
        break;
    case IRType::BlockBinaryGoto:
        os << "cond goto " << instr.a() << " " << instr.op() << " " << instr.b() << std::endl;
        break;
    case IRType::BlockUnaryGoto:
        os << "cond goto " << instr.op() << " " << instr.a() << std::endl;
        break;
    case IRType::PhiFunc:
        os << instr.r() << " = " << instr.a() << std::endl;
        break;
    case IRType::MemoryConvergeMark:
        os << "memory converge " << instr.a() << std::endl;
        break;
    case IRType::TernaryCalc:
        if (instr.op() == IROper::ShiftI) {
            os << instr.r() << " = " << instr.b() << " " << instr.sop() << " " << instr.c() << std::endl;
        }
        else if (instr.op() == IROper::ShiftAddI) {
            os << instr.r() << " = " << instr.a() << " " << IROper::AddI << " (" << instr.b() << " " << instr.sop() << " " << instr.c() << ")" << std::endl;
        }
        else if (instr.op() == IROper::ShiftSubI) {
            os << instr.r() << " = " << instr.a() << " " << IROper::SubI << " (" << instr.b() << " " << instr.sop() << " " << instr.c() << ")" << std::endl;
        }
        else if (instr.op() == IROper::ShiftRsbI) {
            os << instr.r() << " = " << "(" << instr.b() << " " << instr.sop() << " " << instr.c() << ") " << IROper::SubI << " " << instr.a() << std::endl;
        }
        else if (instr.op() == IROper::MulAddI || instr.op() == IROper::MulAddF) {
            os << instr.r() << " = " << instr.a() << " + " << instr.b() << " * " << instr.c() << std::endl;
        } 
        else if (instr.op() == IROper::SignedLargeMulI) {
            os << instr.c() << ", " << instr.r() << " = " << instr.a() << " * " << instr.b() << std::endl;
        }
        else if (instr.op() == IROper::SignedLargeMulAddI) {
            os << instr.r() << " = " << instr.a() << " + " << instr.b() << " * " << instr.c() << "[63:32]" << std::endl;
        }
        break;
    default:
        os << "[Unimplemented IR instruction!]" << std::endl;
        assert(false);
        break;
    }
}
void LinearIRManager::print_ir_list(const IRInstrList& program, std::ostream &os, const std::string &prefix)
{
    for (const auto &instr : program) {
        os << instr.no() << ":";
        os << prefix;
        print_ir_instr(instr,os,prefix);
    }
}
