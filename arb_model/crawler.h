#ifndef CRAWLER_H_
#define CRAWLER_H_

#include <string>
#include <vector>

// One event. outcomes are mutually exclusive and their union is everything
// that can happen, so exactly one of them pays out.
//
// odds[j][i] is the DECIMAL odds (total return per 1 unit staked) that
// bookmaker j pays on outcome i, i.e. gain_ij in legacy/arb_lp.md.
// odds[j][i] = 0.0 means book j does not offer outcome i.
// outcomes[i] and bookmakers[j] are labels only; the model works on i, j.
// odds.size() == bookmakers.size(),
//             odds[j].size() == outcomes.size() for every j.
//
// id, sport_key, commence_time come straight from the odds API and are
// metadata. commence_time is ISO-8601 UTC, once it has passed the market
// is closed and the event must be skipped.
typedef struct {
  std::string id;
  std::string sport_key;
  std::string commence_time;
  std::string name;  // display only, e.g. "Tampa Bay Buccaneers vs Dallas Cowboys"
  std::vector<std::string> outcomes;
  std::vector<std::string> bookmakers;
  std::vector<std::vector<double>> odds;
} Event;

#endif // CRAWLER_H_
