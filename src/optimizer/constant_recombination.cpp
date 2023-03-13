#include <constant_recombination.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <cfg_manager.h>
#include <linear_ir_manager.h>
#include <constant_fold_and_algebraic_simplification.h>
using namespace constant_recombination;
const bool Ofast=true;
//#define DEBUG_INFO_OPTIMIZER
//ɾ���������
namespace constant_recombination {
    //�õ���Ȼ�������ļӷ�
    int natural_add(int a, int b)
    {
        return (int)((unsigned int)a + (unsigned int)b);
    }
    //�õ���Ȼ�������ļ���
    int natural_sub(int a, int b)
    {
        return (int)((unsigned int)a - (unsigned int)b);
    }
}
void ConstantRecombination::delete_related_instr(IRSymbol* sym)
{
    if (sym->kind() == IRSymbolKind::Local || sym->kind() == IRSymbolKind::Param) {
        for (auto sym_ : m_related_sym[sym->def_sym()->get_tag()])
            m_sym_def_set.erase(SymDefPair(sym_));
        m_related_sym[sym->def_sym()->get_tag()].clear();
    } else if (sym->kind() == IRSymbolKind::Global) {
        for (auto sym_ : m_related_sym_glb[sym->get_tag()]) {
#ifdef DEBUG_INFO_OPTIMIZER
            std::cout << "del ";
            LinearIRManager::print_ir_symbol(sym_);
            std::cout << std::endl;
#endif // DEBUG_INFO_OPTIMIZER
            m_sym_def_set.erase(SymDefPair(sym_));
        }
        m_related_sym_glb[sym->get_tag()].clear();
    }
}
bool ConstantRecombination::is_not_considered_type(IRSymbol* sym)
{
    if (sym == nullptr)
        return false;
    if (sym->array_length() != IRArrayLength::IR_NONE_ARRAY)
        return true;
    return false;
}

//�жϷ����Ƿ���ȣ����Ϊv���ͣ��ж�ֵ�Ƿ���ȣ������ж��Ƿ�Ϊͬһ�����ţ�
bool ConstantRecombination::symbol_equal(IRSymbol* a,IRSymbol* b) {
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
//����v���ͣ�����һ���µ�symbol��������ԭsymbol
IRSymbol* ConstantRecombination::symbol_copy(IRSymbol* a) {
    if (a->kind() == IRSymbolKind::Value)
        return m_ir_sym_table->create_value(a->basic_type(), a->value());
    return a;
}
std::vector<Expr> ConstantRecombination::gen_expr(IRInstr* instr)
{
#define VAL_2 m_ir_sym_table->create_value_2(basic_type)
#define VAL_1 m_ir_sym_table->create_value_1(basic_type)
#define VAL_0 m_ir_sym_table->create_value_0(basic_type)
#define VAL_M1 m_ir_sym_table->create_value_m1(basic_type)
#define A symbol_copy(instr->a())
#define B symbol_copy(instr->b())
#define C symbol_copy(instr->c())
    std::vector<Expr> res;
    BasicType basic_type=BasicType::Uncertain;
    if (instr->type() == IRType::Assign) {
        if (instr->r()->basic_type() == BasicType::Int||instr->r()->is_array_or_pointer())
            basic_type = BasicType::Int;
        else
            basic_type = BasicType::Float;
        if (basic_type == BasicType::Int) {
            //<a>+<0>
            res.push_back({ A,VAL_0,IROper::AddI });
            //<0>+<a>
            res.push_back({ VAL_0,A ,IROper::AddI });
        }else if(basic_type == BasicType::Float) {
            //<a>+<0>
            res.push_back({ A,VAL_0,IROper::AddF });
            //<0>+<a>
            res.push_back({ VAL_0,A ,IROper::AddF });
        }
    }else if (instr->type() == IRType::BinaryCalc) {
        switch (instr->op()){
        case IROper::AddF:
            basic_type = BasicType::Float;
            if (instr->a()->is_value_0())
                instr->swap_a_and_b();
            if (instr->b()->is_value_0()) {
                //<a>*<1>
                res.push_back({ A,VAL_1,IROper::MulF });
                //<1>*<a>
                res.push_back({ VAL_1,A,IROper::MulF });
            }
            //<b>+<a>
            res.push_back({ B,A,IROper::AddF });
            //<a>+<b>
            res.push_back({ A,B, IROper::AddF });
            break;
        case IROper::AddI:
            basic_type = BasicType::Int;
            if (instr->a()->is_value_0())
                instr->swap_a_and_b();
            if (instr->b()->is_value_0()){
                //<a>*<1>
                res.push_back({ A,VAL_1,IROper::MulI });
                //<1>*<a>
                res.push_back({ VAL_1,A,IROper::MulI });
            }
            //<b>+<a>
            res.push_back({ B,A,IROper::AddI });
            //<a>+<b>
            res.push_back({ A,B, IROper::AddI });
            break;
        case IROper::SubF:
            //<a>-<b>
            res.push_back({ A,B, IROper::SubF });
            break;
        case IROper::SubI:
            //<a>-<b>
            res.push_back({ A,B, IROper::SubI });
            break;
        case IROper::MulF:
            //<b>*<a>
            res.push_back({ B,A,IROper::MulF });
            //<a>*<b>
            res.push_back({ A,B, IROper::MulF });
            break;
        case IROper::MulI:
            //<b>*<a>
            res.push_back({ B,A,IROper::MulI });
            //<a>*<b>
            res.push_back({ A,B, IROper::MulI });
            break;
        case IROper::DivF:
            //<a>/<b>
            res.push_back({ A,B, IROper::DivF });
            break;
        case IROper::DivI :
            //<a>/<b>
            res.push_back({ A,B, IROper::DivI });
            break;
        default:
            break;
        }
    }else if (instr->type() == IRType::UnaryCalc) {
        switch (instr->op()) {
        case IROper::NegF:
            basic_type = BasicType::Float;
            //<0>-<a>
            res.push_back({ VAL_0,A, IROper::SubF });
            break;
        case IROper::NegI:
            basic_type = BasicType::Int;
            //<0>-<a>
            res.push_back({ VAL_0,A, IROper::SubI });
            break;
        default:
            break;
        }
    }
    else assert(false);
    /*
    for (auto& expr : res) {
        assert(expr.a->basic_type() == expr.b->basic_type());
    }*/
    return res;
#undef A
#undef B
#undef C
#undef VAL_2
#undef VAL_1
#undef VAL_0
#undef VAL_M1
}
void ConstantRecombination::simplify_instr(IRInstr& instr)
{
    switch (instr.op()) {
    case IROper::AddI:
        if (instr.a()->is_value() && instr.b()->is_value()) {//�����۵�
            instr.a()->set_int_value(instr.a()->int_value() + instr.b()->int_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->kind() == IRSymbolKind::Value && instr.a()->int_value() == 0) {//��������,aΪ0��������b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 0)   //��������,bΪ0��������a
            instr.rewrite_type(IRType::Assign);
        break;
    case IROper::AddF:
        if (instr.a()->is_value() && instr.b()->is_value()) {//�����۵�
            instr.a()->set_float_value(instr.a()->float_value() + instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 0.0f) {//��������,aΪ0��������b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 0.0f)   //��������,bΪ0��������a
            instr.rewrite_type(IRType::Assign);
        break;
    case IROper::SubI:
        if (instr.a()->is_value() && instr.b()->is_value()) {//�����۵�
            instr.a()->set_int_value(instr.a()->int_value() - instr.b()->int_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 0) //��������,bΪ0��������a
            instr.rewrite_type(IRType::Assign);
        else if (instr.a()->is_value() && instr.a()->int_value() == 0) { //��������,aΪ0��������-b
            instr.rebind_a(instr.b());
            instr.reset_op(IROper::NegI);
            instr.rewrite_type(IRType::UnaryCalc);
        }
        else if (instr.a() == instr.b()) { //��������,a=b����Ϊ0
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Int, 0));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::SubF:
        if (instr.a()->is_value() && instr.b()->is_value()) {//�����۵�
            instr.a()->set_float_value(instr.a()->float_value() - instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 0.0f) //��������,bΪ0��������a
            instr.rewrite_type(IRType::Assign);
        else if (instr.a()->is_value() && instr.a()->float_value() == 0.0f) { //��������,aΪ0��������-b
            instr.rebind_a(instr.b());
            instr.reset_op(IROper::NegI);
            instr.rewrite_type(IRType::UnaryCalc);
        }
        else if (instr.a() == instr.b()) { //��������,a=b����Ϊ0
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Float, 0.0f));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::MulI:
        if (instr.a()->is_value() && instr.b()->is_value()) {//�����۵�
            instr.a()->set_int_value(instr.a()->int_value() * instr.b()->int_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->int_value() == 0)//��������aΪ0��������a
            instr.rewrite_type(IRType::Assign);
        else if (instr.b()->is_value() && instr.b()->int_value() == 0) {//��������bΪ0��������b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->int_value() == 1) {//��������bΪ1��������a
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->int_value() == 1) {//��������aΪ1��������b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::MulF:
        if (instr.a()->is_value() && instr.b()->is_value()) {//�����۵�
            instr.a()->set_float_value(instr.a()->float_value() * instr.b()->float_value());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 0.0f)//��������aΪ0��������a
            instr.rewrite_type(IRType::Assign);
        else if (instr.b()->is_value() && instr.b()->float_value() == 0.0f) {//��������bΪ0��������b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.b()->is_value() && instr.b()->float_value() == 1.0f) {//��������bΪ1��������a
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()->is_value() && instr.a()->float_value() == 1.0f) {//��������aΪ1��������b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        break;
    default:
        break;
    }
}

static bool in_int_range(int64_t num) {
    return num >= -2147483648LL && num < 2147483648LL;
}

std::pair<bool, IRInstr> ConstantRecombination::try_merge(IRInstr* prev, IRInstr* now)
{
    Expr res;
    bool succeed = false;
    const std::vector<Expr>& expr_list_a = gen_expr(prev);
    const std::vector<Expr>& expr_list_b = gen_expr(now);
    for (auto& expr_a : expr_list_a) {
        for (auto& expr_b : expr_list_b) {
            assert(expr_a.a->basic_type() == expr_b.a->basic_type());
            bool cond=true;
            /*
                �ϲ�����1 �����Ӻϲ� 
                ���� ������
                <a1>+<v1>
                <p>+<v2>
                *������
                �����<a1>+<v1+v2>
            */
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::AddI && expr_b.op == IROper::AddI;
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a,m_ir_sym_table->create_value(BasicType::Int, natural_add(expr_a.b->int_value(),expr_b.b->int_value())),IROper::AddI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 1" << std::endl;
#endif
                break;
            }
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::AddF && expr_b.op == IROper::AddF;
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Float, expr_a.b->float_value()+ expr_b.b->float_value()), IROper::AddF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 1 float" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����2 �������ϲ�
                <a1>-<v1>
                <p>-<v2>
                *������
                �����<a1>-<v1+v2>
            */
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::SubI && expr_b.op == IROper::SubI;
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Int, natural_add(expr_a.b->int_value(),expr_b.b->int_value())), IROper::SubI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 2" << std::endl;
#endif
                break;
            }
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::SubF && expr_b.op == IROper::SubF;
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Float, expr_a.b->float_value()+expr_b.b->float_value()), IROper::SubF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 2 float" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����3 �����Ӽ��ϲ�
                <a1>+<v1>
                <p>-<v2>
                *������
                �����<a1>+<v1-v2>
            */
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::AddI && expr_b.op == IROper::SubI;
            //if (cond) cond = in_int_range((int64_t)expr_a.b->int_value() - (int64_t)expr_b.b->int_value());
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Int, natural_sub(expr_a.b->int_value(), expr_b.b->int_value())), IROper::AddI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 3" << std::endl;
#endif
                break;
            }
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::AddF && expr_b.op == IROper::SubF;
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Float, expr_a.b->float_value() - expr_b.b->float_value()), IROper::AddF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 3 float" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����4 �����Ӽ��ϲ�
                <a1>-<v1>
                <p>+<v2>
                *������
                �����<a1>-<v1-v2>
            */
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::SubI && expr_b.op == IROper::AddI;
            //if (cond) cond = in_int_range((int64_t)expr_a.b->int_value() - (int64_t)expr_b.b->int_value());
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Int, natural_sub(expr_a.b->int_value(), expr_b.b->int_value())), IROper::SubI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 4" << std::endl;
#endif
                break;
            }
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::SubF && expr_b.op == IROper::AddF;
            //if (cond) cond = in_int_range((int64_t)expr_a.b->int_value() - (int64_t)expr_b.b->int_value());
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Float, expr_a.b->float_value()- expr_b.b->float_value()), IROper::SubF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 4 float" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����5 �Ӽ�����
                <a1>-<a2>
                <p>+<a2>
                ��
                <a1>+<a2>
                <p>-<a2>
                �����<a1>+<0>
            */
            cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::SubI && expr_b.op == IROper::AddI || 
                             expr_a.op == IROper::AddI && expr_b.op == IROper::SubI;
            if (cond) cond = symbol_equal(expr_a.b, expr_b.b);
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Int, 0), IROper::AddI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 5" << std::endl;
#endif
                break;
            }
            cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::SubF && expr_b.op == IROper::AddF ||
                             expr_a.op == IROper::AddF && expr_b.op == IROper::SubF;
            if (cond) cond = symbol_equal(expr_a.b, expr_b.b);
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Float, 0), IROper::AddF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 5" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����8 ��������
                <a1>-<a2>
                <a1>-<p>
                �����<a2>+<0>
            */
            cond = expr_b.b == prev->r();
            if (cond) cond = expr_a.op == IROper::SubI && expr_b.op == IROper::SubI;
            if (cond) cond = symbol_equal(expr_a.a, expr_b.a);
            if (cond) {
                succeed = true;
                res = Expr(expr_a.b, m_ir_sym_table->create_value(BasicType::Int, 0), IROper::AddI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 8" << std::endl;
#endif
                break;
            }
            cond = expr_b.b == prev->r();
            if (cond) cond = expr_a.op == IROper::SubF && expr_b.op == IROper::SubF;
            if (cond) cond = symbol_equal(expr_a.a, expr_b.a);
            if (cond) {
                succeed = true;
                res = Expr(expr_a.b, m_ir_sym_table->create_value(BasicType::Float, 0), IROper::AddF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 8 float" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����6 �����˳��ϲ�
                <a1>*<v1>
                <p>/<v2>
                V1��v2����������v1%v2==0��
                �����<a1>*<v1/v2>
            */
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.b->kind() == IRSymbolKind::Value;
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::MulI && expr_b.op == IROper::DivI;
            if (cond) cond = expr_a.b->int_value() % expr_b.b->int_value() == 0;
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Int, expr_a.b->int_value() / expr_b.b->int_value()), IROper::MulI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 6" << std::endl;
#endif
                break;
            }
            /*
                �ϲ�����7 �˼Ӻϲ�
                <a1>*<v1>
                <p>+<a1>
                ������
                �����<a1>*<v1+1>
            */
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = symbol_equal(expr_b.b,expr_a.a);
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::MulI && expr_b.op == IROper::AddI;
            //if (cond) cond = in_int_range((int64_t)expr_a.b->int_value() + 1LL);
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Int,natural_add(expr_a.b->int_value(),1)), IROper::MulI);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 7" << std::endl;
#endif
                break;
            }
            cond = expr_a.b->kind() == IRSymbolKind::Value;
            if (cond) cond = symbol_equal(expr_b.b, expr_a.a);
            if (cond) cond = expr_b.a == prev->r();
            if (cond) cond = expr_a.op == IROper::MulF && expr_b.op == IROper::AddF;
            //if (cond) cond = in_int_range((int64_t)expr_a.b->int_value() + 1LL);
            if (cond) {
                succeed = true;
                res = Expr(expr_a.a, m_ir_sym_table->create_value(BasicType::Float, expr_a.b->float_value()+ 1.0f), IROper::MulF);
#ifdef DEBUG_INFO_OPTIMIZER
                LinearIRManager::print_ir_instr(*prev);
                LinearIRManager::print_ir_instr(*now);
                std::cout << "merge rule 7 float" << std::endl;
#endif
                break;
            }
        }
        if (succeed)
            break;
    }
    IRInstr res_instr = IRInstr::create_return();
    if (succeed) {
        res_instr = IRInstr::create_binary_calc(res.op, now->r(), res.a, res.b);
        simplify_instr(res_instr);
    }
    return { succeed,res_instr };
}
void ConstantRecombination::check_merge(IRInstr* instr)
{
    if (instr->type() == IRType::BinaryCalc) {
        bool stay = false;
        if (instr->op() == IROper::AddI || (Ofast && instr->op() == IROper::AddF) ||
            instr->op() == IROper::MulI || (Ofast && instr->op() == IROper::MulF) ||
            instr->op() == IROper::SubI || (Ofast && instr->op() == IROper::SubF) ||
            instr->op() == IROper::DivI)
            stay = true;
        if (!stay)return;
    }
    else if (instr->type() == IRType::UnaryCalc) {
        bool stay = false;
        if (instr->op() == IROper::NegI || (Ofast && instr->op() == IROper::NegF))
            stay = true;
        if (!stay)return;
    }
    bool one_succeed = true;
    while(one_succeed){
        one_succeed = false;
        if (instr->a() != nullptr) {
            if (auto iter = m_sym_def_set.find(SymDefPair(instr->a())); iter != m_sym_def_set.end()) {
                if (auto [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
            }
        }
        if (instr->b() != nullptr) {
            if (auto iter = m_sym_def_set.find(SymDefPair(instr->b())); iter != m_sym_def_set.end()) {
                if (const auto& [succeed, merge_result] = try_merge(iter->def, instr); succeed) {
                    instr->copy(&merge_result);
                    one_succeed = true;
                }
            }
        }
    }
}
void ConstantRecombination::check_insert(IRInstr* instr)
{
    if (instr->type() == IRType::BinaryCalc) {
        bool stay = false;
        if (instr->op() == IROper::AddI || (Ofast && instr->op() == IROper::AddF) ||
            instr->op() == IROper::MulI || (Ofast && instr->op() == IROper::MulF) ||
            instr->op() == IROper::SubI || (Ofast && instr->op() == IROper::SubF) ||
            instr->op() == IROper::DivI)
            stay = true;
        if (!stay)return;
    }
    else if (instr->type() == IRType::UnaryCalc) {
        bool stay = false;
        if (instr->op() == IROper::NegI || (Ofast && instr->op() == IROper::NegF))
            stay = true;
        if (!stay)return;
    }
    if (is_non_pointer_temp_var(instr->r())) {
        //LinearIRManager::print_ir_instr(*instr);
        //�����Ӧ��ֵ���
        if (instr->a() != nullptr && instr->a()->is_global()||
            instr->b() != nullptr && instr->b()->is_global()) {
            if (!m_idfa->global_symbol_optimization_is_valid())
                return;
        }
#ifdef DEBUG_INFO_OPTIMIZER
        //LinearIRManager::print_ir_instr(*instr);
#endif
        m_sym_def_set.insert(SymDefPair(instr));
        //�ڱ�ָ���漰�Ĳ�������ά���������
        if (instr->a() != nullptr && (instr->a()->kind() == IRSymbolKind::Local || instr->a()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->a()->def_sym()->get_tag()].push_back(instr->r());
        else if (instr->a() != nullptr && instr->a()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->a()->get_tag()].push_back(instr->r());
        if (instr->b() != nullptr && (instr->b()->kind() == IRSymbolKind::Local || instr->b()->kind() == IRSymbolKind::Param))
            m_related_sym[instr->b()->def_sym()->get_tag()].push_back(instr->r());
        else if (instr->b() != nullptr && instr->b()->kind() == IRSymbolKind::Global)
            m_related_sym_glb[instr->b()->get_tag()].push_back(instr->r());
    }
}
void ConstantRecombination::work_assign(IRInstr* instr)
{
    //�ݲ���������
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //��鱾����Ƿ��ܹ���Ϊ����������ĺ�ѡ
    //check_insert(instr);
    //ɾ���������
    delete_related_instr(instr->r());
}
void ConstantRecombination::work_unary_calc(IRInstr* instr)
{
    //�ݲ���������
    if (is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //��鱾����Ƿ��ܺ�ǰһ��ϲ�
    check_merge(instr);
    //��鱾����Ƿ��ܹ���Ϊ����������ĺ�ѡ
    check_insert(instr);
    //ɾ���������
    delete_related_instr(instr->r());
}

void ConstantRecombination::work_binary_calc(IRInstr* instr)
{
    //�ݲ���������
    if (is_not_considered_type(instr->b()) ||
        is_not_considered_type(instr->r()) ||
        is_not_considered_type(instr->a())) {
        return;
    }
    //��鱾����Ƿ��ܺ�ǰһ��ϲ�
    check_merge(instr);
    //��鱾����Ƿ��ܹ���Ϊ����������ĺ�ѡ
    check_insert(instr);
    //ɾ���������
    delete_related_instr(instr->r());
}
void ConstantRecombination::work_ir_list(IRInstrList& program)
{
    m_sym_def_set.clear();
    for (auto& instr:program) {
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
        case IRType::Call:
            [[fallthrough]];
        case IRType::CallWithRet:
            //ɾ��������������Ӱ��ı�������Ϣ
            if (m_idfa->global_symbol_optimization_is_valid()) {
                if (!instr.a()->global_sym()->is_internal_function()) {
                    for (int i = 0; i < m_idfa->get_global_var_count(); ++i) {
                        IRSymbol* sym = m_idfa->get_global_var(i);
                        if (m_idfa->get_global_var_def(instr.a()->get_tag()).test(i)) {
                            for (auto sym : m_related_sym_glb[i])
                                m_sym_def_set.erase(sym);
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}
void ConstantRecombination::work_cfg(IRUnit* unit)
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
bool ConstantRecombination::is_non_pointer_temp_var(IRSymbol* sym)
{
    return sym != nullptr && sym->kind() == IRSymbolKind::Temp &&sym->array_length() == IRArrayLength::IR_NONE_ARRAY;
}
void ConstantRecombination::run()
{
    std::cout << "Running pass: Constant Recombination " << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    if (m_idfa->global_symbol_optimization_is_valid()) {
        m_idfa->compute_side_effect_info();
        m_related_sym_glb.clear();
        m_related_sym_glb.resize(m_idfa->get_global_var_count());
    }
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef) {
            m_temp_count = 0;
            m_local_param_count = 0;
            m_related_sym.clear();
            //��l��p�������
            for (auto& def_instr : unit.get_definations()) {
                if (def_instr.a()->kind() == IRSymbolKind::Local || def_instr.a()->kind() == IRSymbolKind::Param)
                    if (def_instr.a()->def_sym()->array_length() == IRArrayLength::IR_NONE_ARRAY) {
                        def_instr.a()->def_sym()->set_tag(m_local_param_count);
                        m_related_sym.push_back({});
                        m_local_param_count++;
                    }
            }
            work_cfg(&unit);
        }
}
