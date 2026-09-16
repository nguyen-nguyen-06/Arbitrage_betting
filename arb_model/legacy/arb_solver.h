#ifndef ARB_SOLVER_H_
#define ARB_SOLVER_H_

#include "../arb_util.h"

// Numerical solution of the arbitrage LP through SimplexSolver.
//
// Step 1: best_odds() picks j* = argmax_j gain_ij for each outcome i
//         (the reduction in arb_lp.md), leaving one odds d_i per outcome.
// Step 2: solve the epigraph-form LP from arb_lp.md,
//
//   max  z
//   s.t. z - d_i x_i + sum_k x_k <= 0     for each outcome i
//        sum_i x_i = B
//        x_i >= 0, z >= 0
//
// Row i says: if outcome i happens, payout minus total stake is at least z.
// With z >= 0 the LP is infeasible exactly when no arbitrage exists, and
// solve() then returns is_arb = false with empty stakes, the same as
// ArbCal::calc(). Same result if there are fewer than 2 outcomes or some
// outcome is offered by no book (row i would read z + B <= 0).
class ArbSolver {
  public:
    static ArbResult solve(const Event& e, double budget);
};

#endif // ARB_SOLVER_H_
