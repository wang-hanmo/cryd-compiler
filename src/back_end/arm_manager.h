#pragma once
#include <arm.h>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <reg_alloc.h>
#include <../ir/cfg.h>
#include <arm_struct.h>

class ArmManager
{
private:
    std::unordered_map<IRSymbol*, Reg*> var_map;            // 记录当前变量和寄存器的对应关系
    std::unordered_map<IRSymbol*, int> local_var_offset;    // 记录局部变量对应栈底的偏移量
    // 记录块标号和块的映射关系
    std::unordered_map<int, ArmBlock*> block_table;
    // 在对每条指令进行寄存器分配时，需要预计算该指令需要分配的寄存器（在rat中分配的寄存器）
    // spill的符号不分配occupied_regs中的寄存器，否则会产生冲突
    std::unordered_set<Reg*> occupied_regs;
    // 对于一条函数调用的四元式，寄存器分配中认为在该式中实参所在寄存器不一定活跃
    // 因此需要专门记录当前函数调用的实参所在的寄存器
    std::unordered_set<Reg*> rparam_regs;
    // 记录基本块是否被访问过
    std::set<IRBlock*> visited;
    std::vector<Reg*> machine_regs;
    // 记录当前寄存器中存放的变量（可能是多个，记录最新的即可）
    std::vector<IRSymbol*> var_of_reg;
    // 记录一个指令的目的寄存器，在源操作数溢出需要分配寄存器时，可以分配目的寄存器
    Reg* dst_reg;
    RAT* rat;                  
    int stack_size = 0;
    // 记录当前行号
    int current_line_no;
    // 记录当前基本块
    ArmBlock* current_block;
    // 记录当前函数
    ArmFunc* current_func;
    // 记录是否为第一次扫描，第一次扫描确定栈空间（哪些符号需要存到内存）和需要保存的寄存器
    bool first_scan;
public:
    // reg_list为一个指令为其运算分量所分配的寄存器
    Reg* Symbol2Reg(IRSymbol* a, bool is_rd, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    Reg* find_free_reg(std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    // 寻找浮点寄存器
    Reg* find_free_freg(std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    Operand2* Symbol2Operand2(IRSymbol* b, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg, bool is_offset = false);
    void GenImmLdrStrInst(InstrType type, Reg* rd, Reg* rn, int imm, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    void CreateShiftInst(IROper op, ShiftOp shift_op, IRSymbol* r, IRSymbol* a, IRSymbol* b, IRSymbol* c, 
                            std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    void CreateBinaryInst(IROper op, IRSymbol* r, IRSymbol* a, IRSymbol* b, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    void CreateUnaryInst(IROper op, IRSymbol* r, IRSymbol* a, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    void CreateTernaryInst(IROper op, IRShiftOper sop, IRSymbol* r, IRSymbol* a, IRSymbol* b, IRSymbol* c, std::unordered_set<Reg*>& reg_list, 
        std::vector<Reg*>& push_reg);
    bool gen_block(IRBlock *block, ArmFunc*& func);
    // 将某个变量的值存回内存
    void store_var(IRSymbol* s, Reg* reg, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    // 将立即数移动到寄存器
    void move_imm(int imm, Reg* reg);
    // 将某个变量的值从内存取回寄存器
    void load_var(IRSymbol* s, Reg* reg);
    // 全局变量数组存回内存
    void store_global_array(IRSymbol* s, Reg* reg, Operand2* offset, std::unordered_set<Reg*>& reg_list, std::vector<Reg*>& push_reg);
    // 在生成每条指令之前，更新occupied_regs,对于ArrayStore、Mod has_dst为false
    void update_occupied_regs(IRSymbol* a, IRSymbol* b, IRSymbol* r, bool has_dst = true, IRSymbol* c = nullptr);
    void update_sp(BinaryOp op, int offset, std::unordered_set<IRSymbol*> allocate_var);
    void move_sp(int offset);
    void ResetFuncData(RAT* rat);
    void gen_func(const IRUnit* func, RAT* reg_alloc, ArmFunc*& arm_func);
    void dfs_block(IRBlock* block, ArmFunc*& func, std::vector<Reg*> general_regs, std::vector<Reg*> vfp_regs, int stack_size);
    void bfs_block(IRBlock* block, ArmFunc*& func, std::vector<Reg*> general_regs, std::vector<Reg*> vfp_regs, int stack_size);
    // 建立arm基本块的前驱后继关系
    void build_edge(IRBlock* block);
    void gen_global_var(const IRInstrList &list, std::ostream &os);
    void gen_arm(const IRProgram &prog, std::list<RAT*> rats, ArmProg*& arm_prog, std::ostream &os);

    static void arm_error(std::string msg, int error_code);
};