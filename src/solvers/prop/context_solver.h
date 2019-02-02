/*******************************************************************\

Module: Context-based Incremental Solver Interface

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Context-based interface for incremental solvers

#ifndef CPROVER_SOLVERS_PROP_CONTEXT_SOLVER_H
#define CPROVER_SOLVERS_PROP_CONTEXT_SOLVER_H

#include "prop_conv.h"

/// A `prop_convt` that wraps another `prop_convt` and provides
/// a push/pop-context interface for incremental solving
class context_solvert : public prop_convt
{
public:
  /// Push a new context on the stack
  /// This context becomes a child context nested in the current context.
  virtual void push_context() = 0;

  /// Pop the current context
  virtual void pop_context() = 0;

  /// Add \p expr to the formula within the current context
  void set_to(const exprt &expr, bool value) override = 0;

  /// Solve the current formula
  decision_proceduret::resultt dec_solve() override;

  /// Convert a Boolean expression \p expr
  literalt convert(const exprt &expr) override;

  /// Replace the variables in \p expr by values from the model
  exprt get(const exprt &expr) const override;

  /// Return the value of the given literal in the model
  tvt l_get(literalt) const override;

  /// Return a textual description of the decision procedure
  std::string decision_procedure_text() const override;

  /// Print satisfying assignment
  void print_assignment(std::ostream &out) const override;

  /// Not supported
  void set_frozen(literalt) override;

  /// Not supported
  void set_frozen(const bvt &) override;

  /// Not supported
  void set_assumptions(const bvt &) override;

  /// Not supported
  bool has_set_assumptions() const override;

  /// Not supported
  void set_all_frozen() override;

  /// Not supported
  bool is_in_conflict(literalt) const override;

  /// Not supported
  bool has_is_in_conflict() const override;

  // Set resource limits
  void set_time_limit_seconds(uint32_t) override;

  /// Returns the number of incremental solver calls
  std::size_t get_number_of_solver_calls() const override;

  /// Returns the underlying solver
  prop_convt &get_solver() const;

  virtual ~context_solvert() = default;

protected:
  explicit context_solvert(prop_convt &prop_conv);

  prop_convt &prop_conv;
};

#endif // CPROVER_SOLVERS_PROP_CONTEXT_SOLVER_H
