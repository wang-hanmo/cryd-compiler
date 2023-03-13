#pragma once
#include <ir_symbol_table.h>
#include <ir_define.h>
#include <cfg.h>
#include <vector>
#include <bitmap.h>
#include <linear_ir_manager.h>
#include <queue>
#include <set>
#include <arm.h>
#include <interval.h>

class LiveIntervals
{
private:
    int m_unit_no;
    std::vector<IRSymbol*> m_symbol;        //符号顶点
    std::vector<Interval> m_live_intervals;   //活跃区间，辅助生成汇编
    std::vector<int> m_first_no;
    std::vector<int> m_last_no;
    int m_start_no;
    int m_end_no;
public:
    LiveIntervals() 
        {m_symbol.clear();m_live_intervals.clear();m_first_no.clear();m_last_no.clear();}
    void set_unit_no(int i_unit_no)                     {m_unit_no = i_unit_no;}
    void set_start_no(int i_start_no)                     {m_start_no = i_start_no;}
    void set_end_no(int i_end_no)                     {m_end_no = i_end_no;}
    void set_first_no(int i, int no)                     {m_first_no[i] = no;}
    void set_last_no(int i, int no)                     {m_last_no[i] = no;}
    void set_symbol(int i, IRSymbol* i_symbol)          {m_symbol[i] = i_symbol;}
    void set_live_intervals(int i, Interval i_live_intervals)    {m_live_intervals[i] = i_live_intervals;}
    void set_first_no(std::vector<int> i_first_no)         {m_first_no = i_first_no;}
    void set_last_no(std::vector<int> i_last_no)           {m_last_no = i_last_no;}
    void set_symbol(std::vector<IRSymbol*> i_symbol)    {m_symbol = i_symbol;}
    void set_live_intervals(std::vector<Interval> i_live_intervals)    {m_live_intervals = i_live_intervals;}
    int get_unit_no()                                   {return m_unit_no;}
    int get_start_no()                                   {return m_start_no;}
    int get_end_no()                                   {return m_end_no;}
    int get_first_no(int i)                            {return m_first_no[i];}
    int get_last_no(int i)                             {return m_last_no[i];}
    void add_first_no(int no)                       {m_first_no.push_back(no);}
    void add_last_no(int no)                        {m_last_no.push_back(no);}
    void add_symbol(IRSymbol* i_symbol)                 {m_symbol.push_back(i_symbol);}
    void add_live_intervals(Interval i_live_intervals)    {m_live_intervals.push_back(i_live_intervals);}
    std::vector<IRSymbol*>& get_symbol()                {return m_symbol;}
    IRSymbol*& get_symbol(int i)                        {return m_symbol[i]; }
    std::vector<Interval>& get_live_intervals()           {return m_live_intervals;}
    Interval& get_live_intervals(int i)                   {return m_live_intervals[i];}
    int size()                                          {return m_symbol.size();}
    bool test_live_intervals(int i, int j)              {return m_live_intervals[i].test(j);}
    void print_li(std::ostream &os = std::cout);

};

class LiveManager
{
private:
    //IRUnit中的所有IRBlock(bfs序)
    std::vector<IRBlock*> m_ir_block;
    //IRUnit中的所有IRSymbol,包括Global、Local、Temp 
    std::vector<IRSymbol*> m_ir_symbol;
    //参数寄存器符号
    std::vector<IRSymbol*> m_argreg;
    //控制流图
    IRProgram* m_cfg;

    //按block索引vector,按symbol索引bitmap
    std::vector<BitMap> m_live_def;
    std::vector<BitMap> m_live_use;
    std::vector<BitMap> m_live_in;
    std::vector<BitMap> m_live_out;

    //按symbol索引vector,按no索引bitmap
    //interval表示变量活跃的区间
    std::vector<Interval> m_live_intervals;
    
    //IRInstr序号，initial时会对按bfs序重新赋序号
    int start_no;
    int end_no;
    int no;
    //索引tag编号
    int blk_idx;
    int sym_idx;
    // visited维护
    std::set<IRBlock*> blk_visited;
    std::set<IRSymbol*> sym_visited;
    //活跃区间范围
    std::vector<int> first_no;
    std::vector<int> last_no;

    //活跃区间总表 
    std::list<LiveIntervals*> m_li;

    void compute_block(IRBlock* block);      //按bfs搜索IRBlock，并对IRUnit编号、收集IRSymbol
    void initial(IRUnit unit);               //初始化
    void compute_live_def_and_use();         //计算LiveDef和LiveUse
    void compute_live_in_and_out();          //计算LiveIn和LiveOut
    void compute_live_intervals();           //计算Live Interval
    void print_liveness();                   //打印Live Interval
    LiveIntervals* build();                  //建立活跃区间总表

public:
    LiveManager(IRProgram* cfg, std::vector<IRSymbol*> argreg):m_cfg(cfg), m_argreg(argreg){}
    //变量活跃区间分析主函数
    std::list<LiveIntervals*>& compute_liveness();
    //打印冲突图
    static void print_li_list(std::list<LiveIntervals*> li_list, std::ostream &os = std::cout);
    static bool check_symbol_in_instr(IRInstr instr, IRSymbol* symbol);
    static bool check_move_edge_conflict(IRInstr instr);
    static void live_error(std::string msg, int error_code);
};