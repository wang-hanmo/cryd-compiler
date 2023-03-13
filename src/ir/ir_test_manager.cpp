#include <ir_test_manager.h>
#include <queue>
#include <set>
#include <sstream>
#include <iomanip>
using namespace std;
string get_c_type_str(BasicType type,bool is_pointer=false)
{
    string res;
    if (type == BasicType::Int)
        res = "int";
    else if (type == BasicType::Float)
        res = "float";
    else if (type == BasicType::Void)
        res = "void";
    else res = "error_type";
    if (is_pointer)
        res += "*";
    return res;
}
void IRTestManager::gen_local_var_def(BasicType type, int index, int length, ostream &os, string prefix)
{
    os << prefix;
    if (length > 0) {
        // os<< get_c_type_str(type) << " *t" << index << " = malloc(sizeof(" << get_c_type_str(type)<<") * "<< length <<");"<<endl;
        os << get_c_type_str(type) << " l" << index << "[" << length << "];" << endl;
    } else {
        os << get_c_type_str(type, length==0) << " l" << index << ";" << endl;
    }
}
void IRTestManager::gen_fparam(BasicType type, int index, bool is_array, ostream &os)
{
    os << get_c_type_str(type,is_array) << " l" << index;
}
void IRTestManager::gen_function_def_head(const IRInstrList &list, ostream &os)
{
    int fparam_count = 0;
    for (const auto &instr : list) {
        try {
            if (instr.type() == IRType::FuncDef) {
                Symbol *symbol = instr.a()->global_sym();
                os << get_c_type_str(symbol->get_val_type().basic()) << " " << symbol->get_name() << "(";
                fparam_count = instr.b()->value().int_value;
                if (fparam_count == 0)
                    os << "){" << endl;
            } else if (instr.type() == IRType::FParam) {
                gen_fparam(instr.a()->basic_type(), instr.a()->index(), instr.a()->array_length() == IRArrayLength::IR_ARRAY_POINTER, os);
                fparam_count--;
                if (fparam_count == 0) {
                    os << "){" << endl;
                } else {
                    os << ",";
                }
            } else if (instr.type() == IRType::LocalDecl) {
                gen_local_var_def(instr.a()->basic_type(), instr.a()->index(), instr.a()->array_length(), os);
            } else {
                throw "[CFG error] unexpected instruction in function define unit";
            }
        } catch (const char *msg) {
            cerr << msg << '\n';
        }
    }
}
void IRTestManager::gen_function_def_tail(const IRInstrList &list, ostream &os)
{
    os << "}" << endl;
}
static char float_point_buf[40];
void IRTestManager::gen_global_var_def_unit(const IRInstrList &list, std::ostream &os)
{
    try {
        for (const auto &instr : list) {
            if (instr.type() != IRType::GlobalDecl) {
                throw "[CFG error] found non-declare instruction in global define unit";
                continue;
            }
            auto symbol = instr.a()->global_sym();
            os << get_c_type_str(symbol->get_val_type().basic()) << " " << symbol->get_name();
            if (symbol->is_array()) {
                os << "[" << symbol->get_val_type().total_length() << "]";
            }
            if (symbol->is_literally_initialized()) {
                os << " = ";
                const auto &init_val = symbol->get_init_value();
                if (symbol->is_array()) {
                    os << "{";
                    const int tot_size = symbol->get_val_type().total_length();
                    int prev_pos = -1;
                    for (const auto &init_unit : init_val) {
                        while (init_unit.pos > prev_pos + 1) {
                            os << "0,";
                            prev_pos++;
                        }
                        if (symbol->get_val_type().basic() == BasicType::Float) {
                            sprintf(float_point_buf, "%a", init_unit.val.float_value);
                            os << float_point_buf << "f";
                        }
                        else
                            os << init_unit.val.int_value;
                        prev_pos++;
                        if (init_unit.pos + 1< tot_size)
                            os << ",";
                    }
                    os << "}";
                } else {
                    if (symbol->get_val_type().basic() == BasicType::Float) {
                        sprintf(float_point_buf, "%a", init_val[0].val.float_value);
                        os << float_point_buf << "f";
                    }
                    else
                        os << init_val[0].val.int_value;
                }
            }
            os << ";" << endl;
        }
    } catch (const char *msg) {
        cerr << msg << endl;
    }
}

static string get_oprand_rval_str(IRSymbol* oprand)
{
    stringstream ss;
    const auto type = oprand->kind();
    const auto vtype = oprand->basic_type();
    if (type == IRSymbolKind::Global) {
        ss << oprand->global_sym()->name();
    }else if (type == IRSymbolKind::Local) {
        ss << "l" << oprand->index();
    }else if (type == IRSymbolKind::Param) {
        ss << "l" << oprand->index();
    } else if (type == IRSymbolKind::Temp) {
        ss << "t" << oprand->index();
    } else if (type == IRSymbolKind::Value) {
        if (vtype == BasicType::Int)
            ss << oprand->value().int_value;
        else if (vtype == BasicType::Float) {
            sprintf(float_point_buf, "%a", oprand->value().float_value);
            ss << float_point_buf<<"f";
            //ss  << setiosflags(ios::scientific)<< setprecision(10) << oprand->value().float_value<<"f";
        }
            
    }
    return ss.str();
}
static string get_oprand_lval_str(IRSymbol* oprand)
{
    stringstream ss;
    const auto type = oprand->kind();
    const auto vtype = oprand->basic_type();
    if (type == IRSymbolKind::Global) {
        ss << oprand->global_sym()->name() << " = ";
    } else if (type == IRSymbolKind::Local) {
        ss << "l" << oprand->index()<<" = ";
    }else if (type == IRSymbolKind::Param) {
        ss << "l" << oprand->index()<<" = ";
    } else if (type == IRSymbolKind::Temp) {
        ss << get_c_type_str(vtype,oprand->array_length()==IRArrayLength::IR_ARRAY_POINTER) << " t" << oprand->index() << " = ";
    }
    return ss.str();
}
static ostream &operator<<(ostream &os, IROper op)
{
    switch (op){
    case IROper::Null:return os << "?";
    case IROper::AddI:return os << "+";
    case IROper::AddF:return os << "+";
    case IROper::SubI:return os << "-";
    case IROper::SubF:return os << "-";
    case IROper::MulI:return os << "*";
    case IROper::MulF:return os << "*";
    case IROper::DivI:return os << "/";
    case IROper::DivF:return os << "/";
    case IROper::ModI:return os << "%";
    case IROper::NegI:return os << "-";
    case IROper::NegF:return os << "-";
    case IROper::IToF:return os << "(float)";
    case IROper::FToI:return os << "(int)";
    case IROper::EqualI:return os << "==";
    case IROper::EqualF:return os << "==";
    case IROper::NotEqualI:return os << "!=";
    case IROper::NotEqualF:return os << "!=";
    case IROper::GreaterI:return os << ">";
    case IROper::GreaterF:return os << ">";
    case IROper::LessI:return os << "<";
    case IROper::LessF:return os << "<";
    case IROper::GreaterEqualI:return os << ">=";
    case IROper::GreaterEqualF:return os << ">=";
    case IROper::LessEqualI:return os << "<=";
    case IROper::LessEqualF:return os << "<=";
    case IROper::NotI:return os << "!";
    case IROper::NotF:return os << "!";
    default:
        break;
    };
    return os;
}
void IRTestManager::gen_block(IRBlock *block, std::ostream &os)
{
    queue<IRSymbol*> rparam_queue;
    os << "ir2c_blk_" << block->get_index() << ":" << std::endl;
    os << "    ;" << std::endl; //添加空语句，以避免gcc编译ir2c报错
    const auto &program = block->get_instr_list_const();
    for (const auto &instr : program) {
        if (instr.type() != IRType::RParam)
            os << "    ";
        try {
            switch (instr.type()) {
            case IRType::BinaryCalc:
                os << get_oprand_lval_str(instr.r())
                   << get_oprand_rval_str(instr.a()) << " " << instr.op() << " " << get_oprand_rval_str(instr.b()) << ";" << endl;
                break;
            case IRType::UnaryCalc:
                os << get_oprand_lval_str(instr.r()) << " " << instr.op() << " " << get_oprand_rval_str(instr.a()) << ";" << endl;
                break;
            case IRType::Assign:
                os << get_oprand_lval_str(instr.r()) << get_oprand_rval_str(instr.a()) << ";" << endl;
                break;
            case IRType::ArrayLoad: {
                os << get_oprand_lval_str(instr.r()) << get_oprand_rval_str(instr.a()) << "[" << get_oprand_rval_str(instr.b()) << "];" << endl;
                break;
            }
            case IRType::ArrayStore: {
                os << get_oprand_rval_str(instr.r()) << "[" << get_oprand_rval_str(instr.a()) << "] = " << get_oprand_rval_str(instr.b()) << ";" << endl;
                break;
            }
            case IRType::RParam:
                rparam_queue.push(instr.a());
                break;
            case IRType::CallWithRet:
                os << get_oprand_lval_str(instr.r());
                [[fallthrough]];//不需要break,可以直接落到下一层
            case IRType::Call: {
                os << instr.a()->global_sym()->name() << "(";
                while (!rparam_queue.empty()) {
                    auto now = rparam_queue.front();
                    rparam_queue.pop();
                    os << get_oprand_rval_str(now);
                    if (!rparam_queue.empty())
                        os << ",";
                }
                os << ");" << endl;
                break;
            }
            case IRType::Return:
                os << "return;" << endl;
                break;
            case IRType::ValReturn:
                os << "return " << get_oprand_rval_str(instr.a()) << ";" << endl;
                break;
            case IRType::BlockGoto:
                os << "goto "
                   << "ir2c_blk_" << block->get_succ(0)->get_index() << ";" << endl;
                break;
            case IRType::BlockCondGoto:
                os << "if(" << get_oprand_rval_str(instr.a()) << ") goto ir2c_blk_" << block->get_succ(1)->get_index() << ";else goto ir2c_blk_" << block->get_succ(0)->get_index() << ";" << endl;
                break;
            default:
                throw "[CFG error] unexpected instruction in block";
                break;
            }
        } catch (const char *msg) {
            cerr << msg << '\n';
        }
    }
    //如果基本块不是以goto或者return结尾，则需要加入跳转语句
    if (block->get_instr_list_const().empty()||
        (block->get_instr_list_const().back().type() != IRType::BlockGoto &&
        block->get_instr_list_const().back().type() != IRType::BlockCondGoto &&
        block->get_instr_list_const().back().type() != IRType::Return &&
        block->get_instr_list_const().back().type() != IRType::ValReturn)) {
        os << "    goto "
           << "ir2c_blk_" << block->get_succ(0)->get_index() << ";" << endl;
    }
}
void IRTestManager::gen_cfg(IRBlock *entry, std::ostream &os)
{
    std::set<IRBlock *> is_printed;
    std::queue<IRBlock *> q;
    q.push(entry);
    while (!q.empty()) {
        IRBlock *now = q.front();
        q.pop();
        if (!now->is_exit()&&!now->is_entry())
            gen_block(now, os);
        for (int k = 0; k <= 1; ++k)
            if (now->get_succ(k) != nullptr && is_printed.find(now->get_succ(k)) == is_printed.end()) {
                q.push(now->get_succ(k));
                is_printed.insert(now->get_succ(k));
            }
    }
}
void IRTestManager::gen_c_source(const IRProgram &prog, ostream &os)
{
    //os << "#include\"sylib.h\"" << endl;
    for (auto &unit : prog) {
        if (unit.get_type() == IRUnitType::FuncDef) {
            gen_function_def_head(unit.get_definations_const(), os);
            gen_cfg(unit.get_entry(), os);
            gen_function_def_tail(unit.get_definations_const(), os);
        } else {
            gen_global_var_def_unit(unit.get_definations_const(), os);
        }
    }
}