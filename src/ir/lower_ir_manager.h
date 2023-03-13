#pragma once
#include <unordered_map>
#include <unordered_set>

#include "ir_symbol_table.h"
#include "cfg.h"
#include "ir_instr.h"

class LowerIRManager{
private:
    IRProgram* m_cfg{nullptr};
    IRSymbolTable* m_ir_sym_table{ nullptr };
    std::vector<IRSymbol*> m_argreg;
    std::vector<Symbol*> m_runtime_functions;
    // 记录全局变量的地址（和局部数组的首地址）和临时变量的映射关系
    std::unordered_map<IRSymbol*, IRSymbol*> global_addr;
    int max_temp_index{0};

    void bind_argreg();
    void bind_runtime_functions();
    void revise_instr();
    void gen_block(IRUnit& unit, IRSymbol* func, std::unordered_map<IRSymbol*, IRSymbol*> global_addr);
    // 第一次扫描，确定函数内使用到的全局变量和局部数组
    void scan_global(IRUnit& unit, std::unordered_set<IRSymbol*>& global_in_func);
    void ir_list_find_index(IRInstrList& program);
    void find_max_index(IRUnit* unit);
    IRSymbol* load_global(IRSymbol* s, std::unordered_map<IRSymbol*, IRSymbol*> global_addr, IRInstrList& instr_list, IRInstrList::iterator& it);
    IRSymbol* store_global(IRSymbol* s, std::unordered_map<IRSymbol*, IRSymbol*> global_addr, IRInstrList& instr_list, IRInstrList::iterator& it);
    IRSymbol* move_imm(IRSymbol* s, IRInstrList& instr_list, IRInstrList::iterator& it);
public:
    void set_cfg(IRProgram* i_cfg)    {m_cfg = i_cfg;}
    void set_ir_symbol_table(IRSymbolTable* i_ir_sym_table) {m_ir_sym_table = i_ir_sym_table;}
    void set_runtime_functions(std::vector<Symbol*> i_runtime_functions) {m_runtime_functions = i_runtime_functions;}
    std::vector<IRSymbol*> get_argreg() {return m_argreg;}

    IRProgram* gen_lower_ir();

};