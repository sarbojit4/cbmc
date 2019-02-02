/*******************************************************************\

Module: Context-based Incremental Solver Interface

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Context-based interface for incremental solvers

#include "context_solver.h"

#include <util/invariant.h>

context_solvert::context_solvert(prop_convt &prop_conv)
  : prop_convt(), prop_conv(prop_conv)
{
}

decision_proceduret::resultt context_solvert::dec_solve()
{
  return prop_conv.dec_solve();
}

literalt context_solvert::convert(const exprt &expr)
{
  return prop_conv.convert(expr);
}

exprt context_solvert::get(const exprt &expr) const
{
  return prop_conv.get(expr);
}

tvt context_solvert::l_get(literalt l) const
{
  return prop_conv.l_get(l);
}

std::string context_solvert::decision_procedure_text() const
{
  return prop_conv.decision_procedure_text();
}

void context_solvert::print_assignment(std::ostream &out) const
{
  prop_conv.print_assignment(out);
}

void context_solvert::set_frozen(literalt)
{
  // unsupported
  UNREACHABLE;
}

void context_solvert::set_frozen(const bvt &)
{
  // unsupported
  UNREACHABLE;
}

void context_solvert::set_assumptions(const bvt &_assumptions)
{
  // unsupported
  UNREACHABLE;
}

bool context_solvert::has_set_assumptions() const
{
  // unsupported
  return false;
}

void context_solvert::set_all_frozen()
{
  // unsupported
  UNREACHABLE;
}

bool context_solvert::is_in_conflict(literalt l) const
{
  // unsupported
  UNREACHABLE;
}

bool context_solvert::has_is_in_conflict() const
{
  // unsupported
  return false;
}

void context_solvert::set_time_limit_seconds(uint32_t limit)
{
  prop_conv.set_time_limit_seconds(limit);
}

std::size_t context_solvert::get_number_of_solver_calls() const
{
  return prop_conv.get_number_of_solver_calls();
}

prop_convt &context_solvert::get_solver() const
{
  return prop_conv;
}
