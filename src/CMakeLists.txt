target_sources(compiler PRIVATE
# Back End
    ./back_end/reg_alloc.cpp
    ./back_end/live.cpp
    ./back_end/arm_define.cpp
    ./back_end/arm_manager.cpp
    ./back_end/arm.cpp
    ./back_end/arm_struct.cpp
# Front End
    ./front_end/ast.cpp
    ./front_end/semantic_analysis.cpp
# IR
    ./ir/ir_define.cpp
    ./ir/ir_instr.cpp
    ./ir/ir_symbol_table.cpp
    ./ir/cfg.cpp
    ./ir/cfg_manager.cpp
    ./ir/linear_ir_manager.cpp
    ./ir/ir_test_manager.cpp
    ./ir/lower_ir_manager.cpp
# Global
    ./global/type_define.cpp
    ./global/symbol_table.cpp
    ./global/bitmap.cpp
    ./global/interval.cpp
# Optimizer
    ./optimizer/optimizer.cpp
    ./optimizer/constant_fold_ast.cpp
    ./optimizer/constant_fold_and_algebraic_simplification.cpp
    ./optimizer/convert_ssa.cpp
    ./optimizer/revert_ssa.cpp
    ./optimizer/sparse_conditional_constant_propagation.cpp
    ./optimizer/block_simplification.cpp
    ./optimizer/local_common_subexpression_elimination.cpp
    ./optimizer/loop_invariant_code_motion.cpp
    ./optimizer/dead_code_elimination.cpp
    ./optimizer/dead_call_elimination.cpp
    ./optimizer/local_copy_propagation.cpp
    ./optimizer/strength_reduction.cpp
    ./optimizer/procedure_integration.cpp
    ./optimizer/procedure_specialization_and_clone.cpp
    ./optimizer/constant_recombination.cpp
    ./optimizer/mod_expansion.cpp
    ./optimizer/simplify_arm.cpp
    ./optimizer/forward_substitution.cpp
    ./optimizer/instruction_combination.cpp
    ./optimizer/loop_unrolling.cpp
    ./optimizer/interprocedural_data_flow_analysis.cpp
    ./optimizer/loop_inversion_ast.cpp
    ./optimizer/local_array_visit_substitution.cpp
    ./optimizer/if_to_switch.cpp
    ./optimizer/global_variable_localization.cpp
    ./optimizer/global_common_subexpression_elimination.cpp
    ./optimizer/constant_array_visit_substitution.cpp
    ./optimizer/global_array_visit_substitution.cpp
    ./optimizer/array_visit_substitution_on_ssa.cpp
    ./optimizer/auto_memorize.cpp
    ./optimizer/other_redundant_elimination.cpp
    main.cpp
)