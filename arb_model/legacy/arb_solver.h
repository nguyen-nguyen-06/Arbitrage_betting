#ifndef ARB_SOLVER_H_
#define ARB_SOLVER_H_

#include <vector>
#include <string>

#include "../crawler.h"
#include "../arb_util.h"
#include "../../simplex.h"

class ArbSolver
{
public:
  static ArbResult solve(const Event &e, double budget = 100);
  static std::vector<ArbResult &> solve_all(const Event &e, double budget_per_event = 100);
};

#endif