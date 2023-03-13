#pragma once
#include<ir_instr.h>
#include<vector>
#include<array>
#include<set>
#include<list>
#include<iostream>
enum class IRUnitType :char
{
    FuncDef = 0,
    VarDef = 1
};
class IRBlock
{
private:
    static int s_index;                        //用于分配index的变量
    std::multiset<IRBlock*> m_anti_edge{};     //反向图的邻接表
    std::array<IRBlock*, 2> m_edge{};          //正向图的邻接表，下标0表示false转移或无条件转移，下标1表示true转移
    IRInstrList m_instr_list;                  //IR指令列表
    int m_index{};                             //用于在输出时提高可读性的标记，仅用于输出，不可用于别的用途
    int m_tag{-1};                             //附加信息，在各种优化算法中使用，生命周期仅限于某个算法内
    /**********控制流分析信息**********/
    IRBlock* m_idom{nullptr};                  //该节点的直接支配节点(支配树上的前驱节点)
    std::vector<IRBlock*> m_idom_child{};      //支配树上的子节点集合
    //从反向边表中删除指定边
    void erase_from_anti_edge(IRBlock* target);
public:
    void add_instr(const IRInstr& instr);
    void add_instr_to_front(const IRInstr& instr);
    IRBlock() { m_edge[0] = m_edge[1] = nullptr; m_instr_list = IRInstrList(); m_index = s_index++; }
    //将唯一的一条活边挪到无条件转移的位置
    void adjust_edge_to_uncond();
    //设置边的函数，同时也会在目标节点添加反向边
    void set_edge(int index,IRBlock* to);
    //从邻接表中删除某条边,同时也会将目标节点中的反向边删除
    void delete_edge(int index);
    //从邻接表中删除某条边，但不考虑目标节点的反向边
    void delete_edge_oneway(int index);
    //维护支配树信息
    void set_idom(IRBlock* idom) { m_idom = idom; };
    void clear_idom_child() { m_idom_child.clear(); }
    void add_idom_child(IRBlock* child) { m_idom_child.push_back(child);}
    void set_index(int index) { m_index = index; }
    void set_tag(int tag) { m_tag = tag; }
    //get函数
    IRBlock* get_idom() { return m_idom; }
    std::vector<IRBlock*> get_idom_child() { return m_idom_child; }
    int get_tag() { return m_tag; }//attach info
    int get_index() { return m_index; }
    const IRInstrList& get_instr_list_const()const{return m_instr_list;}
    IRInstrList& get_instr_list() { return m_instr_list; }
    IRBlock* get_succ(int index=0)const {return m_edge[index];}         //获取后继节点
    const std::multiset<IRBlock*>& get_pred()const { return m_anti_edge; }  //获取前驱节点列表
    int in_degree()const { return m_anti_edge.size(); }    //入度
    int out_degree()const {return (m_edge[0]!=nullptr?1:0)+(m_edge[1]!=nullptr?1:0);}   //出度
    bool is_entry()const { return m_anti_edge.empty(); }//是否为入口
    bool is_exit()const {return m_edge[0]==nullptr&&m_edge[1]==nullptr;}      //是否为出口
    static void reset_index() { s_index = 0; }
    static void delete_graph(IRBlock* target);//释放包括自身节点在内的整个连通子图
};
/*
    IRUnit
    IRProgram由一个或多个IRUnit组成
*/
class IRUnit
{
private:
    IRBlock* m_entry;                           //表示虚拟的入口块，内部为空，VarDef节点此字段无效
    IRBlock* m_exit;                            //表示虚拟的出口块，内部为空，VarDef节点此字段无效
    IRInstrList m_definations;                  //存放所有定义语句，包括全局变量 or 参数和局部变量
    IRUnitType m_type;                          //功能类型
    int m_tag;                                  //附加信息，在各种优化算法中使用，生命周期仅限于某个算法内
    bool m_is_dominator_tree_info_valid{false}; //支配树信息是否有效(一些优化算法会添加/删除节点，可能导致已经求出的支配树信息失效)
public:
    IRUnit(IRUnitType i_type,IRBlock* i_entry=nullptr, IRBlock* i_exit=nullptr):m_type(i_type),m_entry(i_entry),m_exit(i_exit){
        m_definations = IRInstrList();
    }
    ~IRUnit(){}
    //<t><l><p>型编号的最大值
    int find_max_index_for_tlp_var();
    //构造新FuncDef型IRUnit，并为m_entry和m_exit分配空间
    static IRUnit create_func_def();
    //构造新VarDef型IRUnit
    static IRUnit create_var_def();
    void add_def_instr(const IRInstr& instr);
    //set
    void set_dominator_tree_info_valid(bool valid) { m_is_dominator_tree_info_valid = valid; }
    void set_tag(int tag) { m_tag = tag; }
    void set_entry(IRBlock* i_entry) {m_entry = i_entry;}
    void set_exit(IRBlock* i_exit) {m_exit = i_exit;}
    //get
    IRBlock* get_entry()const {return m_entry;}
    IRBlock* get_exit()const {return m_exit;}
    IRUnitType get_type()const {return m_type;}
    int get_tag() { return m_tag; }//attach info
    bool is_dominator_tree_info_valid() { return m_is_dominator_tree_info_valid; }
    const IRInstrList& get_definations_const()const {return m_definations;}
    IRInstrList& get_definations()  {return m_definations;}
    static IRUnit* clone(IRUnit unit);
};
using IRProgram=std::list<IRUnit>;