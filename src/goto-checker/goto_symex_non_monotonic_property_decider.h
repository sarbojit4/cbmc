/*******************************************************************\

Module: Non-Monotonic Property Decider for Goto-Symex

Author: Daniel Kroening, Peter Schrammel

\*******************************************************************/

/// \file
/// Non-Monotonic Property Decider for Goto-Symex

#ifndef CPROVER_GOTO_CHECKER_GOTO_SYMEX_NON_MONOTONIC_PROPERTY_DECIDER_H
#define CPROVER_GOTO_CHECKER_GOTO_SYMEX_NON_MONOTONIC_PROPERTY_DECIDER_H

#include "goto_symex_property_decider.h"

#include <solvers/prop/context_solver.h>

/// Provides management of goal variables that encode properties
class goto_symex_non_monotonic_property_decidert
  : public goto_symex_property_decidert
{
public:
  goto_symex_non_monotonic_property_decidert(
    const optionst &options,
    ui_message_handlert &ui_message_handler,
    symex_target_equationt &equation,
    const namespacet &ns);

  /// Returns the solver instance
  prop_convt &get_solver() const override;
  context_solvert &get_context_solver() const;

protected:
  std::unique_ptr<context_solvert> context_solver;
};

#endif // CPROVER_GOTO_CHECKER_GOTO_SYMEX_NON_MONOTONIC_PROPERTY_DECIDER_H
