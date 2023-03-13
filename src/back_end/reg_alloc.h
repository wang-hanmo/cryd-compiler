#pragma once
#include <ir_symbol_table.h>
#include <ir_define.h>
#include <cfg.h>
#include <vector>
#include <bitmap.h>
#include <linear_ir_manager.h>
#include <arm_define.h>
#include <stack>
#include <arm.h>
#include <live.h>
#include <interval.h>

class RIG
{
private:
    int m_unit_no;
    int m_var_count;
    IRInstrList m_mov_instr;
    std::vector<IRSymbol*> m_symbol;                //符号顶点
    std::vector<int> m_interfere_degree;            //顶点冲突边的度
    std::vector<int> m_move_degree;                 //顶点移动边的度
    std::vector<BitMap> m_interfere;                //冲突边邻接矩阵
    std::vector<BitMap> m_move;                     //移动边邻接矩阵
    BitMap m_exist_flag{BitMap(1)};                 //符号在冲突图中的标志
    std::unordered_map<IRSymbol*, int> m_search;    //符号查找索引
    std::set<int> m_move_unrelated;                 //move无关符号集合
    std::set<int> m_move_related;                   //move有关符号集合
    std::stack<int> m_select_stack;                 //选择栈
    std::stack<int> m_spill_stack;                  //溢出栈
    std::vector<std::pair<int, int>> m_coalesce;    //合并的结点及其映射
    std::vector<int> m_spill_time;                  //符号出现次数
    std::unordered_map<IRSymbol*, IRSymbol*> m_fparam_memory;    //形参查找索引
public:
    RIG(int i_unit_no, std::vector<IRSymbol*> i_symbol);
    void set_unit_no(int i_unit_no)                     {m_unit_no = i_unit_no;}
    void add_mov_instr(IRInstr i_mov_instr)         {m_mov_instr.push_back(i_mov_instr);}
    void add_symbol(IRSymbol* i_symbol)                 {m_symbol.push_back(i_symbol);}
    void add_interfere(BitMap i_interfere)              {m_interfere.push_back(i_interfere);}
    void add_move(BitMap i_move)                        {m_interfere.push_back(i_move);}
    void add_coalesce(int i, int j)                     {m_coalesce.push_back(std::pair<int, int>(i, j));}
    void add_fparam_memory(IRSymbol* sym, IRSymbol* mem)       {m_fparam_memory.emplace(sym, mem);}
    int get_unit_no()                                   {return m_unit_no;}
    std::vector<IRSymbol*>& get_symbol()                {return m_symbol;}
    IRSymbol*& get_symbol(int i)                        {return m_symbol[i];}
    std::vector<BitMap>& get_interfere()                {return m_interfere;}
    std::vector<BitMap>& get_move()                     {return m_move;}
    void increase_spill_time(int i, int t = 1)          {m_spill_time[i] += t;}
    int get_spill_time(int i)                           {return m_spill_time[i];}
    std::vector<int>& get_spill_time()                  {return m_spill_time;}
    int size()                                          {return m_var_count;}
    void set_interfere(int i, int j);
    void reset_interfere(int i, int j);
    bool test_interfere(int i, int j);
    bool test_interfere_abs(int i, int j)               {return m_interfere[i].test(j);}
    void set_move(int i, int j);
    void reset_move(int i, int j);
    bool test_move(int i, int j);
    bool test_move_abs(int i, int j)                    {return m_move[i].test(j);}
    bool is_rig_empty()                                 {return m_exist_flag.empty();}
    void delete_node(int i);
    void insert_node(int i);
    bool test_node(int i)                               {return m_exist_flag.test(i);}
    BitMap& get_exist()                                 {return m_exist_flag;}
    void insert_move_related(int symid)                 {m_move_related.emplace(symid);}
    void delete_move_related(int symid)                 {m_move_related.erase(symid);}
    bool is_move_related(int symid)                     {return m_move_related.count(symid);}
    std::set<int>& get_move_related()                   {return m_move_related;}
    void insert_move_unrelated(int symid)               {m_move_unrelated.emplace(symid);}
    void delete_move_unrelated(int symid)               {m_move_unrelated.erase(symid);}
    bool is_move_unrelated(int symid)                   {return m_move_unrelated.count(symid);}
    std::set<int>& get_move_unrelated()                 {return m_move_unrelated;}
    void push_select_stack(int symid)                   {m_select_stack.push(symid);}
    void pop_select_stack()                             {m_select_stack.pop();}
    int top_select_stack()                              {return m_select_stack.top();}
    bool empty_select_stack()                           {return m_select_stack.empty();}
    void push_spill_stack(int symid)                    {m_spill_stack.push(symid);}
    void pop_spill_stack()                              {m_spill_stack.pop();}
    int top_spill_stack()                               {return m_spill_stack.top();}
    bool empty_spill_stack()                            {return m_spill_stack.empty();}
    int get_interfere_degree(int symid)                 {return m_interfere_degree[symid];}
    int get_move_degree(int symid)                      {return m_move_degree[symid];}
    IRInstrList& get_mov_instr()                        {return m_mov_instr;}
    std::vector<std::pair<int, int>>& get_coalesce()    {return m_coalesce;}
    std::unordered_map<IRSymbol*, IRSymbol*>& get_fparam_memory() {return m_fparam_memory;}
    IRSymbol* fparam_memory(IRSymbol* symbol)   {return m_fparam_memory.count(symbol) ? m_fparam_memory.at(symbol) : nullptr;}
    int coalesce_idx(int symid);
    int symbol_idx(IRSymbol* symbol);
    void print_rig();
};

class RAT   // Register Allocation Table
{
private:
    //分配方法
    int m_mode_r;
    int m_mode_s;
    //IRUnit编号
    int m_unit_no;
    //IR符号
    std::vector<IRSymbol*> m_ir_symbol;
    //寄存器号
    std::vector<RegID> m_reg_id;
    //spill情况
    std::vector<bool> m_spill;
    //IR符号活跃区间
    std::vector<Interval> m_sym_live_intervals;
    //寄存器活跃区间
    std::vector<Interval> m_reg_live_intervals;
    //IRUnit起始行号
    int m_start_no;
    //IRUnit结束行号
    int m_end_no;
    //散列查找
    std::unordered_map<IRSymbol*, int> m_search_map;
    //溢出符号
    std::set<IRSymbol*> m_spill_symbol;
    //溢出内存分配
    std::unordered_map<IRSymbol*, IRSymbol*> m_spill_memory;
public:
    void set_unit_no(int i_unit_no)                             {m_unit_no = i_unit_no;}
    void set_mode(int i_type, int i_mode)                       {if(i_type) m_mode_s = i_mode; else m_mode_r = i_mode;}
    void set_start_no(int i_start_no)                           {m_start_no = i_start_no;}
    void set_end_no(int i_end_no)                               {m_end_no = i_end_no;}
    void set_ir_symbol(std::vector<IRSymbol*> i_ir_symbol)      {m_ir_symbol = i_ir_symbol;}
    void set_reg_id(std::vector<RegID> i_reg_id)                {m_reg_id = i_reg_id;}
    void set_spill(std::vector<bool> i_spill)                   {m_spill = i_spill;}
    void set_sym_live_intervals(std::vector<Interval> i_sym_live_intervals)   {m_sym_live_intervals = i_sym_live_intervals;}
    void set_reg_live_intervals(std::vector<Interval> i_reg_live_intervals)   {m_reg_live_intervals = i_reg_live_intervals;}
    void set_map(std::unordered_map<IRSymbol*,int> map)         {m_search_map = map;}
    void set_spill_symbol(std::set<IRSymbol*> i_spill_symbol)         {m_spill_symbol = i_spill_symbol;}
    void set_spill_memory(std::unordered_map<IRSymbol*,IRSymbol*> i_spill_memory)         {m_spill_memory = i_spill_memory;}
    void init_reg_id(int type, int n)                           {std::vector<RegID> t(n,type ? ALLOCATABLE_REGISTERS_S[0] : ALLOCATABLE_REGISTERS[0]); m_reg_id = t;}
    void init_spill(int n)                                      {std::vector<bool> t(n,true); m_spill = t;}
    void init_reg_live_intervals(int n, int len)                {std::vector<Interval> t(n, Interval(len)); m_reg_live_intervals = t;}
    void init_map()                                             {m_search_map.clear();}
    void init_spill_symbol()                                    {m_spill_symbol.clear();}
    void init_spill_memory()                                    {m_spill_memory.clear();}
    void add_ir_symbol(IRSymbol* i_ir_symbol)                   {m_ir_symbol.push_back(i_ir_symbol);}
    void add_reg_id(RegID i_reg_id)                             {m_reg_id.push_back(i_reg_id);}
    void add_spill(bool i_spill)                                {m_spill.push_back(i_spill);}
    void add_sym_live_intervals(Interval i_sym_live_intervals)    {m_sym_live_intervals.push_back(i_sym_live_intervals);}
    void add_reg_live_intervals(Interval i_reg_live_intervals)    {m_reg_live_intervals.push_back(i_reg_live_intervals);}
    void add_spill_symbol(IRSymbol* symbol)                     {m_spill_symbol.emplace(symbol);}
    void add_spill_memory(IRSymbol* symbol, IRSymbol* memory)   {m_spill_memory.emplace(symbol, memory);}
    int get_unit_no()                                           {return m_unit_no;}
    int get_mode(int i_type)                                    {return (i_type ? m_mode_s : m_mode_r);}
    int get_start_no()                                          {return m_start_no;}
    int get_end_no()                                            {return m_end_no;}
    std::vector<IRSymbol*>& get_ir_symbol()                     {return m_ir_symbol;}
    IRSymbol*& get_ir_symbol(int i)                             {return m_ir_symbol[i];}
    std::vector<RegID>& get_reg_id()                            {return m_reg_id;}
    RegID& get_reg_id(int i)                                    {return m_reg_id[i];}
    std::vector<bool>& get_spill()                              {return m_spill;}
    bool get_spill(int i)                                       {return m_spill[i];}
    std::vector<Interval>& get_sym_live_intervals()               {return m_sym_live_intervals;}
    Interval& get_sym_live_intervals(int i)                       {return m_sym_live_intervals[i];}
    std::vector<Interval>& get_reg_live_intervals()               {return m_reg_live_intervals;}
    Interval& get_reg_live_intervals(int i)                       {return m_reg_live_intervals[i];}
    std::unordered_map<IRSymbol*,int>& get_map()                {return m_search_map;}
    std::set<IRSymbol*>& get_spill_symbol()                     {return m_spill_symbol;}
    bool spill_symbol(IRSymbol* symbol)                         {return m_spill_symbol.count(symbol);}
    std::unordered_map<IRSymbol*,IRSymbol*>& get_spill_memory() {return m_spill_memory;}
    IRSymbol* spill_memory(IRSymbol* symbol)                    {return m_spill_memory.count(symbol) ? m_spill_memory.at(symbol) : nullptr;}
    int size()                                                  {return m_ir_symbol.size();}
    void set_reg_id(int i, RegID i_reg_id)                      {m_reg_id[i] = i_reg_id;}
    void set_spill(int i, bool i_spill)                         {m_spill[i] = i_spill;}
    void emplace_map(IRSymbol* symbol, int index)               {m_search_map.emplace(symbol, index);}
    int at_map(IRSymbol* symbol);                                   
    void print_rat(std::ostream &os);
    bool is_spill(IRSymbol* symbol);
    RegID reg_alloc(IRSymbol* symbol);
    bool test_sym_live(IRSymbol* symbol,int no);
    bool test_reg_live(RegID reg,int no);
    bool test_reg_used(RegID reg);
};

class RegisterAllocator
{
private:
    //迭代次数
    static int ra_time;
    //模式 0：图着色， 1：线性扫描
    int m_mode{0};
    //类型 0：整型，1：浮点型
    int m_type{0};
    //活跃区间
    std::list<LiveIntervals*> m_li;
    //分配方案
    std::list<RAT*> m_allocation;
    //符号数
    int m_var_count;
    //参数寄存器
    std::vector<IRSymbol*> m_argreg;
    //控制流图&低级IR
    IRProgram* m_cfg;
    //IR符号表
    IRSymbolTable* m_ir_sym_table;

    
    static int mem_idx;
    static int tmp_idx;
    
    bool m_done;

    void spilt_li(LiveIntervals* li, LiveIntervals** r_li, LiveIntervals** s_li);
    RAT* merge_rat(RAT* r_rat, RAT* s_rat);
    int spill_cost(int t);

    /****************图着色****************/

    RIG* build(LiveIntervals* li, IRUnit unit);             //构造冲突图
    void simplify(RIG* rig);                                //简化
    bool coalesce(RIG* rig, LiveIntervals* li);             //合并移动边
    bool freeze(RIG* rig);                                  //冻结
    bool spill(RIG* rig);                                   //溢出
    RAT* select(RIG* rig, LiveIntervals* li);               //选择寄存器
    void rewrite(RIG* rig, RAT* rat, IRUnit unit);           //重写指令

    RAT* coloring_allocate(LiveIntervals* li, IRUnit unit);
    /****************线性扫描****************/
    //堆排序函数
    void swap(LiveIntervals* li, int i, int j);
    void heapify(LiveIntervals* li, int n, int i);
    void build_heap(LiveIntervals* li);

    void heap_sort(LiveIntervals* li);                      //排序
    RAT* scan(LiveIntervals* li);                           //线性扫描

    RAT* linear_allocate(LiveIntervals* li);
public:
    RegisterAllocator(std::list<LiveIntervals*> i_li, std::vector<IRSymbol*> i_argreg, IRProgram* i_cfg, IRSymbolTable* i_ir_sym_table, int i_mode = 0):
    m_li(i_li), m_argreg(i_argreg), m_cfg(i_cfg), m_ir_sym_table(i_ir_sym_table), m_mode(i_mode), m_done(true){}
    std::list<RAT*> allocate_register();
    static void print_allocation(std::list<RAT*> allocation, std::ostream &os = std::cout);
    static void print_rig_list(std::list<RIG*> rig_list);
    static void register_error(std::string msg, int error_code);
    bool is_done()  {return m_done;}
};