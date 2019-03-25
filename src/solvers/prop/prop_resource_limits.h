/*******************************************************************\

Module: Context-based Incremental Solver Interface

Author: Peter Schrammel

\*******************************************************************/

/// \file
/// Context-based interface for incremental solvers

#ifndef CPROVER_SOLVERS_PROP_PROP_RESOURCE_LIMITS_H
#define CPROVER_SOLVERS_PROP_PROP_RESOURCE_LIMITS_H

class prop_resource_limitst
{
public:
  virtual void set_time_limit_seconds(uint32_t) = 0;

  virtual ~prop_resource_limitst() = default;
};

#endif // CPROVER_SOLVERS_PROP_PROP_RESOURCE_LIMITS_H
