#include<ir_instr.h>
#include<cassert>
#include<memory>
using namespace std;
int i_no = 0;
IRInstr::IRInstr(IRType i_type, IRSymbol* i_a, IRSymbol* i_b, IRSymbol* i_r, IROper i_op,
    IRJumpTarget i_jmp_tar_false, IRJumpTarget i_jmp_tar_true, IRShiftOper i_sop, IRSymbol* i_c, const std::vector<CaseLabel>& i_case_vec) :
    m_type(i_type), m_r(i_r), m_a(i_a), m_b(i_b), m_op(i_op), m_jmp_tar_false(i_jmp_tar_false), m_jmp_tar_true(i_jmp_tar_true), m_no(i_no++),
    m_sop(i_sop), m_c(i_c)
{
}
IRInstr IRInstr::create_phi_func(IRSymbol* r,IRSymbol* phi_func)
{
    return IRInstr(IRType::PhiFunc, phi_func, nullptr, r);
}
IRInstr IRInstr::create_label(int lbl0)
{
    return IRInstr(IRType::Label,nullptr,nullptr,nullptr,IROper::Null,lbl0);
}

IRInstr IRInstr::create_binary_calc(IROper op, IRSymbol* r, IRSymbol* a, IRSymbol* b)
{
    return IRInstr(IRType::BinaryCalc,a,b,r,op);
}

IRInstr IRInstr::create_unary_calc(IROper op, IRSymbol* r, IRSymbol* a)
{
    return IRInstr(IRType::UnaryCalc,a,nullptr,r,op);
}

IRInstr IRInstr::create_assign(IRSymbol* r, IRSymbol* a)
{
    return IRInstr(IRType::Assign,a,nullptr,r);
}

IRInstr IRInstr::create_array_load(IRSymbol* r, IRSymbol* base, IRSymbol* offset)
{
    return IRInstr(IRType::ArrayLoad,base,offset,r);
}

IRInstr IRInstr::create_array_store(IRSymbol* base, IRSymbol* offset, IRSymbol* src)
{
    return IRInstr(IRType::ArrayStore,offset,src,base);
}

IRInstr IRInstr::create_goto(int lbl0)
{
    return IRInstr(IRType::Goto, nullptr, nullptr, nullptr, IROper::Null, lbl0);
}

IRInstr IRInstr::create_cond_goto(IRSymbol* cond, int lbl1, int lbl0)
{
    return IRInstr(IRType::CondGoto, cond, nullptr, nullptr, IROper::Null, lbl0,lbl1);
}
IRInstr IRInstr::create_switch(IRSymbol* a, const std::vector<CaseLabel>& case_vec)
{
    return IRInstr(IRType::Switch, a, nullptr, nullptr, IROper::Null, -1, -1,IRShiftOper::Null,nullptr,case_vec);
}
IRInstr IRInstr::create_call(IRSymbol* func, IRSymbol* param_count)
{
    return IRInstr(IRType::Call, func,param_count);
}
IRInstr IRInstr::create_call_with_ret(IRSymbol* r, IRSymbol* func, IRSymbol* param_count)
{
    return IRInstr(IRType::CallWithRet,func,param_count,r);
}
IRInstr IRInstr::create_return()
{
    return IRInstr(IRType::Return);
}

IRInstr IRInstr::create_value_return(IRSymbol* return_value)
{
    return IRInstr(IRType::ValReturn,return_value);
}

IRInstr IRInstr::create_func_def(IRSymbol* func, IRSymbol* param_count)
{
    return IRInstr(IRType::FuncDef,func,param_count);
}

IRInstr IRInstr::create_func_end()
{
    return IRInstr(IRType::FuncEnd);
}

IRInstr IRInstr::create_f_param(IRSymbol* a)
{
    return IRInstr(IRType::FParam,a);
}

IRInstr IRInstr::create_r_param(IRSymbol* a)
{
    return IRInstr(IRType::RParam,a);
}

IRInstr IRInstr::create_local_decl(IRSymbol* a)
{
    return IRInstr(IRType::LocalDecl,a);
}

IRInstr IRInstr::create_global_decl(IRSymbol* a)
{
    return IRInstr(IRType::GlobalDecl,a);
}

IRInstr IRInstr::create_load(IRSymbol* r, IRSymbol* a)
{
    return IRInstr(IRType::Load,a,nullptr,r);
}
IRInstr IRInstr::create_store(IRSymbol* r, IRSymbol* a)
{
    return IRInstr(IRType::Store,a,nullptr,r);
}

IRInstr IRInstr::create_block_goto()
{
    return IRInstr(IRType::BlockGoto);
}

IRInstr IRInstr::create_block_cond_goto(IRSymbol* cond)
{
    return IRInstr(IRType::BlockCondGoto,cond);
}
IRInstr IRInstr::create_binary_calc(IROper op, IRSymbol* r, IRSymbol* c, IRSymbol* a, IRSymbol* b)
{
    return IRInstr(IRType::BinaryCalc, a, b, r, op,-1,-1,IRShiftOper::Null,c);
}
IRInstr IRInstr::create_ternary_calc(IROper op, IRShiftOper sop, IRSymbol* r, IRSymbol* a, IRSymbol* b, IRSymbol* c)
{
    return IRInstr(IRType::TernaryCalc, a, b, r, op, -1, -1, sop, c);
}
IRInstr IRInstr::create_block_binary_goto(IROper op, IRSymbol* r, IRSymbol* a, IRSymbol* b)
{
    return IRInstr(IRType::BlockBinaryGoto, a, b, r, op);
}
IRInstr IRInstr::create_block_unary_goto(IROper op, IRSymbol* r, IRSymbol* a)
{
    return IRInstr(IRType::BlockUnaryGoto, a, nullptr, r, op);
}
//类似于数组版的phi函数，但是没有参数
IRInstr IRInstr::create_mem_converge_mark(IRSymbol* a)
{
    return IRInstr(IRType::MemoryConvergeMark, a);
}
void IRInstr::rewrite_type(IRType new_type) {
    m_type = new_type;
    switch (m_type) {
    case IRType::BinaryCalc:
        if (m_op != IROper::SignedLargeMulI)
            m_c = nullptr;
        break;
    case IRType::UnaryCalc:
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::Assign:
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::Load:
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::Store:
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::RParam:
        m_r = nullptr;
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::ArrayStore:
        m_c = nullptr;
        break;
    case IRType::ArrayLoad:
        m_c = nullptr;
        break;
    case IRType::Call:
        m_r = nullptr;
        m_c = nullptr;
        break;
    case IRType::CallWithRet:
        m_c = nullptr;
        break;
    case IRType::Return:
        m_a = nullptr;
        m_b = nullptr;
        m_r = nullptr;
        m_c = nullptr;
        break;
    case IRType::ValReturn:
        m_r = nullptr;
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::BlockGoto:
        m_r = nullptr;
        m_a = nullptr;
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::BlockCondGoto:
        m_r = nullptr;
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::BlockUnaryGoto:
        m_r = nullptr;
        m_b = nullptr;
        m_c = nullptr;
        break;
    case IRType::BlockBinaryGoto:
        m_r = nullptr;
        m_c = nullptr;
        break;
    case IRType::PhiFunc:
        break;
    case IRType::TernaryCalc:
        if (m_op == IROper::ShiftI)
            m_a = nullptr;
        break;
    default:
        assert(0);
        break;
    }
};
void IRInstr::copy(const IRInstr* target)
{
    m_r = target->m_r;
    m_c = target->m_c;
    m_a = target->m_a;
    m_b = target->m_b;
    m_op = target->m_op;
    m_sop = target->m_sop;
    m_type = target->m_type;
    m_jmp_tar_false = target->m_jmp_tar_false;
    m_jmp_tar_true = target->m_jmp_tar_true;
}
std::vector<IRSymbol*> IRInstr::rvalues()const
{
    switch (m_type) {
    case IRType::BinaryCalc:
        return { m_a,m_b };
    case IRType::UnaryCalc:
        return { m_a };
    case IRType::Assign:
        return { m_a };
    case IRType::Load:
        return { m_a };
    case IRType::Store:
        return { m_r,m_a };
    case IRType::RParam:
        return { m_a };
    case IRType::ArrayStore:
        return { m_a,m_b,m_r };
    case IRType::ArrayLoad:
        return { m_a,m_b };
    case IRType::Call:
        return { m_a };
    case IRType::CallWithRet:
        return { m_a };
    case IRType::Return:
        return {};
    case IRType::ValReturn:
        return { m_a };
    case IRType::BlockGoto:
        return {};
    case IRType::BlockCondGoto:
        return { m_a };
    case IRType::BlockUnaryGoto:
        return { m_a };
    case IRType::BlockBinaryGoto:
        return { m_a , m_b};
    case IRType::PhiFunc:
        return { m_a };
    case IRType::TernaryCalc:
        if (m_op == IROper::ShiftI)
            return { m_b,m_c };
        return { m_a,m_b,m_c };
    default:
        assert(0);
        break;
    }
}
std::vector<IRSymbol*> IRInstr::lvalues()const
{
    switch (m_type) {
    case IRType::BinaryCalc:
        if (m_op == IROper::SignedLargeMulI)
            return{ m_r,m_c };
        return { m_r };
    case IRType::UnaryCalc:
        return { m_r };
    case IRType::Assign:
        return { m_r };
    case IRType::Load:
        return { m_r };
    case IRType::Store:
        return {};
    case IRType::RParam:
        return {};
    case IRType::ArrayStore:
        return {};
    case IRType::ArrayLoad:
        return { m_r };
    case IRType::Call:
        return {};
    case IRType::CallWithRet:
        return { m_r };
    case IRType::Return:
        return {};
    case IRType::ValReturn:
        return {};
    case IRType::BlockGoto:
        return {};
    case IRType::BlockCondGoto:
        return {};
    case IRType::BlockUnaryGoto:
        return {};
    case IRType::BlockBinaryGoto:
        return {};
    case IRType::PhiFunc:
        return { m_r };
    case IRType::TernaryCalc:
        return { m_r };
    default:
        assert(0);
        break;
    }
}