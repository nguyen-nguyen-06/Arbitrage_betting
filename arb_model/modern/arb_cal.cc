#include "arb_cal.h"

ArbResult ArbCal::calc(const Event& e, double budget) {
  ArbResult res;
  best_odds(e, res);
  int n = res.odds.size();
  if (n < 2 || budget <= 0.0) return res;

  double S = 0.0;
  for (int i = 0; i < n; i++) {
    if (res.odds[i] <= 0.0) return res; // outcome i uncovered, no hedge possible
    S += 1.0 / res.odds[i];
  }
  // z* = B (1/S - 1) <= 0 here, so the LP with z >= 0 is infeasible: no arb.
  if (S >= 1.0) return res;

  // At the optimum every outcome pays exactly B/S (Step 3 of the proof),
  // so stake_i is whatever buys that payout at odds d_i.
  double payout = budget / S;
  res.stakes.resize(n);
  res.payouts.assign(n, payout);
  for (int i = 0; i < n; i++) res.stakes[i] = payout / res.odds[i];

  res.profit = payout - budget; // = B (1/S - 1) > 0
  res.roi = res.profit / budget;
  res.is_arb = true;
  return res;
}
