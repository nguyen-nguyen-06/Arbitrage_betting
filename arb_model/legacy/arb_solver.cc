#include "arb_solver.h"

#include "../../simplex.h"

static const double ZERO_ = 1e-9;

ArbResult ArbSolver::solve(const Event& e, double budget) {
  ArbResult res;
  best_odds(e, res);
  int n = res.odds.size();
  if (n < 2 || budget <= 0.0) return res;
  for (int i = 0; i < n; i++) {
    if (res.odds[i] <= 0.0) return res; // outcome i uncovered, LP would be infeasible
  }

  // Variables: x_0 .. x_{n-1}, then z at index n.
  int num_var = n + 1;
  std::vector<double> c(num_var, 0.0);
  c[n] = 1.0; // max z

  // n rows:  z - d_i x_i + sum_k x_k <= 0
  // so the coef on x_k is 1 - d_i when k == i, else 1; coef on z is 1.
  std::vector<std::vector<double>> A_le(n, std::vector<double>(num_var, 1.0));
  std::vector<double> b_le(n, 0.0);
  for (int i = 0; i < n; i++) A_le[i][i] -= res.odds[i];

  // 1 row:  sum_i x_i = B   (z not included)
  std::vector<std::vector<double>> A_eq(1, std::vector<double>(num_var, 1.0));
  A_eq[0][n] = 0.0;
  std::vector<double> b_eq = {budget};

  SimplexSolver lp(c, A_le, b_le, A_eq, b_eq);
  SimplexResult sol = lp.solve();

  // Infeasible: no z >= 0 covers every outcome, i.e. no arbitrage.
  // z* = 0 is also not an arb (break-even at best).
  if (!sol.feasible || !sol.bounded || sol.opt_val <= ZERO_) return res;

  res.stakes.assign(sol.opt_sol.begin(), sol.opt_sol.begin() + n);
  res.payouts.resize(n);
  for (int i = 0; i < n; i++) res.payouts[i] = res.stakes[i] * res.odds[i];

  res.profit = sol.opt_val; // z*
  res.roi = res.profit / budget;
  res.is_arb = true;
  return res;
}
