#include <ast.h>
#include <cfg_manager.h>
#include <fstream>
#include <iostream>
#include <ir_test_manager.h>
#include <lexer.ll.h>
#include <linear_ir_manager.h>
#include <memory>
#include <optimizer.h>
#include <parser.yy.hpp>
#include <semantic_analysis.h>
#include <reg_alloc.h>
#include <live.h>
#include <string>
#include <arm_manager.h>
#include <arm_struct.h>
#include <lower_ir_manager.h>
#include <interprocedural_data_flow_analysis.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
void gen_ir2c_source(const IRProgram &cfg, std::string filename)
{
    // std::cout << "===============CFG to C================"<<std::endl;
    if(filename.find_last_of('\\')!=std::string::npos)
        filename = filename.substr(filename.find_last_of('\\')+1);
    std::fstream fs;
    std::cout << "ir2c_" << filename << std::endl;
    fs.open("ir2c_" + filename, std::ios::out);
    IRTestManager::gen_c_source(cfg,fs);
    fs.close();
}
char* g_old_stack;
int g_argc;
char** g_argv;
extern int yydebug;
char* g_new_stack;

std::string g_input_filename;
std::string g_output_filename = "test.s";
bool g_ir2c_flag = false;
bool g_asm_flag = false;
int g_opt_level = 0;

bool my_clipp()
{
    int i = 1;
    bool input_flag = false;
    while(i < g_argc)
    {
        if(!strcmp(g_argv[i], "-I"))
        {
            // std::cout << "set ir2c" << std::endl;
            g_ir2c_flag = true;
        }
        else if(!strcmp(g_argv[i], "-S"))
        {
            // std::cout << "set asm" << std::endl;
            g_asm_flag = true;
        }
        else if(!strcmp(g_argv[i], "-O1"))
        {
            // std::cout << "set opt1" << std::endl;
            g_opt_level = 1;
        }
        else if(!strcmp(g_argv[i], "-O2"))
        {
            // std::cout << "set opt2" << std::endl;
            g_opt_level = 2;
        }
        else if(!strcmp(g_argv[i], "-o"))
        {
            // std::cout << "set obj" << std::endl;
            if(i + 1 >= g_argc)
                return false;
            g_output_filename = g_argv[++i];
        }
        else if(!input_flag)
        {
            // std::cout << "set src" << std::endl;
            g_input_filename = g_argv[i];
            input_flag = true;
        }
        else
        {
            std::cout << g_argv[i] << std::endl;
            return false;
        }
        i++;
    }
    return input_flag;
}

extern int yyparse();
int main(int argc, char *argv[])
{
#ifndef WIN32
    const rlim_t kStackSize = 64L * 1024L * 1024L; // min stack size = 64 Mb
    struct rlimit rl;
    int result;
    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            setrlimit(RLIMIT_STACK, &rl);
        }
    }
#endif
#ifdef BUILD_CLANG_X64
    //64M栈空间
    const int STACK_SIZE = 64 << 20;
    //申请栈空间
    g_new_stack = (char*)malloc(STACK_SIZE) + STACK_SIZE;
    __asm__("movq %%rsp,%0\n":"=r"(g_old_stack));
    __asm__("movq %0,%%rsp\n"::"r"(g_new_stack));
#endif

    g_argc = argc;
    g_argv = (char**)malloc(argc * sizeof(char*));
    for (int i = 0; i < argc; ++i)
        g_argv[i] = argv[i];
    //参数解析
    if(my_clipp())
    {
        //没有-I或-S,则默认-S生成汇编
        if(!g_ir2c_flag && !g_asm_flag)
            g_asm_flag = true;
        std::cout << "input_file: " << g_input_filename << "\n"
                  << "output_file: " << g_output_filename << "\n"
                  << "asm_flag: " << g_asm_flag << "\n"
                  << "ir2c_flag: " << g_ir2c_flag <<"\n"
                  << "opt_level:" << g_opt_level << std::endl;
    } else {
        std::cout << "Input:";
        for (int i = 0; i < argc;++i)
            std::cout << g_argv[i] << " ";
        std::cout << std::endl;
        std::cout << "Usage:" << g_argv[0] << " input_file [-I] [-S] [-o output_file] [-O1|-O2]" << std::endl;
        return -1;
    }
    yyin = fopen(g_input_filename.c_str(), "r");
    if (!yyin) {
        std::cout << "Can't open file " << g_input_filename << std::endl;
        return -1;
    }

    yyparse();
    std::cout << "Lexical analysis finished!" << std::endl;
    std::cout << "Syntax analysis finished!" << std::endl;
    SymbolTable symbol_table;
    auto runtime_functions = symbol_table.add_runtime_functions();
    SemanticAnalysis::semantic_analysis(g_ast_root, &symbol_table);
    std::cout << "Semantic analysis finished!" << std::endl;
    Optimizer optimizer;
    optimizer.add_pass(PassType::ConstantFoldAST, true);
    optimizer.add_pass(PassType::LoopInversionAST, true);
    if (g_opt_level >= 2)
        optimizer.add_pass(PassType::ConstantArrayVisitSubstitutionAST, true);
    //optimizer.add_pass(PassType::IfToSwitch,true);
    optimizer.set_target_ast(g_ast_root);
    optimizer.run_pass(PassTarget::AST);


    std::cout << "==========Abstract Syntax Tree=========" << std::endl;
    // g_ast_root->printNode();
    std::cout << "=======================================" << std::endl;
    std::cout << "==============Symbol Table=============" << std::endl;
    //symbol_table.print_table();
    std::cout << "=======================================" << std::endl;

    IRSymbolTable ir_symbol_table;
    LinearIRManager::bind_ir_symbol_table(&ir_symbol_table);
    LinearIRManager::bind_symbol_table(&symbol_table);
    IRInstrList* ir = LinearIRManager::gen_ir(g_ast_root);
    std::cout << "================Linear IR==============" << std::endl;
    //LinearIRManager::print_ir_list(*ir);
    std::cout << "=======================================" << std::endl;
    auto cfg = CFGManager::gen_from_linear_ir(*ir);
    delete ir;
    std::cout << "===========Control Flow Graph==========" << std::endl;
    //CFGManager::print_ir_program(*cfg, std::cout);
    std::cout << "=======================================" << std::endl;
    //MIR优化
    if (g_opt_level>=2) {
        //前期死代码删除
        optimizer.add_pass(PassType::ConvertSSA, true);
        optimizer.add_pass(PassType::ProcedureSpecializationAndClone, true);
        optimizer.add_pass(PassType::DeadCodeElimination, true);
        optimizer.add_pass(PassType::DeadCallElimination, true);
        //第0轮冗余删除
        optimizer.add_pass(PassType::SparseConditionalConstantPropagation, true);
        optimizer.add_pass(PassType::ConstantFoldAndAlgebraicSimplification, true);
        optimizer.add_pass(PassType::GlobalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::LocalCopyPropagation, true);
        optimizer.add_pass(PassType::ConstantRecombination, true);
        optimizer.add_pass(PassType::DeadCodeElimination, true);
        optimizer.add_pass(PassType::GlobalArrayVisitSubstitution, true);
        optimizer.add_pass(PassType::ArrayVisitSubstitutionOnSSA, true);
        optimizer.add_pass(PassType::GlobalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::RevertSSA, true);
        //前期非SSA优化
        optimizer.add_pass(PassType::ProcedureIntegration, true);
        optimizer.add_pass(PassType::GlobalVarLocalization, true);
        optimizer.add_pass(PassType::BlockSimplification, true);
        optimizer.add_pass(PassType::ConstantFoldAndAlgebraicSimplification, true);
        //转为SSA，以及常量传播
        optimizer.add_pass(PassType::ConvertSSA, true);
        optimizer.add_pass(PassType::SparseConditionalConstantPropagation, true);
        
        //第1轮冗余删除
        optimizer.add_pass(PassType::GlobalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::LocalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::LocalCopyPropagation, true);
        optimizer.add_pass(PassType::ConstantRecombination, true);
        optimizer.add_pass(PassType::ConstantFoldAndAlgebraicSimplification, true);
        optimizer.add_pass(PassType::DeadCodeElimination, true);
        //模展开
        optimizer.add_pass(PassType::ModExpansion, true);
        //第2轮冗余删除
        optimizer.add_pass(PassType::GlobalArrayVisitSubstitution, true);
        optimizer.add_pass(PassType::GlobalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::LocalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::LocalCopyPropagation, true);
        optimizer.add_pass(PassType::ConstantRecombination, true);
        optimizer.add_pass(PassType::ConstantFoldAndAlgebraicSimplification, true);
        optimizer.add_pass(PassType::LocalCopyPropagation, true);
        optimizer.add_pass(PassType::ConstantFoldAndAlgebraicSimplification, true);
        optimizer.add_pass(PassType::DeadCodeElimination, true);
        optimizer.add_pass(PassType::DeadCallElimination, true);
        optimizer.add_pass(PassType::RevertSSA, true);
        //循环相关优化
        optimizer.add_pass(PassType::LoopInvariantCodeMotion, true);
        optimizer.add_pass(PassType::ConvertSSA, true);
        optimizer.add_pass(PassType::LoopUnrolling, true);
        optimizer.add_pass(PassType::SparseConditionalConstantPropagation, true);
        optimizer.add_pass(PassType::RevertSSA, true);
        //TODO:支持SSA形式上的基本块化简
        optimizer.add_pass(PassType::BlockSimplification, true);
        // optimizer.add_pass(PassType::AutoMemorize, true);
        //冗余memset删除
        //循环展开后优化
        optimizer.add_pass(PassType::LocalArrayVisitSubstitution, true);
        optimizer.add_pass(PassType::ConvertSSA, true);
        optimizer.add_pass(PassType::GlobalArrayVisitSubstitution, true);
        optimizer.add_pass(PassType::SparseConditionalConstantPropagation, true);  
        optimizer.add_pass(PassType::LocalCommonSubexpressionElimination, true);
        optimizer.add_pass(PassType::LocalCopyPropagation, true);
        optimizer.add_pass(PassType::DeadCodeElimination, true);
        optimizer.add_pass(PassType::ConstantRecombination, true);
        optimizer.add_pass(PassType::ConstantFoldAndAlgebraicSimplification, true);
        optimizer.add_pass(PassType::DeadCodeElimination, true);
        optimizer.add_pass(PassType::RevertSSA, true);
        optimizer.add_pass(PassType::BlockSimplification, true);
        optimizer.add_pass(PassType::OtherRedundantElimination, true);
        InterproceduralDataFlowAnalysis analyzer;
        analyzer.set_target(cfg, &ir_symbol_table);
        //开启全局变量数据流分析
        analyzer.enable_global_symbol_optimization();
        optimizer.set_target_cfg(cfg);
        optimizer.set_ir_sym_table(&ir_symbol_table);
        optimizer.set_sym_table(&symbol_table);
        optimizer.set_idfa(&analyzer);
        optimizer.run_pass(PassTarget::CFG);
    } else {
        //非O2下至少跑一个空基本块删除优化
        optimizer.add_pass(PassType::BlockSimplification, true);
        optimizer.set_target_cfg(cfg);
        optimizer.set_ir_sym_table(&ir_symbol_table);
        optimizer.run_pass(PassTarget::CFG);
    }
    std::cout << "===========Control Flow Graph==========" << std::endl;
    // CFGManager::print_ir_program(*cfg, std::cout);
    std::cout << "=======================================" << std::endl;
    if (g_ir2c_flag){
        // std::cout << "===========Control Flow Graph==========" << std::endl;
        //CFGManager::print_ir_program(*cfg, std::cout);
        //std::cout << "=======================================" << std::endl;
        gen_ir2c_source(*cfg, g_input_filename);
    }
    if (g_asm_flag) {
        if (g_opt_level >= 2) {
            //预转换（加入体系结构限定的指令)
            optimizer.add_pass(PassType::InstructionCombination, true);
            optimizer.add_pass(PassType::StrengthReduction, true);
            optimizer.add_pass(PassType::InstructionCombination, true);
            optimizer.add_pass(PassType::ForwardSubstitution, true);
            optimizer.run_pass(PassTarget::LIR);
        }
        std::cout << "===========Control Flow Graph==========" << std::endl;
        //CFGManager::print_ir_program(*cfg, std::cout);
        std::cout << "=======================================" << std::endl;
        LowerIRManager* lower_ir_manager = new LowerIRManager();
        lower_ir_manager->set_cfg(cfg);
        lower_ir_manager->set_ir_symbol_table(&ir_symbol_table);
        lower_ir_manager->set_runtime_functions(runtime_functions);
        lower_ir_manager->gen_lower_ir();
        std::vector<IRSymbol*> argreg = lower_ir_manager->get_argreg();
        delete lower_ir_manager;
        std::cout << "===========Control Flow Graph==========" << std::endl;
        //CFGManager::print_ir_program(*cfg, std::cout);
        std::cout << "=======================================" << std::endl;
        bool ra_done = false;
        int ra_time = 0;
        std::list<LiveIntervals*> live_intervals;
        std::list<RAT*> register_allocation;
        while(!ra_done)
        {
            for(auto& li: live_intervals)
                delete li;
            live_intervals.clear();
            for(auto& rat: register_allocation)
                delete rat;
            register_allocation.clear();
            std::cout << "---ROUND " << ra_time++ << "---" <<std::endl;
            LiveManager* live_manager = new LiveManager(cfg, argreg);
            live_intervals = live_manager->compute_liveness();
            delete live_manager;
            std::cout << "===========Control Flow Graph==========" << std::endl;
            //CFGManager::print_ir_program(*cfg, std::cout);
            std::cout << "=======================================" << std::endl;
            std::cout << "=============Live Intervals============" << std::endl;
            // LiveManager::print_li_list(live_intervals);
            std::cout << "=======================================" << std::endl;
            
            RegisterAllocator* register_allocator = new RegisterAllocator(live_intervals, argreg, cfg, &ir_symbol_table);
            register_allocation = register_allocator->allocate_register();
            ra_done = register_allocator->is_done();
            delete register_allocator;
            std::cout << "=======Register Allocation Table=======" << std::endl;
            // RegisterAllocator::print_allocation(register_allocation);
            std::cout << "=======================================" << std::endl;
        }
        std::cout << "=============Live Intervals============" << std::endl;
        // LiveManager::print_li_list(live_intervals);
        std::cout << "=======================================" << std::endl;
        std::cout << "=======Register Allocation Table=======" << std::endl;
        // RegisterAllocator::print_allocation(register_allocation);
        std::cout << "=======================================" << std::endl;
        std::cout << "===========Control Flow Graph==========" << std::endl;
        // CFGManager::print_ir_program(*cfg, std::cout);
        std::cout << "=======================================" << std::endl;
        std::cout << "===========ARMv7 asm==========" << std::endl;
        std::ofstream outfile(g_output_filename.c_str());
        // std::string file_name = "test.txt";
        // outfile.open(file_name);
        ArmManager* manager = new ArmManager();
        ArmProg* prog = new ArmProg();
        manager->gen_arm(*cfg, register_allocation, prog, outfile);
        assert(outfile.is_open());
        delete(manager);
        if (g_opt_level >= 2) {
            optimizer.set_target_arm(prog);
            optimizer.add_pass(PassType::SimplifyArm, true);
            optimizer.run_pass(PassTarget::ARM);
        }
        prog->gen_asm(outfile);
        delete(prog);
        outfile.close();
    }
#ifdef BUILD_CLANG_X64
    __asm__("movq %0,%%rsp\n"::"r"(g_old_stack));
#endif
    return 0;
}
/*
* 
void dbg_lex_output()
{
#define ofile std::cout
    // std::ofstream ofile;
    // ofile.open("lexer.txt",std::ios::out);
    int res;
    while ((res = yylex())) {
        std::string s;
        if (res == INTCONST) {
            s = "INTCONST";
        }
        else if (res == FLOATCONST) {
            s = "FLOATCONST";
        }
        else if (res == IDENT) {
            s = "IDENT";
        }
        else if (res == IF) {
            s = "IF";
        }
        else if (res == ELSE) {
            s = "ELSE";
        }
        else if (res == WHILE) {
            s = "WHILE";
        }
        else if (res == BREAK) {
            s = "BREAK";
        }
        else if (res == CONTINUE) {
            s = "CONTINUE";
        }
        else if (res == RETURN) {
            s = "RETURN";
        }
        else if (res == INT) {
            s = "INT";
        }
        else if (res == FLOAT) {
            s = "FLOAT";
        }
        else if (res == VOID) {
            s = "VOID";
        }
        else if (res == CONST) {
            s = "CONST";
        }
        else if (res == L_PAREN) {
            s = "L_PAREN";
        }
        else if (res == R_PAREN) {
            s = "R_PAREN";
        }
        else if (res == L_BRACK) {
            s = "L_BRACK";
        }
        else if (res == R_BRACK) {
            s = "R_BRACK";
        }
        else if (res == L_BRACE) {
            s = "L_BRACE";
        }
        else if (res == R_BRACE) {
            s = "R_BRACE";
        }
        else if (res == SEMI) {
            s = "SEMI";
        }
        else if (res == COMMA) {
            s = "COMMA";
        }
        else if (res == ASSIGN) {
            s = "ASSIGN";
        }
        else if (res == ADD) {
            s = "ADD";
        }
        else if (res == SUB) {
            s = "SUB";
        }
        else if (res == MUL) {
            s = "MUL";
        }
        else if (res == DIV) {
            s = "DIV";
        }
        else if (res == MOD) {
            s = "MOD";
        }
        else if (res == AND) {
            s = "AND";
        }
        else if (res == OR) {
            s = "OR";
        }
        else if (res == NOT) {
            s = "NOT";
        }
        else if (res == GT) {
            s = "GT";
        }
        else if (res == LT) {
            s = "LT";
        }
        else if (res == LE) {
            s = "LE";
        }
        else if (res == GE) {
            s = "GE";
        }
        else if (res == EQ) {
            s = "EQ";
        }
        else if (res == NEQ) {
            s = "NEQ";
        }
        else {
            s = "?";
        }
        ofile << s;
        if (res == INTCONST) {
            ofile << " " << yylval.int_const_val << std::endl;
        }
        else if (res == FLOATCONST) {
            ofile << " " << yylval.float_const_val << std::endl;
        }
        else if (res == IDENT) {
            ofile << " NO." << yylval.int_const_val << std::endl;
        }
        else {
            ofile << std::endl;
        }
    }
}
*/