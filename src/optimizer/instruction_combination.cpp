#include <instruction_combination.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <cfg_manager.h>
#include <linear_ir_manager.h>
#include <constant_recombination.h>
#include <constant_fold_and_algebraic_simplification.h>
#define NATURAL_ADD(x,y) constant_recombination::natural_add((x),(y))
#define NATURAL_SUB(x,y) constant_recombination::natural_sub((x),(y))
using namespace instruction_combination;
//#define DEBUG_INFO_OPTIMIZER
//删除关联语句
void InstructionCombination::delete_related_instr(IRSymbol* sym)
{
    if (sym->kind() == IRSymbolKind::Local || sym->kind() == IRSymbolKind::Param) {
        for (auto sym_ : m_related_sym[sym->get_tag()])
            m_sym_def_set.erase(SymDefPair(sym_));
        m_related_sym[sym->get_tag()].clear();
    } else if (sym->kind() == IRSymbolKind::Global) {
        for (auto sym_ : m_related_sym_glb[sym->get_tag()])
            m_sym_def_set_glb.erase(SymDefPair(sym_));
        m_related_sym_glb[sym->get_tag()].clear();
    }
}
bool InstructionCombination::is_not_considered_type(IRSymbol* sym)
{
    if (sym == nullptr)
        return false;
    if (sym->kind() == IRSymbolKind::Register || sym->kind() == IRSymbolKind::Memory)
        return true;
    if (sym->array_length() != IRArrayLength::IR_NONE_ARRAY)
        return true;
    return false;
}

//判断符号是否相等（如果为v类型，判断值是否相等，否则判断是否为同一个符号）
bool InstructionCombination::symbol_equal(IRSymbol* a,IRSymbol* b) {
    if (a->kind() != b->kind()||a->basic_type()!=b->basic_type())
        return false;
    if (a->kind() == IRSymbolKind::Value) {
        if (a->basic_type() == BasicType::Int)
            return a->int_value() == b->int_value();
        else if (a->basic_type() == BasicType::Float)
            return a->float_value() == b->float_value();
        else assert(false);
    }
    return a == b;
}
//对于v类型，创建一个新的symbol，否则用原symbol
IRSymbol* InstructionCombination::symbol_copy(IRSymbol* a) {
    if (a->kind() == IRSymbolKind::Value)
        return m_ir_sym_table->create_value(a->basic_type(), a->value());
    return a;
}
std::vector<MulAddExpr> InstructionCombination::gen_mul_add_expr(IRInstr* instr)
{
#define VAL_2 m_ir_sym_table->create_value_2(basic_type)
#define VAL_1 m_ir_sym_table->create_value_1(basic_type)
#define VAL_0 m_ir_sym_table->create_value_0(basic_type)
#define VAL_M1 m_ir_sym_table->create_value_m1(basic_type)
#define A symbol_copy(instr->a())
#define B symbol_copy(instr->b())
#define C symbol_copy(instr->c())
    std::vector<MulAddExpr> res;
    if (instr->type() == IRType::Assign) {
        auto basic_type = instr->a()->basic_type();
        //<a>+<0>*<0>
        res.push_back({ A,VAL_0,VAL_0 });
        //<0>+<1>*<a>
        res.push_back({ VAL_0,VAL_1,A });
        //<0>+<a>*<1>
        res.push_back({ VAL_0,A,VAL_1 });
    }else if (instr->type() == IRType::BinaryCalc) {
        BasicType basic_type;
        bool reset_type = true;
        switch (instr->op()){
        case IROper::AddI:
            basic_type = BasicType::Int;
            reset_type = false;
            [[fallthrough]];
        case IROper::AddF:
            if(reset_type)
                basic_type = BasicType::Float;
            if (symbol_equal(instr->a(), instr->b())) {
                //<0>+<a1>*<2>
                res.push_back({ VAL_0,A,VAL_2 });
                //<0>+<2>*<a1>
                res.push_back({ VAL_0,VAL_2,A });
            } else {
                //<a>+<b>*<1>
                res.push_back({ A,B,VAL_1 });
                //<b>+<a>*<1>
                res.push_back({ B,A,VAL_1 });
                //<a>+<1>*<b>
                res.push_back({ A,VAL_1,B });
                //<b>+<1>*<a>
                res.push_back({ B,VAL_1,A });
            }
            break;
        case IROper::SubI:
            basic_type = BasicType::Int;
            reset_type = false;
            [[fallthrough]];
        case IROper::SubF:
            if (reset_type)
                basic_type = BasicType::Float;
            if (symbol_equal(instr->a(), instr->b())) {
                //<0>+<0>*<0>
                res.push_back({ VAL_0,VAL_0,VAL_0 });
            } else {
                //<a>+<b>*<-1>
                res.push_back({ A,B,VAL_M1 });
                //<a>+<-1>*<b>
                res.push_back({ A,VAL_M1,B });
            }
            break;
        case IROper::MulI:
            basic_type = BasicType::Int;
            reset_type = false;
            [[fallthrough]];
        case IROper::MulF:
            if (reset_type)
                basic_type = BasicType::Float;
            //<0>+<a>*<b>
            res.push_back({ VAL_0,A,B });
            //<0>+<b>*<a>
            res.push_back({ VAL_0,B,A });
            break;
        default:
            break;
        }
    }else if (instr->type() == IRType::UnaryCalc) {
        BasicType basic_type;
        bool reset_type = true;
        switch (instr->op()) {
        case IROper::NegI:
            basic_type = BasicType::Int;
            reset_type = false;
            [[fallthrough]];
        case IROper::NegF:
            if (reset_type)
                basic_type = BasicType::Float;
            //<0>+<a>*<-1>
            res.push_back({ VAL_0,A,VAL_M1 });
            //<0>+<-1>*<a>
            res.push_back({ VAL_0,VAL_M1,A });
            break;
        default:
            break;
        }
    }else if (instr->type() == IRType::TernaryCalc) {
        switch (instr->op()) {
        case IROper::MulAddI:
            [[fallthrough]];
        case IROper::MulAddF:
            //<a>+<b>*<c>
            res.push_back({ A,B,C });
            //<a>+<c>*<b>
            res.push_back({ A,C,B });
            break;
        default:
            break;
        }
    }
    else assert(false);
    return res;
#undef A
#undef B
#undef C
#undef VAL_2
#undef VAL_1
#undef VAL_0
#undef VAL_M1
}
void InstructionCombination::simplify_binary_calc_instr(IRInstr& instr)
{
    switch (instr.op()) {
    case IROper::AddI:
        if (instr.a()->is_value() && instr.b()->is_value()) {//常量折叠
            instr.a()->set_int_value(instr.a()->int_value() + instr.b()->int_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->kind() == IRSymbolKind::Value && instr.a()->int_value() == 0) {//代数化简,a为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 0)   //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        break;
    case IROper::AddF:
        if (instr.a()->is_value() && instr.b()->is_value()) {//常量折叠
            instr.a()->set_float_value(instr.a()->float_value() + instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 0) {//代数化简,a为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 0)   //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        break;
    case IROper::SubI:
        if (instr.a()->is_value() && instr.b()->is_value()) {//常量折叠
            instr.a()->set_int_value(instr.a()->int_value() - instr.b()->int_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 0) //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (instr.a()->is_value() && instr.a()->int_value() == 0) { //代数化简,a为0则结果等于-b
            instr.rebind_a(instr.b());
            instr.reset_op(IROper::NegI);
            instr.rewrite_type(IRType::UnaryCalc);
        }
        else if (instr.a() == instr.b()) { //代数化简,a=b则结果为0
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Int, 0));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::SubF:
        if (instr.a()->is_value() && instr.b()->is_value()) {//常量折叠
            instr.a()->set_float_value(instr.a()->float_value() - instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 0) //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (instr.a()->is_value() && instr.a()->float_value() == 0) { //代数化简,a为0则结果等于-b
            instr.rebind_a(instr.b());
            instr.reset_op(IROper::NegF);
            instr.rewrite_type(IRType::UnaryCalc);
        }
        else if (instr.a() == instr.b()) { //代数化简,a=b则结果为0
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Float, 0.0f));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::MulI:
        if (instr.a()->is_value() && instr.b()->is_value()) {//常量折叠
            instr.a()->set_int_value(instr.a()->float_value() * instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->int_value() == 0)//代数化简，a为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (instr.b()->is_value() && instr.b()->int_value() == 0) {//代数化简，b为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 1) {//代数化简，b为1则结果等于a
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->int_value() == 1) {//代数化简，a为1则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::MulF:
        if (instr.a()->is_value() && instr.b()->is_value()) {//常量折叠
            instr.a()->set_float_value(instr.a()->float_value() * instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 0)//代数化简，a为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (instr.b()->is_value() && instr.b()->float_value() == 0) {//代数化简，b为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 1.0f) {//代数化简，b为1.0则结果等于a
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 1.0f) {//代数化简，a为1.0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        break;
    default:
        break;
    }
}
void InstructionCombination::simplify_mul_add_instr(IRInstr& instr)
{
    assert(instr.type() == IRType::TernaryCalc);
    if (instr.op() == IROper::MulAddI) {
        if (instr.c()->is_value() && instr.c()->int_value() == 0)
            instr.rewrite_type(IRType::Assign);
        else if (instr.b()->is_value() && instr.b()->int_value() == 0)
            instr.rewrite_type(IRType::Assign);
        else if (instr.c()->is_value() && instr.c()->int_value() == 1) {
            instr.reset_op(IROper::AddI);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 1) {
            instr.rebind_b(instr.c());
            instr.reset_op(IROper::AddI);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.c()->is_value() && instr.c()->int_value() == -1) {
            instr.reset_op(IROper::SubI);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == -1) {
            instr.rebind_b(instr.c());
            instr.reset_op(IROper::SubI);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.a()->is_value() && instr.a()->int_value() == 0) {
            instr.reset_op(IROper::MulI);
            instr.rebind_a(instr.b());
            instr.rebind_b(instr.c());
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (symbol_equal(instr.a(),instr.b())&&instr.c()->is_value()) {
            instr.reset_op(IROper::MulI);
            instr.rebind_b(m_ir_sym_table->create_value(BasicType::Int, instr.c()->int_value() + 1));
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (symbol_equal(instr.a(), instr.c()) && instr.b()->is_value()) {
            instr.reset_op(IROper::MulI);
            instr.rebind_b(m_ir_sym_table->create_value(BasicType::Int, instr.b()->int_value() + 1));
            instr.rewrite_type(IRType::BinaryCalc);
        }
            
    } else if (instr.op() == IROper::MulAddF) {
        if (instr.c()->is_value() && instr.c()->float_value() == 0.0f)
            instr.rewrite_type(IRType::Assign);
        else if (instr.b()->is_value() && instr.b()->float_value() == 0.0f)
            instr.rewrite_type(IRType::Assign);
        else if (instr.c()->is_value() && instr.c()->float_value() == 1.0f) {
            instr.reset_op(IROper::AddF);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 1.0f) {
            instr.rebind_b(instr.c());
            instr.reset_op(IROper::AddF);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.c()->is_value() && instr.c()->float_value() == -1.0f) {
            instr.reset_op(IROper::SubF);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == -1.0f) {
            instr.rebind_b(instr.c());
            instr.reset_op(IROper::SubF);
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 0.0) {
            instr.reset_op(IROper::MulF);
            instr.rebind_a(instr.b());
            instr.rebind_b(instr.c());
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (symbol_equal(instr.a(), instr.b()) && instr.c()->kind() == IRSymbolKind::Value) {
            instr.reset_op(IROper::MulF);
            instr.rebind_b(m_ir_sym_table->create_value(BasicType::Int, instr.c()->float_value() + 1.0f));
            instr.rewrite_type(IRType::BinaryCalc);
        }
        else if (symbol_equal(instr.a(), instr.c()) && instr.b()->kind() == IRSymbolKind::Value) {
            instr.reset_op(IROper::MulF);
            instr.rebind_b(m_ir_sym_table->create_value(BasicType::Int, instr.b()->float_value() + 1.0f));
            instr.rewrite_type(IRType::BinaryCalc);
        }
    }
    if (instr.type() == IRType::BinaryCalc) {
        simplify_binary_calc_instr(instr);
    }
}
std::pair<bool, IRInstr> InstructionCombination::try_merge(IRInstr* prev, IRInstr* now)
{
    MulAddExpr res;
    bool succeed = false;
    const std::vector<MulAddExpr>& expr_list_a = gen_mul_add_expr(prev);
    const std::vector<MulAddExpr>& expr_list_b = gen_mul_add_expr(now);
    for (auto& expr_a : expr_list_a) {
        for (auto& expr_b : expr_list_b) {
            assert(expr_a.a->basic_type() == expr_b.a->basic_type());
            auto basic_type = expr_a.a->basic_type();
            bool cond1, cond2, cond3,cond4,cond5;
            /*
                合并策略4 整数
                <0>+<a1>*<v1>
                <0>+<p>*<v2>
                结果：<0>+<a1>*<v1*v2>
            */
            bool cond0 = basic_type == BasicType::Int;
            if (basic_type == BasicType::Int) {
                cond1 = expr_a.a->kind() == IRSymbolKind::Value && expr_a.a->int_value() == 0;
                cond2 = expr_b.a->kind() == IRSymbolKind::Value && expr_b.a->int_value() == 0;
            }
            else {
                cond1 = expr_a.a->kind() == IRSymbolKind::Value && expr_a.a->float_value() == 0.0;
                cond2 = expr_b.a->kind() == IRSymbolKind::Value && expr_b.a->float_value() == 0.0;
            }
            cond3 = expr_b.b == prev->r();
            cond4 = expr_a.c->kind() == IRSymbolKind::Value;
            cond5 = expr_b.c->kind() == IRSymbolKind::Value;
            if (cond0 && cond1 && cond2 && cond3 && cond4 && cond5) {
                succeed = true;
                if (basic_type == BasicType::Int)
                    res = MulAddExpr(expr_a.a, expr_a.b, m_ir_sym_table->create_value(basic_type, expr_a.c->int_value() * expr_b.c->int_value()));
                else res = MulAddExpr(expr_a.a, expr_a.b, m_ir_sym_table->create_value(basic_type, expr_a.c->float_value() * expr_b.c->float_value()));
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 4" << std::endl;
#endif
                break;
            }
            /*
               合并策略2：（整数）
               <a1>+<a2>*<v1>
               <p>+<a2>*<v2>
               *溢出检查
               结果：<a1>+<a2>*<v1 + v2>
           */
            cond1 = symbol_equal(expr_a.b, expr_b.b);
            cond2 = expr_a.c->kind() == IRSymbolKind::Value && expr_b.c->kind() == IRSymbolKind::Value;
            cond3 = expr_b.a == prev->r();
            cond4 = basic_type == BasicType::Int;
            if (cond1 && cond2 && cond3&& cond4) {
                succeed = true;
                res = MulAddExpr(expr_a.a, expr_a.b, m_ir_sym_table->create_value(basic_type, NATURAL_ADD(expr_a.c->int_value(), expr_b.c->int_value())));
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 2" << std::endl;
#endif
                break;
            }

            /*
               合并策略6：（整数）
               <a1>+<a2>*<-1>
               <a1>+<p>*<-1>
               结果：<a2>+<0>*<0>
           */
            cond1 = symbol_equal(expr_a.a, expr_b.a);
            cond2 = expr_a.c->is_value_m1() && expr_b.c->is_value_m1();
            cond3 = expr_b.b == prev->r();
            cond4 = basic_type == BasicType::Int;
            if (cond1 && cond2 && cond3 && cond4) {
                succeed = true;
                res = MulAddExpr(expr_a.b, m_ir_sym_table->create_value_0(basic_type), m_ir_sym_table->create_value_0(basic_type));
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 6" << std::endl;
#endif
                break;
            }
            /*
               合并策略5：（浮点数）
               <0>+<a2>*<v1>
               <p>+<a2>*<v2>
               结果：<p>+<a2>*<v1 + v2>
           */
            /*
            cond1 = symbol_equal(expr_a.b, expr_b.b);
            cond2 = expr_a.c->kind() == IRSymbolKind::Value && expr_b.c->kind() == IRSymbolKind::Value;
            cond3 = expr_b.a == prev->r();
            cond4 = basic_type == BasicType::Float;
            cond5 = expr_a.a->kind() == IRSymbolKind::Value && expr_a.a->float_value() == 0.0;
            if (cond1 && cond2 && cond3 && cond4 && cond5) {
                succeed = true;
                res = MulAddExpr(expr_a.a, expr_a.b, m_ir_sym_table->create_value(basic_type, expr_a.c->float_value() + expr_b.c->float_value()));
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 5" << std::endl;
#endif
                break;
            }*/
            /*
                合并策略1：（整数）
                <0>+<a1>*<a2>
                <p>+<a3>*<1>
                结果：<a3>+<a1>*<a2>
            */
            cond1 = false;
            if (basic_type == BasicType::Int) {
                cond1 = expr_a.a->kind() == IRSymbolKind::Value && expr_a.a->int_value() == 0;
                cond2 = expr_b.a == prev->r();
                cond3 = expr_b.c->kind() == IRSymbolKind::Value && expr_b.c->int_value() == 1;
            }
            if (cond1 && cond2 && cond3) {
                succeed = true;
                res = MulAddExpr(expr_b.b, expr_a.b, expr_a.c);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 1" << std::endl;
#endif
                break;
            }
           
            /*
                合并策略3(整数、浮点数）
                <a1>+<0>*<0>
                <0>+<p>*<v1>
                结果：<0>+<a1>*<v1>
            */
            if (basic_type == BasicType::Int) {
                cond1 = expr_a.b->kind() == IRSymbolKind::Value && expr_a.b->int_value() == 0;
                cond2 = expr_a.c->kind() == IRSymbolKind::Value && expr_a.c->int_value() == 0;
                cond3 = expr_b.a->kind() == IRSymbolKind::Value && expr_b.a->int_value() == 0;
            }
            else {
                cond1 = expr_a.b->kind() == IRSymbolKind::Value && expr_a.b->float_value() == 0.0f;
                cond2 = expr_a.c->kind() == IRSymbolKind::Value && expr_a.c->float_value() == 0.0f;
                cond3 = expr_b.a->kind() == IRSymbolKind::Value && expr_b.a->float_value() == 0.0f;
            }
            cond4 = expr_a.c->kind() == IRSymbolKind::Value;
            cond5 = expr_b.b == prev->r();
            if (cond1 && cond2 && cond3 && cond4 && cond5) {
                succeed = true;
                if (basic_type == BasicType::Int)
                    res = MulAddExpr(expr_b.a, expr_a.a, expr_b.c);
                else res = MulAddExpr(expr_b.a, expr_a.a, expr_b.c);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 3" << std::endl;
                break;
#endif
            }
            
        }
        if (succeed)
            break;
    }
    IRInstr res_instr = IRInstr::create_return();
    if (succeed) {
        auto basic_type = res.a->basic_type();
        if (basic_type == BasicType::Int) {
            res_instr = IRInstr::create_ternary_calc(IROper::MulAddI, IRShiftOper::Null, now->r(), res.a, res.b, res.c);
        }
        else res_instr = IRInstr::create_ternary_calc(IROper::MulAddF, IRShiftOper::Null, now->r(), res.a, res.b, res.c);
        simplify_mul_add_instr(res_instr);
    }
    return { succeed,res_instr };
}
void InstructionCombination::check_merge(IRInstr* instr)
{
    if (instr->type() == IRType::BinaryCalc) {
        if (instr->op() != IROper::AddI &&
            instr->op() != IROper::AddF &&
            instr->op() != IROper::MulI &&
            instr->op() != IROper::MulF &&
            instr->op() != IROper::SubI &&
            instr->op() != IROper::SubF)
            return;
    }else if(instr->type() == IRType::TernaryCalc){
        if (instr->op() != IROper::MulAddI&& instr->op() != IROper::MulAddF)
            return;
    }
    bool one_succeed = true;
    while(one_succeed){
        one_succeed = false;
        if (instr->a() != nullptr) {
            if (auto iter = m_sym_def_set.find(SymDefPair(instr->a())); iter != m_sym_def_set.end()) {
                if (auto [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    iter->def->set_no(0);
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
                m_sym_def_set.erase(iter);
            }
            else if (auto iter = m_sym_def_set_glb.find(SymDefPair(instr->a())); iter != m_sym_def_set_glb.end()) {
                if (const auto& [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    iter->def->set_no(0);
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
                m_sym_def_set_glb.erase(iter);
            }
        }
        if (instr->b() != nullptr) {
            if (auto iter = m_sym_def_set.find(SymDefPair(instr->b())); iter != m_sym_def_set.end()) {
                if (const auto& [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    iter->def->set_no(0);
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
                m_sym_def_set.erase(iter);
            }
            else if (auto iter = m_sym_def_set_glb.find(SymDefPair(instr->b())); iter != m_sym_def_set_glb.end()) {
                if (const auto& [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    iter->def->set_no(0);
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
                m_sym_def_set_glb.erase(iter);
            }
        }
        if (instr->c() != nullptr) {
            if (auto iter = m_sym_def_set.find(SymDefPair(instr->c())); iter != m_sym_def_set.end()) {
                if (const auto& [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    iter->def->set_no(0);
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
                m_sym_def_set.erase(iter);
            }
            else if (auto iter = m_sym_def_set_glb.find(SymDefPair(instr->c())); iter != m_sym_def_set_glb.end()) {
                if (const auto& [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    iter->def->set_no(0);
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
                m_sym_def_set_glb.erase(iter);
            }
        }
    }
}
void InstructionCombination::check_insert(IRInstr* instr)
{
    if (instr->type() == IRType::BinaryCalc) {
        if (instr->op() != IROper::AddI &&
            instr->op() != IROper::AddF &&
            instr->op() != IROper::MulI &&
            instr->op() != IROper::MulF &&
            instr->op() != IROper::SubI &&
            instr->op() != IROper::SubF)
            return;
    }
    else if (instr->type() == IRType::TernaryCalc) {
        if (instr->op() != IROper::MulAddI && instr->op() != IROper::MulAddF)
            return;
    }
    if (is_non_pointer_temp_var(instr->r()) && m_use_count[instr->r()->get_tag()] == 1) {
        //LinearIRManager::print_ir_instr(*instr);
        //插入对应赋值语句
        if (instr->a() != nullptr && instr->a()->is_global() ||
            instr->b() != nullptr && instr->b()->is_global() ||
            instr->c() != nullptr && instr->c()->is_global()) {
            m_sym_def_set_glb.insert(SymDefPair(instr));
        }else m_sym_def_set.insert(SymDefPair(instr));
        //在本指令涉及的操作数上维护关联语句
        if (instr->a() != nullptr && (instr->a()->kind() == IRSymbolKind::Local || instr->a()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->a()->get_tag()].push_back(instr->r());
        else if (instr->a() != nullptr && instr->a()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->a()->get_tag()].push_back(instr->r());
        if (instr->b() != nullptr && (instr->b()->kind() == IRSymbolKind::Local || instr->b()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->b()->get_tag()].push_back(instr->r());
        else if (instr->b() != nullptr && instr->b()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->b()->get_tag()].push_back(instr->r());
        if (instr->c() != nullptr && (instr->c()->kind() == IRSymbolKind::Local || instr->c()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->c()->get_tag()].push_back(instr->r());
        else if (instr->c() != nullptr && instr->c()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->c()->get_tag()].push_back(instr->r());
    }
}
void InstructionCombination::work_assign(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //检查本语句是否能和前一句合并
    check_merge(instr);
    //检查本语句是否能够作为替代其他语句的候选
    check_insert(instr);
    //删除关联语句
    delete_related_instr(instr->r());
}
void InstructionCombination::work_unary_calc(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //检查本语句是否能和前一句合并
    check_merge(instr);
    //检查本语句是否能够作为替代其他语句的候选
    check_insert(instr);
    //删除关联语句
    delete_related_instr(instr->r());
}

void InstructionCombination::work_binary_calc(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->b()) ||
        is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //检查本语句是否能和前一句合并
    check_merge(instr);
    //检查本语句是否能够作为替代其他语句的候选
    check_insert(instr);
    //删除关联语句
    delete_related_instr(instr->r());
}
void InstructionCombination::work_ternary_calc(IRInstr* instr)
{
    //暂不考虑数组
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a()) ||
        is_not_considered_type(instr->b()) ||
        is_not_considered_type(instr->c())) {
        return;
    }
    //检查本语句是否能被替代
    check_merge(instr);
    //检查本语句是否能够作为替代其他语句的候选
    check_insert(instr);
    //删除关联语句
    delete_related_instr(instr->r());
}
void InstructionCombination::work_ir_list(IRInstrList& program)
{
    m_sym_def_set.clear();
    m_sym_def_set_glb.clear();
    for (auto& instr:program) {
        instr.set_no(1);//1表示活代码，0表示死代码
        switch (instr.type()) {
        case IRType::Assign:
            work_assign(&instr);
            break;
        case IRType::UnaryCalc:
            work_unary_calc(&instr);
            break;
        case IRType::BinaryCalc:
            work_binary_calc(&instr);
            break;
        case IRType::TernaryCalc:
            work_ternary_calc(&instr);
            break;
        case IRType::Call:
            [[fallthrough]];
        case IRType::CallWithRet:
            m_sym_def_set_glb.clear();
            break;
        default:
            break;
        }
    }
    //删除被替代的t变量
    for (auto instr = program.begin(); instr != program.end();) {
        if (instr->no()==0)
            instr = program.erase(instr);
        else instr++;
    }
}
void InstructionCombination::work_cfg(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        work_ir_list(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void InstructionCombination::init()
{
    m_global_count = 0;
    m_related_sym_glb.clear();
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::VarDef) {
            for (auto& instr : unit.get_definations()) {
                if (instr.a()->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                    instr.a()->set_tag(m_global_count++);
                    m_related_sym_glb.push_back({});
                }
            }
        }
}
bool InstructionCombination::is_non_pointer_temp_var(IRSymbol* sym)
{
    return sym != nullptr && sym->kind() == IRSymbolKind::Temp &&sym->array_length() == IRArrayLength::IR_NONE_ARRAY;
}
//按照bfs序遍历块，定值一定在使用之前出现
void InstructionCombination::get_use_info_for_temp_vars(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        for (auto& instr : now->get_instr_list()) {
            if (is_non_pointer_temp_var(instr.r())) {
                instr.r()->set_tag(m_temp_count);
                m_temp_count++;
                m_use_count.push_back(0);
            }
            if (is_non_pointer_temp_var(instr.a())) {
                assert(instr.a()->get_tag() >= 0);
                m_use_count[instr.a()->get_tag()]++;
            }
            if (is_non_pointer_temp_var(instr.b())) {
                assert(instr.b()->get_tag() >= 0);
                m_use_count[instr.b()->get_tag()]++;
            }
            if (is_non_pointer_temp_var(instr.c())) {
                assert(instr.c()->get_tag() >= 0);
                m_use_count[instr.c()->get_tag()]++;
            }
        }
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void InstructionCombination::run()
{
    std::cout << "Running pass: Instruction Combination " << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    init();
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef) {
            m_temp_count = 0;
            m_use_count.clear();
            m_local_param_count = 0;
            m_related_sym.clear();
            //对l、p变量编号
            for (auto& def_instr : unit.get_definations()) {
                if (def_instr.a()->kind() == IRSymbolKind::Local || def_instr.a()->kind() == IRSymbolKind::Param)
                    if (def_instr.a()->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                        def_instr.a()->set_tag(m_local_param_count);
                        m_related_sym.push_back({});
                        m_local_param_count++;
                    }
            }
            get_use_info_for_temp_vars(&unit);
            work_cfg(&unit);
        }
}
