#pragma once
#include <string>
#include <vector>
#include <ast.h>
#include <cfg.h>
#include <arm_struct.h>
#include <interprocedural_data_flow_analysis.h>
enum class PassTarget :int
{
    //优化作用目标
    AST,
    CFG,
    SSA,
    LIR,
    ARM
};
enum class PassType :int
{
    //优化类型
    //AST
    ConstantFoldAST,                        //常量折叠
    LoopInversionAST,                       //循环倒置
    IfToSwitch,                             //if转switch
    ConstantArrayVisitSubstitutionAST,                 //数组没有任何修改动作用初始值替换
    ArrayVisitSubstitutionOnSSA,            //在SSA上的数组访问替代
    //RecombinationAST,                       //重结合（引入+=,-=,*=,/=,%=语句,暴露更多公共子表达式）
    //MIR
    ConstantFoldAndAlgebraicSimplification, //常量折叠和代数化简
    BlockSimplification,                    //基本块化简
    SparseConditionalConstantPropagation,   //稀有条件常量传播
    DeadCallElimination,                    //死调用删除
    DeadCodeElimination,                    //死代码删除
    ConvertSSA,                             //转为SSA形式
    RevertSSA,                              //从SSA转回原形式
    LocalArrayVisitSubstitution,            //局部数组访问替代
    GlobalArrayVisitSubstitution,           //全局数组访问替代
    AutoMemorize,                           //递归函数自动记忆化
    SimplifyArm,                            //汇编化简
    LocalCommonSubexpressionElimination,    //局部公共子表示式删除
    GlobalCommonSubexpressionElimination,   //全局公共子表示式删除
    LocalCopyPropagation,                   //局部复写传播
    LoopInvariantCodeMotion,                //循环不变式外提
    LoopUnrolling,                          //循环展开
    ProcedureIntegration,                   //过程集成
    ConstantRecombination,                  //常量重结合
    ModExpansion,                           //模运算展开
    ProcedureSpecializationAndClone,        //过程特殊化和克隆
    GlobalVarLocalization,                  //全局变量局部化
    OtherRedundantElimination,              //其他冗余删除
    //LIR
    StrengthReduction,                      //强度削弱
    InstructionCombination,                 //指令融合
    ForwardSubstitution,                    //向前替代
};

class Pass
{
private:
    //pass类型
    PassType m_type;
    //pass作用目标
    PassTarget m_target;
    //执行选项
    bool m_emit{ false };
protected:
    union
    {
        ASTNode* m_ast{nullptr};
        IRProgram* m_cfg;
        ArmProg* m_arm;
    };
    InterproceduralDataFlowAnalysis* m_idfa;
    IRSymbolTable* m_ir_sym_table{nullptr};
    SymbolTable* m_sym_table{nullptr};
public:
    Pass(PassType i_type,PassTarget i_target, bool i_emit): m_type(i_type),m_target(i_target), m_emit(i_emit) {}
    //set函数
    void set_pass_name(PassType i_type) {m_type = i_type;}
    void set_pass_emit(bool i_emit) {m_emit = i_emit;}
    void set_target_ast(ASTNode* i_ast) { m_ast = i_ast; };
    void set_target_cfg(IRProgram* i_cfg) { m_cfg = i_cfg; };
    void set_target_arm(ArmProg* i_arm) { m_arm = i_arm; };
    void set_ir_sym_table(IRSymbolTable* i_ir_sym_table) { m_ir_sym_table = i_ir_sym_table; };
    void set_sym_table(SymbolTable* i_sym_table) { m_sym_table = i_sym_table; };
    void set_target_idfa(InterproceduralDataFlowAnalysis* i_idfa) { m_idfa = i_idfa; };
    //get函数
    PassType get_pass_name() {return m_type;}
    bool get_pass_emit() {return m_emit;}
    PassTarget get_pass_target() { return m_target; }
    //run函数，由继承的子类实现
    virtual void run() = 0;
};

class Optimizer
{
private:
    ASTNode* m_ast{nullptr};
    IRProgram* m_cfg{nullptr};
    ArmProg* m_arm{nullptr};
    InterproceduralDataFlowAnalysis* m_idfa;
    IRSymbolTable* m_ir_sym_table{ nullptr };
    SymbolTable* m_sym_table{nullptr};
    std::vector<Pass*> m_passes{};
public:
    static void optimizer_error(std::string msg);
    void set_target_ast(ASTNode* target_ast);
    void set_target_cfg(IRProgram* target_cfg);
    void set_target_arm(ArmProg* i_arm);
    void set_idfa(InterproceduralDataFlowAnalysis* i_idfa);
    void set_ir_sym_table(IRSymbolTable* target_ir_sym_table);
    void set_sym_table(SymbolTable* target_sym_table);
    void add_pass(PassType pass_type, bool emit);
    void clear_all_passes();
    void run_pass(PassTarget target);
};