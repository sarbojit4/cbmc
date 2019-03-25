/*******************************************************************\

Module: Context-based Incremental Solver Interface

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Context-based interface for incremental solvers

#ifndef CPROVER_SOLVERS_PROP_PROP_CONTEXT_H
#define CPROVER_SOLVERS_PROP_PROP_CONTEXT_H

class prop_contextt
{
public:
  /// Push a new context on the stack
  /// This context becomes a child context nested in the current context.
  virtual void push_context() = 0;

  /// Pop the current context
  virtual void pop_context() = 0;

  virtual ~prop_contextt() = default;
};

#endif // CPROVER_SOLVERS_PROP_PROP_CONTEXT_H
