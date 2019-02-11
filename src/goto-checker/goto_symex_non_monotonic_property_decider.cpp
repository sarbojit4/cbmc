/*******************************************************************\

Module: Non-monotonic Property Decider for Goto-Symex

Author: Daniel Kroening, Peter Schrammel

\*******************************************************************/

/// \file
/// Non-monotonic Property Decider for Goto-Symex

#include "goto_symex_non_monotonic_property_decider.h"

#include <solvers/prop/context_prop_conv_solver.h>

goto_symex_non_monotonic_property_decidert::
  goto_symex_non_monotonic_property_decidert(
    const optionst &options,
    ui_message_handlert &ui_message_handler,
    symex_target_equationt &equation,
    const namespacet &ns)
  : goto_symex_property_decidert(options, ui_message_handler, equation, ns),
    context_solver(
      util_make_unique<context_prop_conv_solvert>(solver->prop_conv()))
{
}

context_solvert &
goto_symex_non_monotonic_property_decidert::get_context_solver() const
{
  return *context_solver;
}

prop_convt &goto_symex_non_monotonic_property_decidert::get_solver() const
{
  return *context_solver;
}
