/*******************************************************************\

Module: Push/pop-context wrapper for propt-based refinement solvers

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Push/pop-context wrapper for `propt`-based refinement solvers

#include "context_prop_conv_solver.h"

context_prop_conv_solvert::context_prop_conv_solvert(prop_convt &prop_conv)
  : context_solvert(prop_conv)
{
}

const char *context_prop_conv_solvert::context_prefix = "prop_conv::context$";

void context_prop_conv_solvert::set_to(const exprt &expr, bool value)
{
  if(context_literals.empty())
  {
    // We are in the root context.
    prop_conv.set_to(expr, value);
  }
  else
  {
    // We have a child context. We add context_literal ==> expr to the formula.
    prop_conv.set_to(
      or_exprt(literal_exprt(!context_literals.back()), expr), value);
  }
}

void context_prop_conv_solvert::push_context()
{
  // We create a new context literal.
  literalt context_literal = prop_conv.convert(symbol_exprt(
    context_prefix + std::to_string(context_literal_counter++), bool_typet()));

  context_literals.push_back(context_literal);
  prop_conv.set_assumptions(context_literals);
}

void context_prop_conv_solvert::pop_context()
{
  PRECONDITION(!context_literals.empty());

  // We remove the last context literal from the stack.
  context_literals.pop_back();
  prop_conv.set_assumptions(context_literals);
}
