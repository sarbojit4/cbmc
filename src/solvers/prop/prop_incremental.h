/*******************************************************************\

Module: Context-based Incremental Solver Interface

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Context-based interface for incremental solvers

#ifndef CPROVER_SOLVERS_PROP_PROP_INCREMENTAL_H
#define CPROVER_SOLVERS_PROP_PROP_INCREMENTAL_H

class prop_incrementalt
{
public:
  virtual void set_frozen(literalt) = 0;
  virtual void set_frozen(const bvt &) = 0;
  virtual void set_all_frozen() = 0;

  virtual ~prop_incrementalt() = default;
};

#endif // CPROVER_SOLVERS_PROP_PROP_INCREMENTAL_H
