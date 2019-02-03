/*******************************************************************\

Module: Push/pop-context wrapper for propt-based refinement solvers

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Push/pop-context wrapper for `propt`-based refinement solvers

#ifndef CPROVER_SOLVERS_PROP_CONTEXT_PROP_CONV_SOLVER_H
#define CPROVER_SOLVERS_PROP_CONTEXT_PROP_CONV_SOLVER_H

#include "context_solver.h"

/// Provides a push/pop-context interface for incremental solving
/// to `propt`-based solvers such as `bv_refinementt`.
class context_prop_conv_solvert : public context_solvert
{
public:
  explicit context_prop_conv_solvert(prop_convt &prop_conv);

  void set_to(const exprt &expr, bool value) override;

  void push_context() override;

  void pop_context() override;

  static const char *context_prefix;

protected:
  // context literals used as assumptions
  bvt context_literals;

  // unique context literal names
  std::size_t context_literal_counter = 0;
};

#endif // CPROVER_SOLVERS_PROP_CONTEXT_PROP_CONV_SOLVER_H
