#include <simplify_arm.h>
#include <cassert>

using namespace std;

const int MaxInstNum = 5;
const int debug = 1;
int conditioning_count = 0;
int eliminate_branch_count = 0;
int eliminate_ldrstr_count = 0;

void SimplifyArm::eliminate_identical_move(ArmFunc*& func)
{
    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end(); ++iter) {
        auto block = *iter;
        auto& insts = block->get_inst_list();
        for (auto it = insts.begin(); it != insts.end();) {
            if (it->get_type() == InstrType::Move) {
                if (it->get_operand2()->get_string() == it->get_rd()->get_string()) {
                    it = insts.erase(it);
                    continue;
                }
            } else if (it->get_type() == InstrType::MoveS) {
                if (it->get_rd() != nullptr && it->get_rm() != nullptr && it->get_rd() == it->get_rm()) {
                    it = insts.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

void SimplifyArm::eliminate_useless_branch_before(ArmFunc*& func)
{
    unordered_map<string, ArmBlock*> block_table;
    for (auto& bb: func->get_blocks())
        block_table.emplace(bb->get_label(), bb);
    // 若一个基本块只有一条指令且为跳转，则删除此块，并将所有跳到此块的分支目标改为此块的跳转目标
    unordered_map<string, string> label;
    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end();) {
        auto block = *iter;
        auto& insts = block->get_inst_list();
        if (insts.size() == 1) {
            auto inst = insts.front();
            // inst.gen_asm(cout);
            if (inst.get_type() == InstrType::Branch && !inst.get_is_l() && !inst.get_is_x()) {
                label.emplace(block->get_label(), insts.front().get_label());
                // inst.gen_asm(cout);
                iter = func->get_blocks().erase(iter);
                continue;
            }
        }
        ++iter;
    }

    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end(); ++iter) {
        auto block = *iter;
        auto& insts = block->get_inst_list();
        auto& succ = block->get_edge();
        for (auto& bb: succ) {
            if (label.find(bb->get_label()) != label.end())
                bb = block_table.at(label.at(bb->get_label()));
        }
        for (auto it = insts.begin(); it != insts.end(); ++it) {
            if (it->get_type() == InstrType::Branch && !it->get_is_l() && !it->get_is_x()) {
                if (label.find(it->get_label()) != label.end()) {
                    it->set_label(label.at(it->get_label()));
                    ++eliminate_branch_count;
                }
            }
        }
    }
}

void SimplifyArm::eliminate_useless_branch(ArmFunc*& func)
{
    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end(); ++iter) {
        auto block = *iter;
        auto& insts = block->get_inst_list();
        ++iter;
        if (iter == func->get_blocks().end())   //next_bb
            break;
        auto next_bb = *iter;
        --iter;
        for (auto it = insts.begin(); it != insts.end();) {
            if (it->get_type() == InstrType::Branch && !it->get_is_l() && !it->get_is_x()) {
                if (it->get_label() == next_bb->get_label()) {
                    it = insts.erase(it);
                    ++eliminate_branch_count;
                }
                else ++it;
            }
            else ++it;
        }
        auto inst_it = insts.rbegin();
        if (insts.size() >= 2) {
            ++inst_it;
            if (insts.rbegin()->get_type() == InstrType::Branch && !insts.rbegin()->get_is_l() && !insts.rbegin()->get_is_x() &&
                inst_it->get_type() == InstrType::Branch && !inst_it->get_is_l() && !inst_it->get_is_x()) {
                // 变成直接跳转
                insts.rbegin()->set_cond(Cond::AL);    
            }
        }
    }
}

void SimplifyArm::eliminate_useless_ldrstr(ArmFunc*& func)
{
    // case1:
    // ldr r0 [mem1]
    // str r0 [mem1]    eliminate str
    // case2:
    // str r0 [mem1]
    // ldr r0 [mem1]    eliminate ldr
    // case3:
    // str r0 [mem1]
    // ldr r1 [mem1]    -> mov r1, r0
    for (auto& block: func->get_blocks()) {
        auto& insts = block->get_inst_list();
        for (auto it = insts.begin(); it != insts.end();) {
            auto cur_inst = *it;
            ++it;
            if (it == insts.end())
                break;
            auto next_inst = *it;
            auto cur_rd = cur_inst.get_rd();
            auto cur_rn = cur_inst.get_rn();
            auto cur_offset = cur_inst.get_operand2();
            auto next_rd = next_inst.get_rd();
            auto next_rn = next_inst.get_rn();
            auto next_offset = next_inst.get_operand2();
            if (((cur_inst.get_type() == InstrType::Load || cur_inst.get_type() == InstrType::LoadS) &&
            (next_inst.get_type() == InstrType::Store || next_inst.get_type() == InstrType::StoreS)) ||
            ((cur_inst.get_type() == InstrType::Store || cur_inst.get_type() == InstrType::StoreS) &&
            (next_inst.get_type() == InstrType::Load|| next_inst.get_type() == InstrType::LoadS))) {
                if (cur_rd == next_rd && cur_rn == next_rn && 
                cur_offset == nullptr && next_offset == nullptr && cur_inst.get_imm() == next_inst.get_imm()) {
                    // cur_inst.gen_asm(cout);
                    // next_inst.gen_asm(cout);
                    it = insts.erase(it);
                    ++eliminate_ldrstr_count;
                    continue;
                }
            }
            if ((cur_inst.get_type() == InstrType::Store || cur_inst.get_type() == InstrType::StoreS) &&
            (next_inst.get_type() == InstrType::Load|| next_inst.get_type() == InstrType::LoadS)) {
                if (cur_rn == next_rn && cur_offset == nullptr && next_offset == nullptr && cur_inst.get_imm() == next_inst.get_imm()) {
                    it = insts.erase(it);
                    ArmInstr* inst = nullptr;
                    if (cur_rd->get_reg_id() >= RegCount || next_rd->get_reg_id() >= RegCount) {
                        auto instr = ArmInstr::create_float_move(Cond::AL, next_rd, cur_rd);
                        inst = &instr;
                    } else {
                        auto instr = ArmInstr::create_move(Cond::AL, false, next_rd, new Operand2(cur_rd));
                        inst = &instr;
                    }
                    // cur_inst.gen_asm(cout);
                    // next_inst.gen_asm(cout);
                    it = insts.insert(it, *inst);
                    ++eliminate_ldrstr_count;
                }
            }
        }
    }
}

void SimplifyArm::inst_conditioning(ArmFunc*& func)
{
    unordered_set<ArmBlock*> deleted;
    unordered_set<ArmBlock*> visited;
    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end(); ++iter) {
        auto block = *iter;
        auto& insts = block->get_inst_list();
        auto succ = block->get_edge();
        if (succ.size() != 2)
            continue;
        auto bb_true = succ[1];
        auto bb_false = succ[0];
        // 不能是自身
        if (bb_true == block || bb_false == block)
            continue;
        if (deleted.find(bb_true) != deleted.end() || deleted.find(bb_false) != deleted.end() ||
        visited.find(bb_true) != visited.end() || visited.find(bb_false) != visited.end())
            continue;
        auto next_bb = choose_next_bb(block, bb_true, bb_false);
        ArmBlock* next_next_bb = nullptr;
        if (next_bb == nullptr)
            continue;
        else if (next_bb == bb_true)
            next_next_bb = bb_false;
        else    
            next_next_bb = bb_true;
        // 给next_bb中的指令带上条件
        for (auto it = insts.begin(); it != insts.end();) {
            if (it->get_type() == InstrType::Branch && !it->get_is_l() && !it->get_is_x()) {
                Cond cond;
                if (it->get_label() == next_next_bb->get_label())
                    cond = GetOppositeCond(it->get_cond());
                else
                    cond = it->get_cond();
                for (auto& inst: next_bb->get_inst_list()) {
                    inst.set_cond(cond);
                    // inst.gen_asm(cout);
                }
                it = insts.erase(it);
                continue;
            }
            ++it;
        }
        ++conditioning_count;
        visited.emplace(block);
        deleted.emplace(next_bb);
        deleted.emplace(next_next_bb);
        // 在当前块后分别插入next_bb和next_next_bb
        auto next_block = new ArmBlock(next_bb->get_label());
        auto next_next_block = new ArmBlock(next_next_bb->get_label());
        copy_block(next_block, next_bb);
        copy_block(next_next_block, next_next_bb);

        // ArmBlock next_block = *next_bb;
        // ArmBlock next_next_block = *next_next_bb;
        // next_bb = &next_block;
        // next_next_bb = &next_next_block;
        ++iter;
        if (iter == func->get_blocks().end()) {
            func->get_blocks().emplace_back(next_block);
            func->get_blocks().emplace_back(next_next_block);
            break;
        } else {
            iter = func->get_blocks().insert(iter, next_block);
            iter = func->get_blocks().insert(++iter, next_next_block);
            --iter;
        }
    }
    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end();) {
        auto block = *iter;
        if (deleted.find(block) != deleted.end())
            iter = func->get_blocks().erase(iter);
        else
            ++iter;
    }
}

void SimplifyArm::inst_conditioning_simple(ArmFunc*& func)
{
    for (auto iter = func->get_blocks().begin(); iter != func->get_blocks().end(); ++iter) {
        auto block = *iter;
        auto& insts = block->get_inst_list();
        ++iter;
        if (iter == func->get_blocks().end())   //next_bb
            break;
        ++iter;
        if (iter == func->get_blocks().end())   //next_next_bb
            break;
        auto next_next_bb = *iter;
        --iter;
        auto next_bb = *iter;
        --iter;
        for (auto it = insts.begin(); it != insts.end();) {
            if (it->get_type() == InstrType::Branch && !it->get_is_l() && !it->get_is_x()) {
                if (it->get_cond() != Cond::AL && next_bb->pred_num() == 1 && next_bb->get_pred() == block && next_bb->get_inst_list().size() <= MaxInstNum &&
                    it->get_label() == next_next_bb->get_label()) {
                    bool replace = true;
                    Cond cond = GetOppositeCond(it->get_cond());
                    // 基本块内都是无条件指令且都是基本运算或者移动指令才使用条件指令
                    for (auto inst: next_bb->get_inst_list()) {
                        InstrType type = inst.get_type();
                        if (inst.get_cond() != Cond::AL || type == InstrType::Branch && inst.get_is_l() ||
                            type == InstrType::Pop || type == InstrType::Push || type == InstrType::PopS || type == InstrType::PushS) {
                            replace = false;
                            break;
                        }
                    }
                    if (replace) {
                        for (auto& inst: next_bb->get_inst_list()) {
                            inst.set_cond(cond);
                            // inst.gen_asm(cout);
                        }
                        it = insts.erase(it);
                        ++conditioning_count;
                        continue;
                    }
                }
            }
            ++it;
        }
    }
}

ArmBlock* SimplifyArm::choose_next_bb(ArmBlock* bb, ArmBlock* bb_true, ArmBlock* bb_false)
{
    int limit_bb_true = MaxInstNum;
    int limit_bb_false = MaxInstNum;
    // 转换成功后必定可以省略一条branch，可以增加指令条数限制
    if (bb_true->get_edge().size() > 0 && bb_true->get_edge(0) == bb_false)
        limit_bb_true = MaxInstNum + 1;
    if (bb_false->get_edge().size() > 0 && bb_false->get_edge(0) == bb_true)
        limit_bb_false = MaxInstNum + 1;
    if (bb_true->pred_num() == 1 && bb_true->get_inst_list().size() <= limit_bb_true) {
        bool replace = true;
        for (auto inst: bb_true->get_inst_list()) {
            InstrType type = inst.get_type();
            if (inst.get_cond() != Cond::AL || type == InstrType::Branch && inst.get_is_l() ||
                type == InstrType::Pop || type == InstrType::Push || type == InstrType::PopS || type == InstrType::PushS) {
                replace = false;
                break;
            }
        }
        if (replace) {
            return bb_true;
        }
    }
    if (bb_false->pred_num() == 1 && bb_false->get_inst_list().size() <= limit_bb_false) {
        bool replace = true;
        for (auto inst: bb_false->get_inst_list()) {
            InstrType type = inst.get_type();
            if (inst.get_cond() != Cond::AL || type == InstrType::Branch && inst.get_is_l() ||
                type == InstrType::Pop || type == InstrType::Push || type == InstrType::PopS || type == InstrType::PushS) {
                replace = false;
                break;
            }
        }
        if (replace) {
            return bb_false;
        }
    }
    return nullptr;
}

void SimplifyArm::copy_block(ArmBlock* dst, ArmBlock* src)
{
    for (auto bb: src->get_edge())
        dst->set_edge(bb);
    for (auto bb: src->get_pred_list())
        dst->add_pred(bb);
    auto& inst_list = dst->get_inst_list();
    for (auto inst: src->get_inst_list())
        inst_list.emplace_back(inst);
}

void SimplifyArm::run()
{
    std::cout << "Running pass: SimplifyArm" << std::endl;
    if (m_arm == nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    assert(m_arm != nullptr);
    for (auto& func: m_arm->get_func_list()) {
        eliminate_identical_move(func);
        eliminate_useless_ldrstr(func);
        eliminate_useless_branch_before(func);
        // eliminate_useless_branch(func);
        // inst_conditioning_simple(func);
        inst_conditioning(func);
        eliminate_useless_branch(func);
    }
    if (debug && conditioning_count > 0)
        cerr << "conditioning_count: " << conditioning_count << endl; 
    if (debug && eliminate_branch_count > 0)
        cerr << "eliminate_branch_count: " << eliminate_branch_count << endl; 
    if (debug && eliminate_ldrstr_count > 0)
        cerr << "eliminate_ldrstr_count: " << eliminate_ldrstr_count << endl; 
}