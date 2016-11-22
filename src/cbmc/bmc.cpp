/*******************************************************************\

Module: Symbolic Execution of ANSI-C

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <fstream>
#include <iostream>
#include <memory>

#include <util/string2int.h>
#include <util/i2string.h>
#include <util/source_location.h>
#include <util/time_stopping.h>
#include <util/message.h>
#include <util/json.h>

#include <langapi/mode.h>
#include <langapi/languages.h>
#include <langapi/language_util.h>

#include <ansi-c/ansi_c_language.h>

#include <goto-programs/xml_goto_trace.h>
#include <goto-programs/json_goto_trace.h>
#include <goto-programs/graphml_goto_trace.h>

#include <goto-symex/build_goto_trace.h>
#include <goto-symex/slice.h>
#include <goto-symex/slice_by_trace.h>
#include <goto-symex/memory_model_sc.h>
#include <goto-symex/memory_model_tso.h>
#include <goto-symex/memory_model_pso.h>

#include "counterexample_beautification.h"
#include "fault_localization.h"
#include "bmc.h"

/*******************************************************************\

Function: bmct::do_unwind_module

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::do_unwind_module()
{
  // this is a hook for hw-cbmc
}

/*******************************************************************\

Function: bmct::error_trace

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::error_trace()
{
  if(options.get_bool_option("stop-when-unsat")) return; 

  status() << "Building error trace" << eom;

  goto_tracet &goto_trace=safety_checkert::error_trace;
  build_goto_trace(equation, prop_conv, ns, goto_trace);
  
  switch(ui)
  {
  case ui_message_handlert::PLAIN:
    std::cout << "\n" << "Counterexample:" << "\n";
    show_goto_trace(std::cout, ns, goto_trace);
    break;
  
  case ui_message_handlert::XML_UI:
    {
      xmlt xml;
      convert(ns, goto_trace, xml);
      std::cout << xml << "\n";
    }
    break;
  
  case ui_message_handlert::JSON_UI:
    {
      json_objectt json_result;
      json_arrayt &result_array=json_result["results"].make_array();
      json_objectt &result=result_array.push_back().make_object();
      const goto_trace_stept &step=goto_trace.steps.back();
      result["property"]=
        json_stringt(id2string(step.pc->source_location.get_property_id()));
      result["description"]=
        json_stringt(id2string(step.pc->source_location.get_comment()));
      result["status"]=json_stringt("failed");
      jsont &json_trace=result["trace"];
      convert(ns, goto_trace, json_trace);
      std::cout << ",\n" << json_result;
    }
    break;
  }

  const std::string graphml=options.get_option("graphml-cex");
  if(!graphml.empty())
  {
    graphmlt cex_graph;
    convert(ns, goto_trace, cex_graph);

    if(graphml=="-")
      write_graphml(cex_graph, std::cout);
    else
    {
      std::ofstream out(graphml.c_str());
      write_graphml(cex_graph, out);
    }
  }
}

/*******************************************************************\

Function: bmct::do_conversion

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::do_conversion()
{
  // convert HDL (hook for hw-cbmc)
  do_unwind_module();
  
  status() << "converting SSA" << eom;

  // convert SSA
  equation.convert(prop_conv);

  // the 'extra constraints'
  if(!bmc_constraints.empty())
  {
    status() << "converting constraints" << eom;
    
    forall_expr_list(it, bmc_constraints)
      prop_conv.set_to_true(*it);
  }
}

/*******************************************************************\

Function: bmct::run_decision_procedure

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

decision_proceduret::resultt
bmct::run_decision_procedure(prop_convt &prop_conv)
{
  status() << "Passing problem to " 
           << prop_conv.decision_procedure_text() << eom;

  prop_conv.set_message_handler(get_message_handler());

  // stop the time
  absolute_timet sat_start=current_time();
  
  do_conversion();

  status() << "Running " << prop_conv.decision_procedure_text() << eom;

  decision_proceduret::resultt dec_result=prop_conv.dec_solve();
  // output runtime

  {
    absolute_timet sat_stop=current_time();
    status() << "Runtime decision procedure: "
             << (sat_stop-sat_start) << "s" << eom;
  }

  return dec_result;
}

/*******************************************************************\

Function: bmct::report_success

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::report_success()
{
  result() << "VERIFICATION SUCCESSFUL" << eom;

  switch(ui)
  {
  case ui_message_handlert::PLAIN:
    break;
    
  case ui_message_handlert::XML_UI:
    {
      xmlt xml("cprover-status");
      xml.data="SUCCESS";
      std::cout << xml;
      std::cout << "\n";
    }
    break;

  case ui_message_handlert::JSON_UI:
    {
      json_objectt json_result;
      json_result["cProverStatus"]=json_stringt("success");
      std::cout << ",\n" << json_result;
    }
    break;
  }
}

/*******************************************************************\

Function: bmct::report_failure

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::report_failure()
{
  result() << "VERIFICATION FAILED" << eom;

  switch(ui)
  {
  case ui_message_handlert::PLAIN:
    break;
    
  case ui_message_handlert::XML_UI:
    {
      xmlt xml("cprover-status");
      xml.data="FAILURE";
      std::cout << xml;
      std::cout << "\n";
    }
    break;

  case ui_message_handlert::JSON_UI:
    {
      json_objectt json_result;
      json_result["cProverStatus"]=json_stringt("failure");
      std::cout << ",\n" << json_result;
    }
    break;
  }
}

/*******************************************************************\

Function: bmct::slice

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::slice()
{
  // any properties to check at all?
  if(symex.remaining_vccs==0)
  {
    // we should build a thread-aware SSA slicer
    statistics() << "no slicing due to threads" << eom;
  }
  else
  {
    if(options.get_bool_option("slice-formula"))
    {
      slice();
      statistics() << "slicing removed "
                   << equation.count_ignored_SSA_steps()
                   << " assignments" << eom;
    }
    else
    {
      if(options.get_list_option("cover").empty())
      {
        simple_slice(equation);
        statistics() << "simple slicing removed "
                     << equation.count_ignored_SSA_steps()
                     << " assignments" << eom;
      }
    }
  }
}

/*******************************************************************\

Function: bmct::show

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

safety_checkert::resultt bmct::show(const goto_functionst &goto_functions)
{
  if(options.get_bool_option("show-vcc"))
  {
    show_vcc();
    return safety_checkert::SAFE; // to indicate non-error
  }

  if(options.get_bool_option("program-only"))
  {
    show_program();
    return safety_checkert::SAFE;
  }

  return safety_checkert::UNKNOWN;
}

/*******************************************************************\

Function: bmct::show_program

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::show_program()
{
  unsigned count=1;

  languagest languages(ns, new_ansi_c_language());
  
  std::cout << "\n" << "Program constraints:" << "\n";

  for(symex_target_equationt::SSA_stepst::const_iterator
      it=equation.SSA_steps.begin();
      it!=equation.SSA_steps.end(); it++)
  {
    std::cout << "// " << it->source.pc->location_number << " ";
    std::cout << it->source.pc->source_location.as_string() << "\n";

    if(it->is_assignment())
    {
      std::string string_value;
      languages.from_expr(it->cond_expr, string_value);
      std::cout << "(" << count << ") " << string_value << "\n";

      if(!it->guard.is_true())
      {
        languages.from_expr(it->guard, string_value);
        std::cout << std::string(i2string(count).size()+3, ' ');
        std::cout << "guard: " << string_value << "\n";
      }
      
      count++;
    }
    else if(it->is_assert())
    {
      std::string string_value;
      languages.from_expr(it->cond_expr, string_value);
      std::cout << "(" << count << ") ASSERT("
                << string_value <<") " << "\n";

      if(!it->guard.is_true())
      {
        languages.from_expr(it->guard, string_value);
        std::cout << std::string(i2string(count).size()+3, ' ');
        std::cout << "guard: " << string_value << "\n";
      }

      count++;
    }  
    else if(it->is_assume())
    {
      std::string string_value;
      languages.from_expr(it->cond_expr, string_value);
      std::cout << "(" << count << ") ASSUME("
                << string_value <<") " << "\n";

      if(!it->guard.is_true())
      {
        languages.from_expr(it->guard, string_value);
        std::cout << std::string(i2string(count).size()+3, ' ');
        std::cout << "guard: " << string_value << "\n";
      }

      count++;
    }  
    else if(it->is_constraint())
    {
      std::string string_value;
      languages.from_expr(it->cond_expr, string_value);
      std::cout << "(" << count << ") CONSTRAINT("
                << string_value <<") " << "\n";

      count++;
    }  
    else if(it->is_shared_read() || it->is_shared_write())
    {
      std::string string_value;
      languages.from_expr(it->ssa_lhs, string_value);
      std::cout << "(" << count << ") SHARED_" << (it->is_shared_write()?"WRITE":"READ") << "("
                << string_value <<") " << "\n";

      if(!it->guard.is_true())
      {
        languages.from_expr(it->guard, string_value);
        std::cout << std::string(i2string(count).size()+3, ' ');
        std::cout << "guard: " << string_value << "\n";
      }

      count++;
    }  
  }
}

/*******************************************************************\

Function: bmct::initialize

  Inputs:

 Outputs:

 Purpose: initialize BMC

\*******************************************************************/

safety_checkert::resultt bmct::initialize()
{
  const std::string mm=options.get_option("mm");
  std::unique_ptr<memory_model_baset> memory_model;
  
  if(mm.empty() || mm=="sc")
    memory_model=std::unique_ptr<memory_model_baset>(new memory_model_sct(ns));
  else if(mm=="tso")
    memory_model=std::unique_ptr<memory_model_baset>(new memory_model_tsot(ns));
  else if(mm=="pso")
    memory_model=std::unique_ptr<memory_model_baset>(new memory_model_psot(ns));
  else
  {
    error() << "Invalid memory model " << mm
            << " -- use one of sc, tso, pso" << eom;
    return ERROR;
  }

  symex.set_message_handler(get_message_handler());
  symex.options=options;

  status() << "Starting Bounded Model Checking" << eom;

  symex.last_source_location.make_nil();
 
  // get unwinding info
  setup_unwind();
 
  return UNKNOWN;
 }
 
 /*******************************************************************\
 
Function: bmct::step
 
  Inputs: 
 
 Outputs: 
 
 Purpose: do BMC
 
 \*******************************************************************/
 
safety_checkert::resultt bmct::step(const goto_functionst &goto_functions)
{
  try
  {
    // perform symbolic execution
    symex(goto_functions);
 
    // add a partial ordering, if required    
    if(equation.has_threads())
    {
      memory_model->set_message_handler(get_message_handler());
      (*memory_model)(equation);
    }
 
    statistics() << "size of program expression: "
		 << equation.SSA_steps.size()
		 << " steps" << eom;

    // perform slicing
    slice(); 
 
    // do diverse other options
    {
      resultt result = show(goto_functions);
      if(result != UNKNOWN)
        return result;
    }

    if(!options.get_list_option("cover").empty())
    {
      const optionst::value_listt criteria=
        options.get_list_option("cover");
      return cover(goto_functions, criteria)?
        safety_checkert::ERROR:safety_checkert::SAFE;
    }

    if(options.get_option("localize-faults")!="")
    {
      fault_localizationt fault_localization(
        goto_functions, *this, options);
      return fault_localization();
    }
    
    //do all properties
    if(options.get_bool_option("all-properties"))
      return all_properties(goto_functions, prop_conv);

    //do "normal" BMC
    return stop_on_fail();
  }
 
  catch(std::string &error_str)
  {
    error() << error_str << eom;
    return ERROR;
  }

  catch(const char *error_str)
  {
    error() << error_str << eom;
    return ERROR;
  }

  catch(std::bad_alloc)
  {
    error() << "Out of memory" << eom;
    return ERROR;
  }

  assert(false);
}

/*******************************************************************\

Function: bmc_incrementalt::run
 
  Inputs: 

 Outputs: 

 Purpose: initialize and do BMC

\*******************************************************************/

safety_checkert::resultt bmct::run(
  const goto_functionst &goto_functions)
{
  safety_checkert::resultt result = initialize();
  if(result != UNKNOWN)
    return result;

  return step(goto_functions);
}

/*******************************************************************\

Function: bmct::stop_on_fail

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

safety_checkert::resultt bmct::stop_on_fail(bool show_report)
{
  prop_conv.set_message_handler(get_message_handler());
 
  switch(run_decision_procedure(prop_conv))
  {
  case decision_proceduret::D_UNSATISFIABLE:
    if(show_report)
      report_success();
 
    return SAFE;

  case decision_proceduret::D_SATISFIABLE:
    if(show_report)
    {
      if(options.get_bool_option("beautify"))
        counterexample_beautificationt()(
          dynamic_cast<bv_cbmct &>(prop_conv), equation, ns);
   
      error_trace();
      report_failure();
    }
    return UNSAFE;

  default:
    if(options.get_bool_option("dimacs") ||
       options.get_option("outfile")!="")
      return SAFE;
      
    error() << "decision procedure failed" << eom;

    return ERROR;
  }
}

/*******************************************************************\

Function: bmct::setup_unwind

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void bmct::setup_unwind()
{
  const std::string &set=options.get_option("unwindset");
  std::string::size_type length=set.length();

  for(std::string::size_type idx=0; idx<length; idx++)
  {
    std::string::size_type next=set.find(",", idx);
    std::string val=set.substr(idx, next-idx);

    unsigned thread_nr;
    bool thread_nr_set=false;

    if(!val.empty() &&
       isdigit(val[0]) &&
       val.find(":")!=std::string::npos)
    {
      std::string nr=val.substr(0, val.find(":"));
      thread_nr=unsafe_string2unsigned(nr);
      thread_nr_set=true;
      val.erase(0, nr.size()+1);
    }

    if(val.rfind(":")!=std::string::npos)
    {
      std::string id=val.substr(0, val.rfind(":"));
      long uw=unsafe_string2int(val.substr(val.rfind(":")+1));

      if(thread_nr_set)
        symex.set_unwind_thread_loop_limit(thread_nr, id, uw);
      else
        symex.set_unwind_loop_limit(id, uw);
    }
    
    if(next==std::string::npos) break;
    idx=next;
  }

  if(options.get_option("unwind")!="")
    symex.set_unwind_limit(options.get_unsigned_int_option("unwind"));
}
