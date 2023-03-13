#include <loop_unrolling.h>
#include <cfg_manager.h>
#include <cfg.h>
#include <algorithm>
#include <map>
#include <sstream>
#include <iterator>
#include <linear_ir_manager.h>

void LoopUnrolling::resize_table()
{
    m_stop_spread_sym.resize(m_sym_count);
    m_use_def_chain.resize(m_sym_count);
    m_local_temp_index_mp.resize(m_sym_count);
    m_local_ssa_index_mp.resize(m_sym_count);
    m_ssa_sym_index_mp.resize(m_sym_count,0);
    m_sym_value_table.resize(m_sym_count,0);
    m_has_sym_value_table.resize(m_sym_count,false);
    m_is_stop_spread.resize(m_sym_count,false);
}


void LoopUnrolling::clear_table()
{
    m_use_def_chain.clear();
    m_local_temp_index_mp.clear();
    m_local_ssa_index_mp.clear();
    m_ssa_sym_index_mp.clear();
    m_sym_value_table.clear();
    m_has_sym_value_table.clear();
    m_visited_instr.clear();
    m_store_instrlist.clear();
    m_stop_spread_sym.clear();
    m_is_stop_spread.clear();
}

bool LoopUnrolling::is_ssa_var(IRSymbol* sym)
{
    return ((sym->kind() == IRSymbolKind::Param) ||
            (sym->kind() == IRSymbolKind::Local) ||
            sym->kind() == IRSymbolKind::Temp);
}

bool LoopUnrolling::has_lvalue(IRInstr& instr)
{
    if (instr.type() == IRType::Assign || instr.type() == IRType::ArrayLoad || instr.type() == IRType::PhiFunc
        || instr.type() == IRType::BinaryCalc || instr.type() == IRType::UnaryCalc || instr.type() == IRType::CallWithRet) {
        return is_ssa_var(instr.r());
    }
    return false;
}

void LoopUnrolling::initial_instr_sym_tag(IRInstrList& program)
{
    for(auto& def_instr : program) 
    {
        if (has_lvalue(def_instr)) 
        {
            def_instr.r()->set_tag(m_sym_count++);        
            m_use_def_chain.push_back(&def_instr);
        }
    }
}

void LoopUnrolling::initial_decl_sym_tag(IRInstrList& program)
{
    for(auto& def_instr : program) 
    {
        if (is_ssa_var(def_instr.a())) 
        {
            def_instr.a()->set_tag(m_sym_count);
            def_instr.a()->def_sym()->set_tag(m_sym_count++);      
            m_use_def_chain.push_back(&def_instr);
        }
    }
}


void LoopUnrolling::ir_list_find_index(IRInstrList& program)
{
   for(auto&instr:program)
   {
        if(instr.r()!=nullptr){
            if(instr.r()->kind()==IRSymbolKind::Temp)
            {
                m_max_temp_index = std::max(m_max_temp_index,instr.r()->index()+1);
            }
            else if(instr.r()->ssa_index()>=0)
            {
                 int index = instr.r()->ssa_index();
                 IRSymbol* sym = instr.r()->def_sym();
                 int tmp_index = m_ssa_sym_index_mp[sym->get_tag()];
                 m_ssa_sym_index_mp[sym->get_tag()] = std::max(index + 1,tmp_index);
            }
        }
        if(instr.a()!=nullptr){
            if(instr.a()->kind() == IRSymbolKind::Temp)
            {
                m_max_temp_index = std::max(m_max_temp_index,instr.a()->index()+1);
            }
            else if(instr.a()->ssa_index()>=0)
            {
                 int index = instr.a()->ssa_index();
                 IRSymbol* sym = instr.a()->def_sym();
                 int tmp_index = m_ssa_sym_index_mp[sym->get_tag()];
                 m_ssa_sym_index_mp[sym->get_tag()] = std::max(index + 1,tmp_index);
            }
        }
        if(instr.b()!=nullptr){
            if(instr.b()->kind() == IRSymbolKind::Temp)
            {
                 m_max_temp_index = std::max(m_max_temp_index,instr.b()->index()+1);
            }
            else if(instr.b()->ssa_index()>=0)
            {
                 int index = instr.b()->ssa_index();
                 IRSymbol* sym = instr.b()->def_sym();
                 int tmp_index = m_ssa_sym_index_mp[sym->get_tag()];
                 m_ssa_sym_index_mp[sym->get_tag()] = std::max(index + 1,tmp_index);
            }
        }
   }
}

void LoopUnrolling::rebind_temp_index(IRInstr& instr)
{
    if(instr.a()!=nullptr&&instr.a()->kind()==IRSymbolKind::Temp)
    {
        int index = instr.a()->get_tag();
        if(m_local_temp_index_mp[index])
        {
        instr.rebind_a(m_local_temp_index_mp[index]);
        }
    }
    if(instr.b()!=nullptr&&instr.b()->kind()==IRSymbolKind::Temp)
    {
        int index = instr.b()->get_tag();
        if(m_local_temp_index_mp[index])
        {
        instr.rebind_b(m_local_temp_index_mp[index]);
        }
    }
    if(instr.r()!=nullptr&&instr.r()->kind()==IRSymbolKind::Temp)
    {
        int index = instr.r()->get_tag();
        if(instr.r()->array_length()==IRArrayLength::IR_NONE_ARRAY)
        {
            if(instr.r()->basic_type()==BasicType::Int)
                instr.rebind_r(m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++));
            else 
                instr.rebind_r(m_ir_sym_table->create_temp(BasicType::Float,m_max_temp_index++));
        }
        else{
            if(instr.r()->basic_type()==BasicType::Int)
                instr.rebind_r(m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++,IRArrayLength::IR_ARRAY_POINTER));
            else 
                instr.rebind_r(m_ir_sym_table->create_temp(BasicType::Float,m_max_temp_index++,IRArrayLength::IR_ARRAY_POINTER));
        }
        m_local_temp_index_mp[index] = instr.r();
    }
}

void LoopUnrolling::rebind_ssa_index(IRInstr& instr)
{
    if(instr.a()!=nullptr&&instr.a()->ssa_index()>=0)
    {
        IRSymbol* sym = instr.a()->def_sym();
        if(m_local_ssa_index_mp[sym->get_tag()])
        {
            instr.rebind_a(m_local_ssa_index_mp[sym->get_tag()]);
        }
    }
    if(instr.b()!=nullptr&&instr.b()->ssa_index()>=0)
    {
        IRSymbol* sym = instr.b()->def_sym();
        if(m_local_ssa_index_mp[sym->get_tag()])
        {
            instr.rebind_b(m_local_ssa_index_mp[sym->get_tag()]);
        }
    }
    //rebind rvalue
    if (instr.type() == IRType::ArrayStore) {
        if (instr.r() != nullptr && instr.r()->ssa_index() >= 0) {
            IRSymbol* sym = instr.r()->def_sym();
            if (m_local_ssa_index_mp[sym->get_tag()]) {
                instr.rebind_r(m_local_ssa_index_mp[sym->get_tag()]);
            }
        }
    }else { //rebind lvalue
        if (instr.r() != nullptr && instr.r()->ssa_index() >= 0) {
            IRSymbol* sym = instr.r()->def_sym();
            instr.rebind_r(m_ir_sym_table->create_ssa(sym, m_ssa_sym_index_mp[sym->get_tag()]++));
            m_local_ssa_index_mp[sym->get_tag()] = instr.r();
        }
    }
}

void LoopUnrolling::rebind_phi(IRInstr& instr,IRBlock* header)
{
    IRSymbol* sym;
    if(instr.r()->basic_type()==BasicType::Int)
        sym = m_ir_sym_table->create_phi_func(BasicType::Int);
    else sym = m_ir_sym_table->create_phi_func(BasicType::Float);
    for(auto& param:instr.a()->phi_params())
    {
        if(param.from!=header)
            sym->add_phi_param(param.sym,param.from);
    }
    instr.rebind_a(sym);
}


void LoopUnrolling::compute_local_ssa_max_index(IRInstr& instr)
{
    if(instr.r()!=nullptr&&instr.r()->ssa_index()>=0)
    {
        IRSymbol* sym = instr.r()->def_sym();
        if(!m_local_ssa_index_mp[sym->get_tag()])
        {
            m_local_ssa_index_mp[sym->get_tag()] = instr.r();
        }
        else if(instr.r()->ssa_index()> m_local_ssa_index_mp[sym->get_tag()]->ssa_index())
        {
           m_local_ssa_index_mp[sym->get_tag()] = instr.r();
        }
    }
    if(instr.a()!=nullptr&&instr.a()->ssa_index()>=0)
    {
        IRSymbol* sym = instr.a()->def_sym();
        if(!m_local_ssa_index_mp[sym->get_tag()])
        {
            m_local_ssa_index_mp[sym->get_tag()] = instr.a();
        }
        else if(instr.a()->ssa_index()> m_local_ssa_index_mp[sym->get_tag()]->ssa_index())
        {
           m_local_ssa_index_mp[sym->get_tag()] = instr.a();
        }
    }
    if(instr.b()!=nullptr&&instr.b()->ssa_index()>=0)
    {
        IRSymbol* sym = instr.b()->def_sym();
        if(!m_local_ssa_index_mp[sym->get_tag()])
        {
            m_local_ssa_index_mp[sym->get_tag()] = instr.b();
        }
        else if(instr.b()->ssa_index()> m_local_ssa_index_mp[sym->get_tag()]->ssa_index())
        {
           m_local_ssa_index_mp[sym->get_tag()] = instr.b();
        }
    }
}



bool LoopUnrolling::is_computed_var(IRSymbol* sym)
{
    if(sym->basic_type()!=BasicType::Int)
      return false;
    if(sym->kind()==IRSymbolKind::Value)
      return true;
    if(!is_ssa_var(sym))
      return false;

    if(m_has_sym_value_table[sym->def_sym()->get_tag()])
     return true;
    return false;
}

bool LoopUnrolling::is_computed_expression(IRInstr& instr)
{
    if(instr.type()==IRType::BinaryCalc)
    {
        if(is_computed_var(instr.a())&&is_computed_var(instr.b()))
          return true;
    }
    else if(instr.type()==IRType::UnaryCalc)
    {
        if(is_computed_var(instr.a()))
          return true;
    }
    else if(instr.type()==IRType::Assign)
    {
        if(is_computed_var(instr.a()))
          return true;
    }
    return false;
}

bool LoopUnrolling::can_leave_loop(int start,int target,IROper op,bool leave_condition)
{
    bool can_leave = false;
    switch (op)
    {
        case IROper::EqualI:
            can_leave = (start == target) == leave_condition;
            break;
        case IROper::NotEqualI:
            can_leave = (start != target) == leave_condition;
            break;
        case IROper::GreaterEqualI:
            can_leave = (start >= target) == leave_condition;
            break;
        case IROper::LessI:
            can_leave = (start < target) == leave_condition;
            break;
        case IROper::GreaterI:
            can_leave = (start > target) == leave_condition;
            break;
        case IROper::LessEqualI:
            can_leave = (start <= target) == leave_condition;
            break;
        default:
            break;
    }
    return can_leave;
}



int LoopUnrolling::get_binary_value_result(int num1 ,int num2 ,IROper op)
{
   int res = 0;
   switch (op)
   {
      case IROper::AddI:
          res = num1 + num2;
          break;
      case IROper::SubI:
          res = num1 - num2;
          break;
      case IROper::MulI:
          res = num1 * num2;
          break;
      case IROper::DivI:
          res = num1 / num2;
          break;
      case IROper::ModI:
          res = num1 % num2;
          break;
      case IROper::EqualI:
          res = num1 == num2;
          break;
      case IROper::NotEqualI:
          res = num1 != num2;
          break;
      case IROper::GreaterEqualI:
          res = num1 >= num2;
          break;
      case IROper::LessI:
          res = num1 < num2;
          break;
      case IROper::GreaterI:
          res = num1 > num2;
          break;
      case IROper::LessEqualI:
          res = num1 <= num2;
          break;
      default:
          break;
   }
   return res;
}

int LoopUnrolling::get_unary_value_result(int num1,IROper op)
{
    int res = 0;
    switch (op)
    {
        case IROper::NegI:
            res = -num1;
            break;
        case IROper::NotI:
            res = num1?0:1;
            break;
        default:
            break;
    }
    return res;
}


int LoopUnrolling::get_computed_var(IRSymbol* sym)
{
   int num = 0;
   if(sym->kind()==IRSymbolKind::Value)
      num = sym->value().int_value;
   else num = m_sym_value_table[sym->def_sym()->get_tag()];
   return num;
}



void LoopUnrolling::compute_instr_value(IRInstr& instr)
{
   int res = 0 ;
   if(instr.type()==IRType::BinaryCalc)
   {
       int num1 = get_computed_var(instr.a());
       int num2 = get_computed_var(instr.b());
       res = get_binary_value_result(num1,num2,instr.op());
       m_sym_value_table[instr.r()->def_sym()->get_tag()] = res; 
       m_has_sym_value_table[instr.r()->def_sym()->get_tag()] = true;
       ///*DEBUG*/std::cout<<instr.r()->get_string()<<" "<<res<<std::endl;
   }
   else if(instr.type()==IRType::UnaryCalc)
   {
       int num1 = get_computed_var(instr.a());
       res = get_unary_value_result(num1,instr.op());
       m_sym_value_table[instr.r()->def_sym()->get_tag()] = res; 
       m_has_sym_value_table[instr.r()->def_sym()->get_tag()] = true;
        ///*DEBUG*/std::cout<<instr.r()->get_string()<<" "<<res<<std::endl;
   }
   else if(instr.type()==IRType::Assign)
   {
      res = get_computed_var(instr.a());
      m_sym_value_table[instr.r()->def_sym()->get_tag()] = res; 
      m_has_sym_value_table[instr.r()->def_sym()->get_tag()] = true;
       ///*DEBUG*/std::cout<<instr.r()->get_string()<<" "<<res<<std::endl;
   }

}


int LoopUnrolling::compute_initial_value(IRSymbol* sym,IRBlock* header)
{
    IRInstr* instr = m_use_def_chain[sym->get_tag()];
    switch (instr->type())
    {
        case IRType::Assign:
        {
            //std::cout<<"compute assign"<<std::endl;
            if(instr->a()->kind()==IRSymbolKind::Value)
            {
               // if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
                  // std::cout<<"out of size"<<std::endl;
                m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
                return m_sym_value_table[instr->r()->def_sym()->get_tag()] = instr->a()->value().int_value;
            }
            else if(is_ssa_var(instr->a()))
            {
                /*
                 if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
                   std::cout<<"out of size"<<std::endl;
                std::cout<<m_has_sym_value_table.size()<<std::endl;*/
                m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
                return m_sym_value_table[instr->r()->def_sym()->get_tag()] = compute_initial_value(instr->a(),header);

            }
            break;
        }
        case IRType::UnaryCalc:
        {
            //std::cout<<"compute unarycalc"<<std::endl;
            if(instr->a()->kind()==IRSymbolKind::Value)
            {
                 //if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
                   //std::cout<<"out of size"<<std::endl;
                m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
                return m_sym_value_table[instr->r()->def_sym()->get_tag()] = get_unary_value_result(instr->a()->value().int_value,instr->op());
            }
            else if(is_ssa_var(instr->a()))
            {
                m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
                return m_sym_value_table[instr->r()->def_sym()->get_tag()] = get_unary_value_result(compute_initial_value(instr->a(),header),instr->op());
            }
               
           break;
        }
        case IRType::BinaryCalc:
        {
            //std::cout<<"compute binarycalc"<<std::endl;
            if(instr->a()->kind()==IRSymbolKind::Value)
            {
                // if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
                   //std::cout<<"out of size"<<std::endl;
                m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
                return m_sym_value_table[instr->r()->def_sym()->get_tag()] = get_binary_value_result(instr->a()->value().int_value,compute_initial_value(instr->b(),header),instr->op());
            }
           else if(instr->b()->kind()==IRSymbolKind::Value)
           {
                // if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
                //   std::cout<<"out of size"<<std::endl;
               m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
               return m_sym_value_table[instr->r()->def_sym()->get_tag()] = get_binary_value_result(compute_initial_value(instr->a(),header),instr->b()->value().int_value,instr->op());
           }
           else if(is_ssa_var(instr->a())&&is_ssa_var(instr->b()))
           {
                //if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
               //    std::cout<<"out of size"<<std::endl;
               m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
               return m_sym_value_table[instr->r()->def_sym()->get_tag()] = get_binary_value_result(compute_initial_value(instr->a(),header),compute_initial_value(instr->b(),header),instr->op());
           }
            break;
        }
        case IRType::PhiFunc:
        {
             //std::cout<<"compute phifunc"<<std::endl;
             int res = 0;
             for(auto& param:instr->a()->phi_params())
             {
                 if(param.from==header)
                  continue;
                 res = compute_initial_value(param.sym,header);
                 break;
             }
             /*
             if(instr->r()->def_sym()->get_tag()>m_has_sym_value_table.size())
                   std::cout<<"out of size"<<std::endl;*/
             m_has_sym_value_table[instr->r()->def_sym()->get_tag()] = true;
             return m_sym_value_table[instr->r()->def_sym()->get_tag()] = res;
             break;
        }
    }
    return -1;
}


bool LoopUnrolling::has_initial_value(IRSymbol* sym,IRBlock* header)
{
    if(sym->get_tag()<0)
       return false;
    IRInstr* instr = m_use_def_chain[sym->get_tag()];
    if(m_visited_instr.find(sym->get_tag())!=m_visited_instr.end())
        return false;
    m_visited_instr.insert(sym->get_tag());
    switch (instr->type())
    {
       case IRType::UnaryCalc:
       case IRType::Assign:
       {
           //std::cout<<"assign or unarycalc"<<std::endl;
           if(instr->a()->kind()==IRSymbolKind::Value)
               return true;
           else if(is_ssa_var(instr->a()))
           {
               return has_initial_value(instr->a(),header);
           }
            break;
       }
       case IRType::BinaryCalc:
       {
           //std::cout<<"binarycalc"<<std::endl;
           if(instr->a()->kind()==IRSymbolKind::Value)
           {
                return has_initial_value(instr->b(),header);
           }
           else if(instr->b()->kind()==IRSymbolKind::Value)
           {
                return has_initial_value(instr->a(),header);
           }
           else if(is_ssa_var(instr->a())&&is_ssa_var(instr->b()))
           {
                 return has_initial_value(instr->a(),header)&&has_initial_value(instr->b(),header);
           }
           break;
       }
       case IRType::PhiFunc:
       {
           //std::cout<<"phifunc"<<std::endl;
           std::vector<int> all_initial_res;
           int initial_res;
           //std::cout<<instr->a()->phi_params().size()<<std::endl;
           for(auto& param:instr->a()->phi_params())
           {
               if(param.from==header)
                  continue;
                //std::cout<<param.sym->get_string()<<std::endl;
               if(!has_initial_value(param.sym,header))
                  return false;
               all_initial_res.push_back(compute_initial_value(param.sym,header));
           }
           if(all_initial_res.size()==0)
           {
               //std::cout<<"size 0"<<std::endl;
               return false;
           }
           initial_res = all_initial_res[0];
           for(int i = 1;i<all_initial_res.size();++i)
           {
               if(initial_res!=all_initial_res[i])
               {
                   //std::cout<<"res not the same "<<std::endl;
                   return false;
               }
           }
           return true;
           break;
       }

       default:
           break;
    }
    return false;
}





void LoopUnrolling::update_ssa_index(IRBlock* block,IRBlock* pre_block)
{
    for(auto& instr:block->get_instr_list())
    {
        if(instr.type()!=IRType::PhiFunc)
        {
             if(instr.a()!=nullptr&&instr.a()->ssa_index()>=0)
             {
                 if(!m_is_stop_spread[instr.a()->def_sym()->get_tag()]&&m_local_ssa_index_mp[instr.a()->def_sym()->get_tag()])
                 {
                     instr.rebind_a(m_local_ssa_index_mp[instr.a()->def_sym()->get_tag()]);
                 }
             }
             if(instr.b()!=nullptr&&instr.b()->ssa_index()>=0)
             {
                 if(!m_is_stop_spread[instr.b()->def_sym()->get_tag()]&&m_local_ssa_index_mp[instr.b()->def_sym()->get_tag()])
                 {
                     instr.rebind_b(m_local_ssa_index_mp[instr.b()->def_sym()->get_tag()]);
                 }
             }
             if(instr.r()!=nullptr&&instr.r()->ssa_index()>=0)
             {
                   m_is_stop_spread[instr.r()->def_sym()->get_tag()]=true;
                  if(m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()])
                  {
                     //m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()]=nullptr;
                     m_stop_spread_sym.push_back(instr.r());
                  }
             }
        }
        else{
            IRSymbol* sym;
            if(instr.r()->basic_type()==BasicType::Int)
              sym = m_ir_sym_table->create_phi_func(BasicType::Int);
            else sym = m_ir_sym_table->create_phi_func(BasicType::Float); 
            for(auto& param:instr.a()->phi_params())
            {
                if(param.from==pre_block)
                {
                     if(!m_is_stop_spread[param.sym->def_sym()->get_tag()]&&m_local_ssa_index_mp[param.sym->def_sym()->get_tag()])
                       sym->add_phi_param(m_local_ssa_index_mp[param.sym->def_sym()->get_tag()],param.from);
                     else sym->add_phi_param(param.sym,param.from);
                }
                else sym->add_phi_param(param.sym,param.from);
            }
            instr.rebind_a(sym);
            //m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()]=nullptr;
            m_stop_spread_sym.push_back(instr.r());
             m_is_stop_spread[instr.r()->def_sym()->get_tag()]=true;
            // for(auto& param:instr.a()->phi_params())
            // {
            //     std::cout<<"from block: "<<param.from->get_index()<<" sym:";
            //     if (param.sym->ssa_index() != -1)
            //       std::cout<< "l" << param.sym->index() << "_" << param.sym->ssa_index();
            //     else std::cout<< "l" << param.sym->index(); 
            //     std::cout<<" in block:"<<block->get_index()<<std::endl;
            // }
        }
        
    }
}

void LoopUnrolling::update_ssa_index_of_otherblock(IRBlock* header)
{
    m_stop_spread_sym.clear();
    m_local_ssa_index_mp.clear();
    m_stop_spread_sym.resize(m_sym_count);
    m_local_ssa_index_mp.resize(m_sym_count);
    for(auto& instr:header->get_instr_list())
        compute_local_ssa_max_index(instr);
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(header);
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        m_stop_spread_sym.clear();
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                    m_is_stop_spread.clear();
                    m_is_stop_spread.resize(m_sym_count,false);
                    q.push(now->get_succ(k));
                    visited.insert(now->get_succ(k));
                    update_ssa_index(now->get_succ(k),now);
            }
        }
        for(auto& sym:m_stop_spread_sym)
           m_local_ssa_index_mp[sym->def_sym()->get_tag()] = nullptr;
    }
}

void LoopUnrolling::change_header_block_edge(IRBlock* header)
{
      if(header->get_succ(0)==header)
      {
         IRBlock* next_block = header->get_succ(1);
         header->delete_edge(1);              
         header->delete_edge(0);
         header->set_edge(0,next_block);
      }
      else{
         IRBlock* next_block = header->get_succ(0);
         header->delete_edge(1);
         header->delete_edge(0);
         header->set_edge(0,next_block);
      }
 }

 void LoopUnrolling::change_new_block_edge(IRBlock* header,IRBlock* unroll_block,IRBlock* cond_block)
 {
    //  for(auto& block:header->get_pred())
    //    std::cout<<"block B"<<block->get_index()<<std::endl;
    auto pred_set = header->get_pred();
     for(auto& block:pred_set)
     {
         if(block==header)
           continue;
         if(block->get_succ(0)==header)
         {
             block->delete_edge(0);
             block->set_edge(0,cond_block);
         }
         else{
             block->delete_edge(1);
             block->set_edge(1,cond_block);
         }
     }
     unroll_block->set_edge(1,unroll_block);
     unroll_block->set_edge(0,header);
     cond_block->set_edge(1,unroll_block);
     cond_block->set_edge(0,header);
 }

void LoopUnrolling::work_loop(IRBlock* header,int size,int execute_time)
{
      m_store_instrlist.clear();
      //m_store_instrlist2.clear();
      m_local_ssa_index_mp.clear();
      m_local_temp_index_mp.clear();
      m_local_ssa_index_mp.resize(m_sym_count);
      m_local_temp_index_mp.resize(m_sym_count);
      for(auto& instr:header->get_instr_list())
         compute_local_ssa_max_index(instr);
      for(int i=0;i<execute_time-1;++i)
      {
          int num = 0;
          for(auto& instr:header->get_instr_list())
          {
              if(num<size)
              {
                  if(instr.type()!=IRType::PhiFunc)
                    m_store_instrlist.push_back(instr);
              }
              num++;
          }
      }
      for(int i=0;i<2;++i)
      {
          //m_store_instrlist2.push_front(header->get_instr_list().back());
          header->get_instr_list().pop_back();
      }
      auto iter1 = header->get_instr_list().end();
      header->get_instr_list().splice(iter1,m_store_instrlist);
      auto iter2 = header->get_instr_list().begin();
      std::advance(iter2,size);
      for(;iter2!=header->get_instr_list().end();++iter2)
      {
          IRInstr instr = *iter2;
          rebind_temp_index(*iter2);
          rebind_ssa_index(*iter2);
      }
      for (auto& instr:header->get_instr_list())
      {
         if(instr.type() == IRType::PhiFunc)
            rebind_phi(instr, header);
      }
      header->get_instr_list().push_back(IRInstr::create_block_goto());
}


int LoopUnrolling::predict_time(IRBlock* header,int predict_length)
{
    int time = 1;
    int target = -1;
    bool leave_condition = false;
    IRSymbol* start_sym=nullptr;
    IROper judge_op;
    m_sym_value_table.clear();
    m_has_sym_value_table.clear();
    m_sym_value_table.resize(m_sym_count,0);
    m_has_sym_value_table.resize(m_sym_count,false);
    if(header->get_succ(0)==header)
        leave_condition = true;
    auto iter = ++header->get_instr_list().rbegin();
    target = iter->b()->value().int_value;
    judge_op = iter->op();
    start_sym = iter->a();
    ///*DEBUG*/std::cout<<"target:"<<target<<std::endl;
    ///*DEBUG*/std::cout<<"start_sym:"<<start_sym->get_string()<<std::endl;
    // /*DEBUG*/std::cout<<"1"<<std::endl;
    assert(start_sym != nullptr);
    if(has_initial_value(start_sym,header))
    {
        // /*DEBUG*/std::cout<<"2"<<std::endl;
        m_sym_value_table[start_sym->def_sym()->get_tag()] = compute_initial_value(start_sym,header);
        m_has_sym_value_table[start_sym->def_sym()->get_tag()] = true;
        //std::cout<<"start_value:"<<m_sym_value_table[start_sym->def_sym()->get_tag()]<<std::endl;
    }
    else return -1;
    // /*DEBUG*/std::cout<<"3"<<std::endl;
    while(!can_leave_loop(m_sym_value_table[start_sym->def_sym()->get_tag()],target,judge_op,leave_condition)&&time<=500)
    {
        int pos = 0;
        for(auto instr:header->get_instr_list())
        {
            if(is_computed_expression(instr))
            {
                compute_instr_value(instr);
            }
            pos++;
            if(pos == predict_length)
               break;
        }
        time++;
        //std::cout<<"start_value:"<<m_sym_value_table[start_sym->def_sym()->get_tag()]<<std::endl;
    }
    // /*DEBUG*/std::cout<<"4"<<std::endl;
    return time;

    
}



LoopEnd LoopUnrolling::check_loop(const std::set<IRBlock*>& node_set,IRBlock* header)
{
    //std::cout<<"11111111"<<std::endl;
   if(node_set.size()!=1)
        return LoopEnd::LoopNoneEnd;
    //std::cout<<"1111"<<std::endl;
//    for(auto& instr:header->get_instr_list())
//    {
//        if(instr.r()!=nullptr&&instr.r()->basic_type()!=BasicType::Int)
//            return LoopEnd::LoopNoneEnd;
//        if(instr.a()!=nullptr&&instr.a()->basic_type()!=BasicType::Int)
//            return LoopEnd::LoopNoneEnd;
//        if(instr.b()!=nullptr&&instr.b()->basic_type()!=BasicType::Int)
//            return LoopEnd::LoopNoneEnd;
//    }
   auto iter = ++header->get_instr_list().rbegin();
   if(iter->type()!=IRType::BinaryCalc)
        return LoopEnd::LoopNoneEnd;
   //LinearIRManager::print_ir_instr(*iter,std::cout,"    ");
   if(iter->op()!=IROper::EqualI&&iter->op()!=IROper::NotEqualI&&iter->op()!=IROper::GreaterEqualI&&iter->op()!=IROper::LessEqualI&&iter->op()!=IROper::LessI&&iter->op()!=IROper::GreaterI)
        return LoopEnd::LoopNoneEnd; 
   //std::cout<<"11"<<std::endl;
   if(iter->a()->kind()!=IRSymbolKind::Local&&iter->a()->kind()!=IRSymbolKind::Param)
        return LoopEnd::LoopNoneEnd;
   //std::cout<<"12"<<std::endl;
   if(iter->a()->basic_type()!=BasicType::Int)
        return LoopEnd::LoopNoneEnd;
    //std::cout<<"13"<<std::endl;
   if(iter->b()->basic_type()!=BasicType::Int)
        return LoopEnd::LoopNoneEnd;
    //std::cout<<"14"<<std::endl;
   if(iter->b()->kind()==IRSymbolKind::Value)
   {
      //std::cout<<"15"<<std::endl;
       return LoopEnd::LoopValueEnd;
   }
   else if(iter->b()->kind()==IRSymbolKind::Local||iter->b()->kind()==IRSymbolKind::Param)
   {
       //std::cout<<"115"<<std::endl;
       if(iter->op()==IROper::LessI)
       {
            //std::cout<<"1115"<<std::endl;
            return LoopEnd::LoopVarEnd;
       }
   }
   //std::cout<<"16"<<std::endl;
   return LoopEnd::LoopNoneEnd;
}

bool LoopUnrolling::check_inc_by_one(IRBlock* header)
{
   auto iter = ++header->get_instr_list().rbegin();
   IRSymbol* start_sym = iter->a();
   IRSymbol* sym = nullptr;
   int add_time = 0;
   for(auto& instr:header->get_instr_list())
   {
       if(instr.type()==IRType::PhiFunc)
       {
           if(instr.r()->def_sym()==start_sym->def_sym())
           {
               sym = instr.r();
            //    std::cout<<sym->index() << "_" <<sym->ssa_index()<<std::endl;
            //    for(auto& param:instr.a()->phi_params())
            //    {
            //        if(param.from!=header)
            //        {
            //            std::cout<<param.sym->index() << "_" <<param.sym->ssa_index()<<std::endl;
            //            break;
            //        }
            //    }
               break;
           }
       }
   }
   if(!sym)
      return false;
   while(start_sym!=nullptr&&start_sym!=sym)
   {
       if(add_time>1)
         return false;
       IRInstr* instr = m_use_def_chain[start_sym->get_tag()];
       if(instr->type()==IRType::Assign)
       {
           start_sym = instr->a();
       }
       else if(instr->type()==IRType::BinaryCalc)
       {
           if(instr->op()!=IROper::AddI)
              return false;
           if(!instr->b()->is_value_1())
              return false;
           start_sym = instr->a();
           add_time++;
       }
       else return false;

   }
   if(add_time>1)
         return false;
   if(has_initial_value(start_sym,header))
   {
       if(compute_initial_value(start_sym,header)<0)
          return false;
   }
   else return false;
   return true;
}

IRBlock* LoopUnrolling::create_unroll_block(IRBlock* header,IRBlock* cond_block,int size,int unroll_size)
{
    m_local_ssa_index_mp.clear();
    m_local_ssa_index_mp.resize(m_sym_count);
    // m_local_temp_index_mp.clear();
    // m_local_temp_index_mp.resize(m_sym_count);
    auto it = ++header->get_instr_list().rbegin();
    IRSymbol* start_sym = it->a();
    IRBlock* unroll_block = new IRBlock();
    int num = 0,pos=0;
    for(int i=0;i<unroll_size;i++)
    {
        num = 0;
        for(auto& instr:header->get_instr_list())
        {
            if(num<size)
            {
                if(instr.type()!=IRType::PhiFunc)
                 unroll_block->get_instr_list().push_back(instr);
            }
            num++;
        }
    }
    if(header->in_degree()>2)
    {
       for(auto& instr:cond_block->get_instr_list())
       {
           if(instr.type()==IRType::PhiFunc)
           {
              IRSymbol* sym;
              if(instr.r()->basic_type()==BasicType::Int)
                 sym = m_ir_sym_table->create_phi_func(BasicType::Int);
              else sym = m_ir_sym_table->create_phi_func(BasicType::Float);
              sym->add_phi_param(instr.r(),cond_block);
              IRInstr new_inst = IRInstr::create_phi_func(m_ir_sym_table->create_ssa(instr.r()->def_sym(),m_ssa_sym_index_mp[instr.r()->def_sym()->get_tag()]++),sym);
              unroll_block->get_instr_list().push_front(new_inst);
              m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()] = new_inst.r();
           }
       }
    }
    else{
       for(auto& instr:header->get_instr_list())
       {
           if(instr.type()==IRType::PhiFunc)
           {
              IRSymbol* sym;
              if(instr.r()->basic_type()==BasicType::Int)
                 sym = m_ir_sym_table->create_phi_func(BasicType::Int);
              else sym = m_ir_sym_table->create_phi_func(BasicType::Float);
              for(auto& param:instr.a()->phi_params())
              {
                   if(param.from!=header)
                   {
                       sym->add_phi_param(param.sym,cond_block);
                       break;
                   }
              }
              IRInstr new_inst = IRInstr::create_phi_func(m_ir_sym_table->create_ssa(instr.r()->def_sym(),m_ssa_sym_index_mp[instr.r()->def_sym()->get_tag()]++),sym);
              unroll_block->get_instr_list().push_front(new_inst);
              m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()] = new_inst.r();
           }
       } 
    }
    for(auto& instr:unroll_block->get_instr_list())
    {
        if(instr.type()!=IRType::PhiFunc)
        {
            rebind_temp_index(instr);
            rebind_ssa_index(instr);
        }
    }
    for(auto& instr:unroll_block->get_instr_list())
    {
        if(instr.type()==IRType::PhiFunc)
        {
            instr.a()->add_phi_param(m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()],unroll_block);
        }
    }
    for(auto& instr:header->get_instr_list())
    {
        if(instr.type()==IRType::PhiFunc)
        {
            IRSymbol* sym;
            if(instr.r()->basic_type()==BasicType::Int)
              sym = m_ir_sym_table->create_phi_func(BasicType::Int);
            else sym = m_ir_sym_table->create_phi_func(BasicType::Float);
            sym->add_phi_param(m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()],unroll_block);
            if(header->in_degree()>2)
            {
               for(auto& param:instr.a()->phi_params())
               {
                   if(param.from==header)
                   {
                      sym->add_phi_param(param.sym,param.from);
                      break;
                   }
               }
               for(auto& instruct:cond_block->get_instr_list())
               {
                   if(instruct.r()->def_sym()==instr.r()->def_sym())
                   {
                       sym->add_phi_param(instruct.r(),cond_block);
                       break;
                   }
               }
            }
            else{
                for(auto& param:instr.a()->phi_params())
                {
                    if(param.from!=header)
                       sym->add_phi_param(param.sym,cond_block);
                    else sym->add_phi_param(param.sym,header);
                }
            }
            instr.rebind_a(sym);
        }
    }
    auto iter=cond_block->get_instr_list().begin();
    std::advance(iter,3);
    IRInstr new_inst1 = IRInstr::create_binary_calc(IROper::LessI,m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++),m_local_ssa_index_mp[start_sym->def_sym()->get_tag()],iter->r());
    IRInstr new_inst2 = IRInstr::create_block_cond_goto(new_inst1.r());
    unroll_block->get_instr_list().push_back(new_inst1);
    unroll_block->get_instr_list().push_back(new_inst2);
    return unroll_block;
}


IRBlock* LoopUnrolling::create_cond_block(IRBlock* header,int unroll_size)
{
    // m_local_ssa_index_mp.clear();
    // m_local_ssa_index_mp.resize(m_sym_count);
    IRBlock* cond_block = new IRBlock();
    auto iter = ++header->get_instr_list().rbegin();
    IRSymbol* start_sym = iter->a();
    IRSymbol* end_sym = iter->b();
    IRSymbol* sym = nullptr;
    IRInstr new_inst1 = IRInstr::create_binary_calc(IROper::SubI,m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++),end_sym,start_sym);
    IRInstr new_inst2 = IRInstr::create_binary_calc(IROper::SubI,m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++),new_inst1.r(),m_ir_sym_table->create_value_1(BasicType::Int));
    IRInstr new_inst3 = IRInstr::create_binary_calc(IROper::ModI,m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++),new_inst2.r(),m_ir_sym_table->create_value(BasicType::Int,BasicValue::create_int(unroll_size)));
    IRInstr new_inst4 = IRInstr::create_binary_calc(IROper::SubI,m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++),new_inst2.r(),new_inst3.r());
    IRInstr new_inst5 = IRInstr::create_binary_calc(IROper::LessI,m_ir_sym_table->create_temp(BasicType::Int,m_max_temp_index++),m_ir_sym_table->create_value(BasicType::Int,BasicValue::create_int(unroll_size)),new_inst1.r());
    IRInstr new_inst6 = IRInstr::create_block_cond_goto(new_inst5.r());
    cond_block->get_instr_list().push_back(new_inst1);
    cond_block->get_instr_list().push_back(new_inst2);
    cond_block->get_instr_list().push_back(new_inst3);
    cond_block->get_instr_list().push_back(new_inst4);
    cond_block->get_instr_list().push_back(new_inst5);
    cond_block->get_instr_list().push_back(new_inst6);
    for(auto& instr:header->get_instr_list())
    {
        if(instr.type()==IRType::PhiFunc)
        {
            if(instr.r()->def_sym()==start_sym->def_sym())
            {
                for(auto& param:instr.a()->phi_params())
                {
                    if(param.from!=header)
                    {
                         cond_block->get_instr_list().front().rebind_b(param.sym);
                         break;
                    }
                }
                break;
            }
        }
    }
    for(auto& instr:header->get_instr_list())
    {
        if(instr.type()==IRType::PhiFunc&&header->in_degree()>2)
        {
            IRSymbol* sym;
            if(instr.r()->basic_type()==BasicType::Int)
                 sym = m_ir_sym_table->create_phi_func(BasicType::Int);
            else sym = m_ir_sym_table->create_phi_func(BasicType::Float);
            for(auto& param:instr.a()->phi_params())
            {
                 if(param.from!=header)
                    sym->add_phi_param(param.sym,param.from);
            }
            IRInstr new_inst = IRInstr::create_phi_func(m_ir_sym_table->create_ssa(instr.r()->def_sym(),m_ssa_sym_index_mp[instr.r()->def_sym()->get_tag()]++),sym);
            cond_block->get_instr_list().push_front(new_inst);
            //m_local_ssa_index_mp[instr.r()->def_sym()->get_tag()] = new_inst.r();
        }
    }
    return cond_block;
}

void LoopUnrolling::work_cfg(IRUnit* unit)
{
    auto nloops=CFGManager::find_natural_loop(unit);
    for(auto& loop:nloops)
    {
        auto header = loop.header;
        const auto& node_set = loop.node_set;
        // const auto& exit_set = loop.exit_set;
        //const auto& back_edge_list = loop.back_edge_list;
        int size = header->get_instr_list().size()-2;
        LoopEnd loopend = check_loop(node_set,header);
        //std::cout<<"header: "<<"B"<<header->get_index()<<std::endl;
        if(loopend==LoopEnd::LoopNoneEnd)
        {
            //std::cout<<"continue end none"<<std::endl;
            continue;
        }
        else if(loopend == LoopEnd::LoopValueEnd)
        {
            int execute_time = predict_time(header,size);
             ///*DEBUG*/std::cout<<"execute time:"<<execute_time<<std::endl;
            if(execute_time>500||execute_time == -1)   
                continue;
            //  /*DEBUG*/std::cout<<"2"<<std::endl;
            else if(execute_time>1)
            {
                work_loop(header,size,execute_time);
                change_header_block_edge(header);
                update_ssa_index_of_otherblock(header);
            }  
        }
        else{
            if(!check_inc_by_one(header))
            {
                //std::cout<<"continue end var"<<std::endl;
                continue;
            }
            std::cout<<"loop end var unroll"<<std::endl;
            int unroll_size = 16;
            IRBlock* cond_block = create_cond_block(header,unroll_size);
            IRBlock* unroll_block = create_unroll_block(header,cond_block,size,unroll_size);
            change_new_block_edge(header,unroll_block,cond_block);
            
        }
        //  /*DEBUG*/std::cout<<"header:"<<header->get_index()<<" header size:"<<header->get_instr_list().size()<<std::endl;
        //  /*DEBUG*/std::cout<<"node_set:";
        //  for(auto node:node_set)
        //     /*DEBUG*/std::cout<<node->get_index()<<",";
        //  /*DEBUG*/std::cout<<std::endl;
        //  /*DEBUG*/std::cout<<"exit_set:";
        //  for(auto node:exit_set)
        //     /*DEBUG*/std::cout<<node->get_index()<<",";
        //  /*DEBUG*/std::cout<<std::endl;
        //  /*DEBUG*/std::cout<<"back_edge:";
        //  for(auto node:back_edge_list)
        //     /*DEBUG*/std::cout<<node->get_index()<<",";
        //  /*DEBUG*/std::cout<<std::endl;
    }
    unit->set_dominator_tree_info_valid(false);
}



void LoopUnrolling::find_max_index(IRUnit* unit)
{
    ir_list_find_index(unit->get_definations());
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        ir_list_find_index(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}


void LoopUnrolling::initial_sym_tag(IRUnit* unit)
{
    initial_decl_sym_tag(unit->get_definations());
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        initial_instr_sym_tag(now->get_instr_list());
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}

void LoopUnrolling::vis_cfg(IRUnit* unit)
{
    std::set<IRBlock*>visited;
    std::queue<IRBlock*> q;
    q.push(unit->get_entry());
    while (!q.empty()) {
        IRBlock* now = q.front();
        q.pop();
        std::cout<<"in block:"<<"B"<<now->get_index();
        for (int k = 1; k >= 0; --k) {
            if (now->get_succ(k) != nullptr && visited.find(now->get_succ(k)) == visited.end()) {
                q.push(now->get_succ(k));
                visited.insert(now->get_succ(k));
            }
        }
    }
}

void LoopUnrolling::run()
{
    std::cout << "Running pass: Loop Unrolling " << std::endl;
    if (m_cfg == nullptr ||m_ir_sym_table==nullptr) {
        Optimizer::optimizer_error("No target specified");
    }
    clear_table();
    //m_store_instrlist2.clear();
    m_sym_count = 0;
    m_max_temp_index = 0;
    for(auto& unit:*m_cfg)
    {
        if (unit.get_type() == IRUnitType::FuncDef){
            unit.set_dominator_tree_info_valid(false);
            initial_sym_tag(&unit);
        }
    }
    resize_table();
    for(auto& unit:*m_cfg)
    {
        if (unit.get_type() == IRUnitType::FuncDef){
             find_max_index(&unit);
         }
    }
    // for(auto iter = m_use_def_chain.begin();iter!=m_use_def_chain.end();++iter)
    // {
    //     LinearIRManager::print_ir_instr(*(*iter),std::cout,"    ");
    // }
    for (auto& unit : *m_cfg){
         if (unit.get_type() == IRUnitType::FuncDef){
             work_cfg(&unit);
         }
    }
    // for (auto& unit : *m_cfg){
    //      if (unit.get_type() == IRUnitType::FuncDef){
    //          vis_cfg(&unit);
    //      }
    // }
}
