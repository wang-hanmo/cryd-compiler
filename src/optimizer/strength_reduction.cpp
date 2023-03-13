#include <strength_reduction.h>
#include <ir_define.h>
#include <set>
#include <queue>
#include <ir_instr.h>
#include <ir_symbol_table.h>
#include <cmath>
#include <iterator>
#include <algorithm>

int reduction_count = 0;
//判断一个正数是否为2的整数幂
bool StrengthReduction::is_power_of_2(int num)
{
    if (num < 0)return false;
    return (num & (num - 1)) ? false : true;
}
//求一个2的整数幂次方的数对2取模的值
int StrengthReduction::log2_for_int_pow_of_2(int num)
{
    int pow = -1;
    while (num) {
        num >>= 1;
        pow++;
    }
    return pow;
}
IRInstrList::iterator StrengthReduction::const_mul_weaken(IRInstrList& instr_list, IRInstrList::iterator instr)
{
    IRInstrList insertion_list;
    int multiplier = instr->b()->value().int_value;
    //寻找优化路径(第一步只考虑转化为一条指令的优化)
    //不考虑代数*1和*0，已经在代数化简中做过了
    if (multiplier > 1) {
        //乘数是2的n次方
        if (is_power_of_2(multiplier)) {
            int shift = log2_for_int_pow_of_2(multiplier);
            instr->rebind_b(instr->a());
            instr->rebind_a(nullptr);
            instr->rebind_c(m_ir_sym_table->create_value(BasicType::Int, shift));
            instr->reset_sop(IRShiftOper::LSL);
            instr->reset_op(IROper::ShiftI);
            instr->rewrite_type(IRType::TernaryCalc);
        }//乘数是2的n次方-1
        else if (multiplier == 2147483647 || is_power_of_2(multiplier + 1)) {
            int shift = multiplier == 2147483647 ? 31 : log2_for_int_pow_of_2(multiplier + 1);
            instr->rebind_b(instr->a());
            instr->rebind_c(m_ir_sym_table->create_value(BasicType::Int, shift));
            instr->reset_sop(IRShiftOper::LSL);
            instr->reset_op(IROper::ShiftRsbI);
            instr->rewrite_type(IRType::TernaryCalc);
        }//乘数是2的n次方+1
        else if (is_power_of_2(multiplier - 1)) {
            int shift = log2_for_int_pow_of_2(multiplier - 1);
            instr->rebind_b(instr->a());
            instr->rebind_c(m_ir_sym_table->create_value(BasicType::Int, shift));
            instr->reset_sop(IRShiftOper::LSL);
            instr->reset_op(IROper::ShiftAddI);
            instr->rewrite_type(IRType::TernaryCalc);
        } else {
            auto r = instr->r();
            auto a = instr->a();
            auto b = instr->b();
            // 枚举2^n-1
            for (int i = 2; i < 32; ++i) {
                IRSymbol* temp = nullptr;
                int factor1 = pow(2, i) - 1;
                if (multiplier % factor1 == 0) {
                    int factor2 = multiplier / factor1;
                    // std::cout << "factor1:" << factor1 << " factor2:" << factor2 << std::endl;
                    if (is_power_of_2(factor2)) {
                        // r=a*b=a*(2^n-1)*(2^m) = (1)r=a<<m;(2)r=r<<n-r
                        int shift = log2_for_int_pow_of_2(factor2);
                        auto c = m_ir_sym_table->create_value(BasicType::Int, shift);
                        temp = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index);
                        insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftI, IRShiftOper::LSL, temp, nullptr, a, c));
                    } else if (factor2 == 2147483647 || is_power_of_2(factor2 + 1)) {
                        // r=a*b=a*(2^n-1)*(2^m-1) = (1)r=a<<m-a;(2)r=r<<n-r
                        int shift = factor2 == 2147483647 ? 31 : log2_for_int_pow_of_2(factor2 + 1);
                        auto c = m_ir_sym_table->create_value(BasicType::Int, shift);
                        temp = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index);
                        insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftRsbI, IRShiftOper::LSL, temp, a, a, c));
                    } else if (is_power_of_2(factor2 - 1)) {
                        int shift = log2_for_int_pow_of_2(factor2 - 1);
                        auto c = m_ir_sym_table->create_value(BasicType::Int, shift);
                        temp = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index);
                        insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftAddI, IRShiftOper::LSL, temp, a, a, c));
                    } else
                        continue;
                } else
                    continue;
                std::cerr << "reduction count:" << ++reduction_count << std::endl;
                auto c = m_ir_sym_table->create_value(BasicType::Int, i);
                insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftRsbI, IRShiftOper::LSL, r, temp, temp, c));
                instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
                return instr_list.erase(instr);
            }
            // 枚举2^n+1
            for (int i = 1; i < 31; ++i) {
                IRSymbol* temp = nullptr;
                int factor1 = pow(2, i) + 1;
                if (multiplier % factor1 == 0) {
                    int factor2 = multiplier / factor1;
                    if (is_power_of_2(factor2)) {
                        // r=a*b=a*(2^n+1)*(2^m) = (1)r=a<<m;(2)r=r<<n+r
                        int shift = log2_for_int_pow_of_2(factor2);
                        auto c = m_ir_sym_table->create_value(BasicType::Int, shift);
                        temp = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index);
                        insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftI, IRShiftOper::LSL, temp, nullptr, a, c));
                    } else if (factor2 == 2147483647 || is_power_of_2(factor2 + 1)) {
                        // r=a*b=a*(2^n-1)*(2^m-1) = (1)r=a<<m-a;(2)r=r<<n+r
                        int shift = factor2 == 2147483647 ? 31 : log2_for_int_pow_of_2(factor2 + 1);
                        auto c = m_ir_sym_table->create_value(BasicType::Int, shift);
                        temp = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index);
                        insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftRsbI, IRShiftOper::LSL, temp, a, a, c));
                    } else if (is_power_of_2(factor2 - 1)) {
                        int shift = log2_for_int_pow_of_2(factor2 - 1);
                        auto c = m_ir_sym_table->create_value(BasicType::Int, shift);
                        temp = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index);
                        insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftAddI, IRShiftOper::LSL, temp, a, a, c));
                    } else
                        continue;
                } else
                    continue;
                std::cerr << "reduction count:" << ++reduction_count << std::endl;
                auto c = m_ir_sym_table->create_value(BasicType::Int, i);
                insertion_list.push_back(IRInstr::create_ternary_calc(IROper::ShiftAddI, IRShiftOper::LSL, r, temp, temp, c));
                instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
                return instr_list.erase(instr);
            }
        }
    }
    return ++instr;
}

std::tuple<int, int, int> StrengthReduction::choose_multiplier(int d, int N)
{
    // n/d = n*m/2^(N+l)
    assert(d >= 1);
    int l = ceil(log2((double)d - 0.5));
    int sh_post = l;
    // 2^(N+l)/d <= m <= (2^(N+l)+2^l)/d
    uint64_t m_l = (((uint64_t)1) << (N + l)) / d;
    uint64_t m_h = ((((uint64_t)1) << (N + l)) + (((uint64_t)1) << (l))) / d;
    while ((m_l / 2 < m_h / 2) && (sh_post > 1)) {
        m_l /= 2;
        m_h /= 2;
        sh_post--;
    }
    sh_post--;
    return std::make_tuple((int)(m_h), sh_post, l);
}

IRInstrList::iterator StrengthReduction::const_div_weaken(IRInstrList& instr_list, IRInstrList::iterator instr)
{
    IRInstrList insertion_list;
    int divisor = instr->b()->value().int_value;
    auto res_oprand = instr->r();
    auto dividend = instr->a();
    const int N = 32;
    int d = abs(divisor);
    int m, sh_post, l;
    std::tie(m, sh_post, l) = choose_multiplier(d, N - 1);
    //除以2的n次方的优化
    if (d == 2) {
        IRSymbol* temp_oprand = nullptr;
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftAddI, IRShiftOper::LSR,
            temp_oprand = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            dividend,
            dividend,
            m_ir_sym_table->create_value(BasicType::Int, 31)
        ));
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftI, IRShiftOper::ASR,
            res_oprand,
            nullptr,
            temp_oprand,
            m_ir_sym_table->create_value(BasicType::Int, 1)
        ));
    }else if (is_power_of_2(d)){
        int n = log2_for_int_pow_of_2(d);
        IRSymbol* temp_oprand = nullptr;
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftI,
            IRShiftOper::ASR,
            temp_oprand = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            nullptr,
            dividend,
            m_ir_sym_table->create_value(BasicType::Int, 31)
        ));
        IRSymbol* temp_oprand2 = nullptr;
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftAddI,
            IRShiftOper::LSR,
            temp_oprand2 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            dividend,
            temp_oprand,
            m_ir_sym_table->create_value(BasicType::Int, 32 - n)
        ));
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftI,
            IRShiftOper::ASR,
            res_oprand,
            0,
            temp_oprand2,
            m_ir_sym_table->create_value(BasicType::Int,n)
        ));
    } else if (m >= 0) {
        // q = SRA(MULSH(m, n), sh_post) - XSIGN(n);
        IRSymbol* temp_oprand = nullptr;
        insertion_list.push_back(IRInstr::create_assign(
            temp_oprand = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            m_ir_sym_table->create_value(BasicType::Int, m)
        ));
        IRSymbol* temp_oprand2 = nullptr;
        insertion_list.push_back(IRInstr::create_binary_calc(
            IROper::SmmulI,
            temp_oprand2 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            temp_oprand,
            dividend
        ));
        IRSymbol* temp_oprand3 = nullptr;
        if (sh_post != 0) {
            insertion_list.push_back(IRInstr::create_ternary_calc(
                IROper::ShiftI,
                IRShiftOper::ASR,
                temp_oprand3 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
                nullptr,
                temp_oprand2,
                m_ir_sym_table->create_value(BasicType::Int, sh_post)
            ));
        } else {
            insertion_list.push_back(IRInstr::create_assign(
                temp_oprand3 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
                temp_oprand2
            ));
        }
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftAddI,
            IRShiftOper::LSR,
            res_oprand,
            temp_oprand3,
            dividend,
            m_ir_sym_table->create_value(BasicType::Int, 31)
        ));
    } else {
        // q = SRA(n + MULSH(m - 2^N , n), sh_post) - XSIGN(n); (这里的m为无符号数)
        IRSymbol* temp_oprand = nullptr;
        insertion_list.push_back(IRInstr::create_assign(
            temp_oprand = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            m_ir_sym_table->create_value(BasicType::Int, m)
        ));
        IRSymbol* temp_oprand2 = nullptr;
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::SignedLargeMulAddI,
            IRShiftOper::Null,
            temp_oprand2 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            dividend,
            temp_oprand,
            dividend
        ));
        IRSymbol* temp_oprand3 = nullptr;
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftI,
            IRShiftOper::ASR,
            temp_oprand3 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            nullptr,
            temp_oprand2,
            m_ir_sym_table->create_value(BasicType::Int, sh_post)
        ));
        insertion_list.push_back(IRInstr::create_ternary_calc(
            IROper::ShiftAddI,
            IRShiftOper::LSR,
            res_oprand,
            temp_oprand3,
            dividend,
            m_ir_sym_table->create_value(BasicType::Int, 31)
        ));

    }
    if (divisor < 0) {
        insertion_list.push_back(IRInstr::create_binary_calc(
            IROper::RsbI,
            res_oprand,
            res_oprand,
            m_ir_sym_table->create_value(BasicType::Int, 0)
        ));
    }
    if (insertion_list.empty()) {
        return ++instr;
    } else {
        instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
        return instr_list.erase(instr);
    }
}
IRInstrList::iterator StrengthReduction::const_mod_weaken(IRInstrList& instr_list, IRInstrList::iterator instr)
{
    IRInstrList insertion_list;
    int divisor = instr->b()->value().int_value;
    auto res_oprand = instr->r();
    auto dividend = instr->a();
    //模2的n次方的优化
    if (divisor >= 1) {
        if (is_power_of_2(divisor)) {
            int n = log2_for_int_pow_of_2(divisor);
            IRSymbol* temp_oprand = nullptr;
            insertion_list.push_back(IRInstr::create_ternary_calc(
                IROper::ShiftI,
                IRShiftOper::ASR,
                temp_oprand = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
                nullptr,
                dividend,
                m_ir_sym_table->create_value(BasicType::Int, 31)
            ));
            IRSymbol* temp_oprand2 = nullptr;
            insertion_list.push_back(IRInstr::create_ternary_calc(
                IROper::ShiftAddI,
                IRShiftOper::LSR,
                temp_oprand2 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
                dividend,
                temp_oprand,
                m_ir_sym_table->create_value(BasicType::Int, 32 - n)
            ));
            IRSymbol* temp_oprand3 = nullptr;
            insertion_list.push_back(IRInstr::create_binary_calc(
                IROper::BitAnd,
                temp_oprand3 = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
                temp_oprand2,
                m_ir_sym_table->create_value(BasicType::Int, ~(divisor - 1))
            ));
            insertion_list.push_back(IRInstr::create_binary_calc(
                IROper::SubI,
                res_oprand,
                dividend,
                temp_oprand3
            ));
        }
    }
    if (insertion_list.empty()) {
        return ++instr;
    } else {
        instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
        return instr_list.erase(instr);
    }
}
IRInstrList::iterator StrengthReduction::const_mla_weaken(IRInstrList& instr_list, IRInstrList::iterator instr)
{
    IRInstrList insertion_list;
    int multiplier = instr->c()->value().int_value;
    //寻找优化路径(第一步只考虑转化为一条指令的优化)
    //不考虑代数*1和*0，已经在代数化简中做过了
    bool expand = false;
    if (multiplier > 1) {
        //乘数是2的n次方
        if (is_power_of_2(multiplier)) {
            int shift = log2_for_int_pow_of_2(multiplier);
            instr->rebind_c(m_ir_sym_table->create_value(BasicType::Int, shift));
            instr->reset_sop(IRShiftOper::LSL);
            instr->reset_op(IROper::ShiftAddI);
        }//乘数是2的n次方-1
        else if (multiplier == 2147483647 || is_power_of_2(multiplier + 1)) {
            //展开成乘法和加法
            expand = true;
        }//乘数是2的n次方+1
        else if (is_power_of_2(multiplier - 1)) {
            expand = true;
        }
    }
    if (expand) {
        IRSymbol* temp_oprand = nullptr;
        insertion_list.push_back(IRInstr::create_binary_calc(
            IROper::MulI,
            temp_oprand = m_ir_sym_table->create_temp(BasicType::Int, ++m_max_temp_index),
            instr->b(),
            instr->c()
        ));
        insertion_list.push_back(IRInstr::create_binary_calc(
            IROper::AddI,
            instr->r(),
            instr->a(),
            temp_oprand
        ));
        instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
        instr = instr_list.erase(instr);
        --instr;
        return --instr;
    }
    if (insertion_list.empty()) {
        return ++instr;
    }
    else {
        instr_list.insert(instr, insertion_list.begin(), insertion_list.end());
        return instr_list.erase(instr);
    }
}
void StrengthReduction::work_ir_list(IRInstrList& program)
{
    for (auto instr = program.begin(); instr != program.end();) {
        if (instr->type() == IRType::BinaryCalc) {
            switch (instr->op()) {
            case IROper::MulI:
                //将常数交换到b位置
                if (instr->a()->is_value())
                    instr->swap_a_and_b();
                //对整数常数乘法做优化
                if (instr->b()->is_value())
                    instr = const_mul_weaken(program, instr);
                else ++instr;
                break;
            case IROper::DivI: {
                //对除法做优化
                if (instr->b()->is_value())
                    instr = const_div_weaken(program, instr);
                else ++instr;
                break;
            }case IROper::ModI: {
                //对模运算做优化
                if (instr->b()->is_value())
                    instr = const_mod_weaken(program, instr);
                else ++instr;
                break;
            }
            default:
                ++instr;
                break;
            }
        }
        else if (instr->type() == IRType::TernaryCalc) {
            if (instr->op() == IROper::MulAddI) {
                if (instr->b()->is_value())
                    instr->swap_b_and_c();
                if (instr->c()->is_value())
                    instr = const_mla_weaken(program, instr);
                else ++instr;
            }
            else ++instr;
        }
        else ++instr;
    }
}
void StrengthReduction::work_cfg(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while(!q.empty()){
        IRBlock* now=q.front();
        q.pop();
        work_ir_list(now->get_instr_list());
        for(int k=1;k>=0;--k){
            if(now->get_succ(k)!=nullptr&& visited.find(now->get_succ(k))==visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}
void StrengthReduction::run()
{
    std::cout << "Running pass: Strength Reduction " << std::endl;
    if (m_cfg == nullptr ||m_ir_sym_table==nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    for (auto& unit : *m_cfg){
         if (unit.get_type() == IRUnitType::FuncDef){
             m_max_temp_index=1+unit.find_max_index_for_tlp_var();
             work_cfg(&unit);
         }
    }
}
