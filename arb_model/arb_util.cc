#include "arb_util.h"

void best_odds(const Event& e, ArbResult& res) {
  int n = e.outcomes.size();
  int m = e.bookmakers.size();
  res.name = e.name;
  res.outcomes = e.outcomes;
  res.bookmakers.assign(n, "");
  res.odds.assign(n, 0.0);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      // odds 0.0 = not offered, never beats a real price
      if (e.odds[i][j] > res.odds[i]) {
        res.odds[i] = e.odds[i][j];
        res.bookmakers[i] = e.bookmakers[j];
      }
    }
  }
}
