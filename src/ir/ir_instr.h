#pragma once
#include <vector>
#include <list>
#include <iostream>
#include <ir_symbol_table.h>
#include <type_define.h>
#include <tuple>
#include <algorithm>
#include <cassert>
struct CaseLabel
{
    IRJumpTarget jmp_tar;   //Linear IR
    BasicValue cond;        //CFG
    bool is_default;
    CaseLabel() {}
    CaseLabel(IRJumpTarget jmp_tar_, const BasicValue& cond_,bool is_default_):
        jmp_tar(jmp_tar_),cond(cond_),is_default(is_default_) {

    }
};
class IRInstr
{
private:
    IRType m_type;                                  //指令类型
    IRJumpTarget m_jmp_tar_false,m_jmp_tar_true;    //跳转目标
    //std::vector<CaseLabel> m_case_vec;              //switch语句的目标
    IROper m_op;        	                        //具体操作类型
    IRSymbol* m_r, * m_a, * m_b;                    //MIR三地址码的三个操作数
    int m_no;                                       //ir指令序号
    IRShiftOper m_sop{ IRShiftOper::Null };         //LIR移位运算符
    IRSymbol* m_c{nullptr};                         //LIR操作数c
    IRInstr(IRType i_type,IRSymbol* i_a = nullptr,IRSymbol* i_b = nullptr,IRSymbol* i_r=nullptr,IROper i_op=IROper::Null,
        IRJumpTarget i_jmp_tar_false = -1, IRJumpTarget i_jmp_tar_true = -1, IRShiftOper i_sop = IRShiftOper::Null, IRSymbol* i_c = nullptr, const std::vector<CaseLabel>& i_case_vec = {});
public:
    //MIR工厂函数
    static IRInstr create_label       (int lbl0);
    static IRInstr create_binary_calc (IROper op, IRSymbol* r, IRSymbol* a, IRSymbol* b);
    static IRInstr create_unary_calc  (IROper op, IRSymbol* r, IRSymbol* a);
    static IRInstr create_assign      (IRSymbol* r, IRSymbol* a);
    static IRInstr create_array_load  (IRSymbol* r, IRSymbol* base, IRSymbol* offset);
    static IRInstr create_array_store (IRSymbol* base, IRSymbol* offset, IRSymbol* src);
    static IRInstr create_goto        (int lbl0);
    static IRInstr create_cond_goto   (IRSymbol* cond,int lbl1,int lbl0);
    static IRInstr create_switch      (IRSymbol* a,const std::vector<CaseLabel>& case_vec);
    static IRInstr create_call        (IRSymbol* func, IRSymbol* param_count);
    static IRInstr create_call_with_ret(IRSymbol* r, IRSymbol* func, IRSymbol* param_count);
    static IRInstr create_return      ();
    static IRInstr create_value_return(IRSymbol* return_value);
    static IRInstr create_func_def    (IRSymbol* func, IRSymbol* param_count);
    static IRInstr create_func_end    ();
    static IRInstr create_f_param     (IRSymbol* a);
    static IRInstr create_r_param     (IRSymbol* a);
    static IRInstr create_local_decl  (IRSymbol* a);//size=0表示变量，size>=1表示数组
    static IRInstr create_global_decl (IRSymbol* a);
    static IRInstr create_block_goto        ();
    static IRInstr create_block_cond_goto   (IRSymbol* cond);
    //static IRInstr create_block_switch(const std::vector<CaseLabel>& case_vec);
    static IRInstr create_phi_func    (IRSymbol* r,IRSymbol* phi_func);//SSA形式中的phi函数
    //优化里可能会用到的的标记指令
    static IRInstr create_mem_converge_mark(IRSymbol* a);//类似于数组版的phi函数，但是没有参数
    //LIR工厂函数
    static IRInstr create_binary_calc (IROper op, IRSymbol* r, IRSymbol* c, IRSymbol* a, IRSymbol* b);
    static IRInstr create_ternary_calc(IROper op, IRShiftOper sop, IRSymbol* r,IRSymbol* a, IRSymbol* b, IRSymbol* c);
    static IRInstr create_block_binary_goto(IROper op, IRSymbol* r, IRSymbol* a, IRSymbol* b);
    static IRInstr create_block_unary_goto(IROper op, IRSymbol* r, IRSymbol* a);
    static IRInstr create_load        (IRSymbol* r, IRSymbol* a);
    static IRInstr create_store       (IRSymbol* r, IRSymbol* a);
    
    IRType type()const          {return m_type;}
    int lbl0()const             {return m_jmp_tar_false;}
    int lbl1()const             {return m_jmp_tar_true;}
    IROper op()const            {return m_op;}
    IRShiftOper sop()const      { return m_sop; }
    IRSymbol* c()const          { return m_c; }
    IRSymbol* a()const          { return m_a;}
    IRSymbol* b()const          { return m_b;}
    IRSymbol* r()const          { return m_r; }
    void swap_a_and_b()         { std::swap(m_a, m_b); }
    void swap_b_and_c()         { std::swap(m_b, m_c); }
    int no()const               { return m_no;}
    void set_no(int i_no)           {m_no = i_no;}
    //rebind函数,替换指针指向的符号
    void rebind_a(IRSymbol* new_a) { m_a = new_a; };
    void rebind_b(IRSymbol* new_b) { m_b = new_b; };
    void rebind_c(IRSymbol* new_c) { m_c = new_c; };
    void rebind_r(IRSymbol* new_r) { m_r = new_r; };
    void reset_op(IROper op)   {m_op = op;}
    void reset_sop(IRShiftOper sop) { m_sop = sop; }
    std::vector<IRSymbol*> rvalues()const;
    std::vector<IRSymbol*> lvalues()const;
    //在修改一条指令时，请将用不到的操作数置为nullptr。本函数会自动维护这一性质，请在修改指令时最后再调用该函数
    void rewrite_type(IRType new_type);
    void copy(const IRInstr* target);//复制一条特定的指令
};
using IRInstrList=std::list<IRInstr>;
