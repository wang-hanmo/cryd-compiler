#include <constant_fold_and_algebraic_simplification.h>
#include <string>
#include <iostream>
#include <cassert>
#include <set>
void ConstantFoldAndAlgebraicSimplification::work_binary_calc(IRInstr& instr)
{
    bool a_is_value = instr.a()->kind() == IRSymbolKind::Value;
    bool b_is_value = instr.b()->kind() == IRSymbolKind::Value;
    int int_a = instr.a()->value().int_value;
    float float_a = instr.a()->value().float_value;
    int int_b = instr.b()->value().int_value;
    float float_b = instr.b()->value().float_value;
    switch (instr.op()) {
    case IROper::AddI:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a + int_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->int_value() == 0) {//代数化简,a为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->int_value() == 0)   //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (instr.a() == instr.b()) {//代数化简,a=b则化简为a*2，后续再做进一步优化
            instr.rebind_b(m_ir_sym_table->create_value(BasicType::Int, 2));
            instr.reset_op(IROper::MulI);
        }
        break;
    case IROper::AddF:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_float_value(float_a + float_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->float_value() == 0) {//代数化简,a为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->float_value() == 0)   //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        break;
    case IROper::SubI:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a - int_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->int_value() == 0) //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (a_is_value && instr.a()->int_value() == 0) { //代数化简,a为0则结果等于-b
            instr.rebind_a(instr.b());
            instr.reset_op(IROper::NegI);
            instr.rewrite_type(IRType::UnaryCalc);
        }
        else if (instr.a()==instr.b()) { //代数化简,a=b则结果为0
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Int,0));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::SubF:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_float_value(float_a - float_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->float_value() == 0) //代数化简,b为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (a_is_value && instr.a()->float_value() == 0) { //代数化简,a为0则结果等于-b
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
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a * int_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->int_value() == 0)//代数化简，a为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (b_is_value && instr.b()->int_value() == 0) {//代数化简，b为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->int_value() == 1) {//代数化简，b为1则结果等于a
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->int_value() == 1) {//代数化简，a为1则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::MulF:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_float_value(float_a * float_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->float_value() == 0)//代数化简，a为0则结果等于a
            instr.rewrite_type(IRType::Assign);
        else if (b_is_value && instr.b()->float_value() == 0) {//代数化简，b为0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->float_value() == 1.0f) {//代数化简，b为1.0则结果等于a
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->float_value() == 1.0f) {//代数化简，a为1.0则结果等于b
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::DivI:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a / int_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->int_value() == 0){//代数化简，a为0则结果等于0
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->int_value() == 1) {//代数化简，b为1则结果等于a
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a()==instr.b()) {//代数化简，a=b则结果等于1
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Int, 1));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::DivF:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_float_value(float_a / float_b);
            instr.rewrite_type(IRType::Assign);
        }
        else if (a_is_value && instr.a()->float_value() == 0) {//代数化简，a为0则结果等于0
            instr.rewrite_type(IRType::Assign);
        }
        else if (b_is_value && instr.b()->float_value() == 1.0f) {//代数化简，b为1则结果等于a
            instr.rewrite_type(IRType::Assign);
        }
        else if (instr.a() == instr.b()) {//代数化简，a=b则结果等于1
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Float, 1.0f));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::ModI:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a % int_b);
            instr.rewrite_type(IRType::Assign);
        }else if (b_is_value && instr.b()->int_value() == 1) {//代数化简，b为1则结果等于0
            instr.b()->set_int_value(0);
            instr.rebind_a(instr.b());
            instr.rewrite_type(IRType::Assign);
        }else if (instr.a() == instr.b()) {//代数化简，a=b则结果等于0
            instr.rebind_a(m_ir_sym_table->create_value(BasicType::Int, 0));
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::EqualI:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type()==instr.b()->basic_type());
            instr.a()->set_int_value(int_a == int_b? 1:0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::EqualF:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(float_a == float_b ? 1 : 0);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::NotEqualI:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(int_a != int_b ? 1 : 0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::NotEqualF:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(float_a != float_b ? 1 : 0);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::GreaterI:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(int_a > int_b ? 1 : 0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::GreaterF:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(float_a > float_b ? 1 : 0);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::GreaterEqualI:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(int_a >= int_b ? 1 : 0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::GreaterEqualF:
        if (a_is_value && b_is_value) {//常量折叠
            assert(instr.a()->basic_type() == instr.b()->basic_type());
            instr.a()->set_int_value(float_a >= float_b ? 1 : 0);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::LessI:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a < int_b ? 1 : 0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::LessF:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(float_a < float_b ? 1 : 0);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::LessEqualI:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(int_a <= int_b ? 1 : 0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::LessEqualF:
        if (a_is_value && b_is_value) {//常量折叠
            instr.a()->set_int_value(float_a <= float_b ? 1 : 0);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    default:
        Optimizer::optimizer_error("Unexpected type of instruction");
    }

}
void ConstantFoldAndAlgebraicSimplification::work_unary_calc(IRInstr& instr)
{
    bool a_is_value = instr.a()->kind() == IRSymbolKind::Value;
    int int_a = instr.a()->value().int_value;
    float float_a = instr.a()->value().float_value;
    switch (instr.op()) {
    case IROper::NegI:
        if (a_is_value) {
            instr.a()->set_int_value(-int_a);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::NegF:
        if (a_is_value) {
            instr.a()->set_float_value(-float_a);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::NotI:
        if (a_is_value) {
            instr.a()->set_int_value(int_a==0?1:0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::NotF:
        if (a_is_value) {
            instr.a()->set_int_value(float_a == 0.0 ? 1 : 0);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::IToF:
        if (a_is_value) {
            instr.a()->set_float_value((float)int_a);
            instr.a()->set_basic_type(BasicType::Float);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    case IROper::FToI:
        if (a_is_value) {
            instr.a()->set_int_value((int)float_a);
            instr.a()->set_basic_type(BasicType::Int);
            instr.rewrite_type(IRType::Assign);
        }
        break;
    default:
        Optimizer::optimizer_error("Unexpected type of instruction");
    }
}
void ConstantFoldAndAlgebraicSimplification::ir_list_fold(IRInstrList& program)
{
    for (auto& instr : program){
        switch (instr.type()){
            case IRType::BinaryCalc:
                work_binary_calc(instr);
                break;
            case IRType::UnaryCalc:
                work_unary_calc(instr);
                break;
            default:
                break;
        }
    }
}

void ConstantFoldAndAlgebraicSimplification::control_flow_graph_fold(IRBlock* entry)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(entry);
    while(!q.empty()){
        IRBlock* now=q.front();
        q.pop();
        ir_list_fold(now->get_instr_list());
        for(int k=1;k>=0;--k)
            if(now->get_succ(k)!=nullptr&& visited.find(now->get_succ(k))==visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
    }
}
void ConstantFoldAndAlgebraicSimplification::run()
{
    std::cout << "Running pass: Constant Fold And Algebraic Simplification" << std::endl;
    if (m_cfg == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    for (auto& unit : *m_cfg)
        if (unit.get_type() == IRUnitType::FuncDef)
            control_flow_graph_fold(unit.get_entry());
}