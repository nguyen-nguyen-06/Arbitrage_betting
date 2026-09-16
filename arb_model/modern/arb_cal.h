#ifndef ARB_CAL_H_
#define ARB_CAL_H_

#include "../arb_util.h"

// Closed-form solution of the arbitrage LP (see arb_lp_solved.md).
//
// Step 1: best_odds() picks j* = argmax_j gain_ij for each outcome i, so the
//         multi-book problem collapses to one decimal odds d_i per outcome.
// Step 2: with S = sum_i 1/d_i,
//           x_i* = B / (d_i S)       unique optimal stakes
//           z*   = B (1/S - 1)       guaranteed profit, same for every outcome
//           arb exists  <=>  S < 1
//
// The LP has z >= 0, so when S >= 1 it is infeasible: that is the "no arb"
// answer. calc() then returns is_arb = false with empty stakes/payouts,
// matching what ArbSolver::solve() reports for the same event. Same result
// if there are fewer than 2 outcomes or some outcome is offered by no book.
class ArbCal {
  public:
    static ArbResult calc(const Event& e, double budget);
};

#endif // ARB_CAL_H_
