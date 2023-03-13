#include <ast.h>
#include <cassert>
#include <iostream>
#include <optimizer.h>
#include <sstream>
#include <string>
#include <block_simplification.h>
#include <constant_fold_ast.h>
#include <constant_fold_and_algebraic_simplification.h>
#include <constant_recombination.h>
#include <convert_ssa.h>
#include <dead_call_elimination.h>
#include <dead_code_elimination.h>
#include <local_common_subexpression_elimination.h>
#include <local_copy_propagation.h>
#include <loop_invariant_code_motion.h>
#include <mod_expansion.h>
#include <procedure_integration.h>
#include <revert_ssa.h>
#include <simplify_arm.h>
#include <sparse_conditional_constant_propagation.h>
#include <strength_reduction.h>
#include <forward_substitution.h>
#include <instruction_combination.h>
#include <procedure_specialization_and_clone.h>
#include <loop_unrolling.h>
#include <local_array_visit_substitution.h>
#include <loop_inversion_ast.h>
#include <global_variable_localization.h>
#include <if_to_switch.h>
#include <global_common_subexpression_elimination.h>
#include <constant_array_visit_substitution.h>
#include <global_array_visit_substitution.h>
#include <array_visit_substitution_on_ssa.h>
#include <auto_memorize.h>
#include <other_redundant_elimination.h>
void Optimizer::set_target_ast(ASTNode* target_ast)
{
    m_ast = target_ast;
}
void Optimizer::set_target_cfg(IRProgram* target_cfg)
{
    m_cfg = target_cfg;
}
void Optimizer::set_target_arm(ArmProg* target_arm)
{
    m_arm = target_arm;
}
void Optimizer::set_idfa(InterproceduralDataFlowAnalysis* i_idfa)
{
    m_idfa = i_idfa;
}
void Optimizer::set_ir_sym_table(IRSymbolTable* target_ir_sym_table)
{
    m_ir_sym_table = target_ir_sym_table;
}
void Optimizer::set_sym_table(SymbolTable* target_sym_table) { 
    m_sym_table = target_sym_table; 
}

void Optimizer::add_pass(PassType pass_type, bool emit)
{
    Pass *pass;
    switch (pass_type) {
    case PassType::ConstantFoldAST:
        pass = new ConstantFoldAST(emit);
        break;
    case PassType::LoopInversionAST:
        pass = new LoopInversionAST(emit);
        break;
    case PassType::IfToSwitch:
        pass = new IfToSwitch(emit);
        break;
    case PassType::ConstantArrayVisitSubstitutionAST:
        pass = new ConstantArrayVisitSubstitutionAST(emit);
        break;
    case PassType::ConstantFoldAndAlgebraicSimplification:
        pass = new ConstantFoldAndAlgebraicSimplification(emit);
        break;
    case PassType::BlockSimplification:
        pass = new BlockSimplification(emit);
        break;
    case PassType::ConvertSSA:
        pass = new ConvertSSA(emit);
        break;
    case PassType::SparseConditionalConstantPropagation:
        pass = new SparseConditionalConstantPropagation(emit);
        break;
    case PassType::RevertSSA:
        pass = new RevertSSA(emit);
        break;
    case PassType::LocalCommonSubexpressionElimination:
        pass = new LocalCommonSubexpressionElimination(emit);
        break;
    case PassType::LocalCopyPropagation:
        pass = new LocalCopyPropagation(emit);
        break;
    case PassType::DeadCallElimination:
        pass = new DeadCallElimination(emit);
        break;
    case PassType::DeadCodeElimination:
        pass = new DeadCodeElimination(emit);
        break;
    case PassType::LoopInvariantCodeMotion:
        pass = new LoopInvariantCodeMotion(emit);
        break;
    case PassType::ProcedureIntegration:
        pass = new ProcedureIntegration(emit);
        break;
    case PassType::ConstantRecombination:
        pass = new ConstantRecombination(emit);
        break;
    case PassType::ModExpansion:
        pass = new ModExpansion(emit);
        break;
    case PassType::SimplifyArm:
        pass = new SimplifyArm(emit);
        break;
    case PassType::StrengthReduction:
        pass = new StrengthReduction(emit);
        break;
    case PassType::InstructionCombination:
        pass = new InstructionCombination(emit);
        break;
    case PassType::ForwardSubstitution:
        pass = new ForwardSubstitution(emit);
        break;
    case PassType::ProcedureSpecializationAndClone:
        pass = new ProcedureSpecializationAndClone(emit);
        break;
    case PassType::LoopUnrolling:
        pass = new LoopUnrolling(emit);
        break;
    case PassType::GlobalVarLocalization:
        pass = new GlobalVarLocalization(emit);
        break;
    case PassType::GlobalCommonSubexpressionElimination:
        pass = new GlobalCommonSubexpressionElimination(emit);
        break;
    case PassType::GlobalArrayVisitSubstitution:
        pass = new GlobalArrayVisitSubstitution(emit);
        break;
    case PassType::LocalArrayVisitSubstitution:
        pass = new LocalArrayVisitSubstitution(emit);
        break;
    case PassType::ArrayVisitSubstitutionOnSSA:
        pass = new ArrayVisitSubstitutionOnSSA(emit);
        break;
    case PassType::AutoMemorize:
        pass = new AutoMemorize(emit);
        break;
    case PassType::OtherRedundantElimination:
        pass = new OtherRedundantElimination(emit);
        break;
    default:
        optimizer_error("Unimplemented Pass");
        break;
    }
    m_passes.push_back(pass);
}
void Optimizer::run_pass(PassTarget target)
{
    for (auto &pass : m_passes) {
        if (target == PassTarget::AST) {
            pass->set_target_ast(m_ast);
        }else if (target == PassTarget::CFG) {
            pass->set_target_cfg(m_cfg);
        }else if (target == PassTarget::LIR) {
            pass->set_target_cfg(m_cfg);
        } else {
            pass->set_target_arm(m_arm);
        }
        pass->set_target_idfa(m_idfa);
        pass->set_ir_sym_table(m_ir_sym_table);
        pass->set_sym_table(m_sym_table);
        pass->run();
    }
    clear_all_passes();
}
void Optimizer::optimizer_error(std::string msg)
{
    std::cerr<<"[Optimizer Error] " << msg << std::endl;
    exit(ErrorCode::OPTIMIZER_ERROR);
}
void Optimizer::clear_all_passes()
{
    for (auto pass : m_passes) {
        delete pass;
    }
    m_passes.clear();
}