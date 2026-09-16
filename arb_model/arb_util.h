#ifndef ARB_UTIL_H_
#define ARB_UTIL_H_

#include <string>
#include <vector>

#include "crawler.h"

typedef struct {
  bool is_arb = false;
  double profit = 0.0;
  double roi = 0.0;
  std::vector<double> stakes;
  std::vector<double> payouts;
  std::vector<std::string> outcomes;
  std::vector<std::string> bookmakers;  // best book per outcome
  std::vector<double> odds;             // best odds per outcome
  std::string name;                     // name of the event. For example: "A vs B"
} ArbResult;

// The reduction in legacy/arb_lp.md: for each outcome i take
// j* = argmax_j gain_ij and put the whole stake for i at book j*.
// Fills res.outcomes, res.bookmakers, res.odds, res.name.
// Everything else in res is left untouched.
// An outcome no book offers gets bookmakers[i] = "" and odds[i] = 0.0.
void best_odds(const Event& e, ArbResult& res);

#endif // ARB_UTIL_H_
