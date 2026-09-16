// arb — find guaranteed-profit bets across bookmakers.
//
//   arb --file odds.json [options]          offline: parse a saved API response
//   arb --fetch [--sport KEY] [options]     live: hit The Odds API (needs ODDS_API_KEY)
//
// Options:
//   --budget B      total stake to split across outcomes        (default 100)
//   --lp            solve with the simplex (legacy/) and cross-check the closed form
//   --all           print every event, not just arbs
//   --decimal       input prices are already decimal odds (default: American)
//   --sport KEY     e.g. americanfootball_nfl, soccer_epl        (default upcoming)
//   --regions R     us,us2,uk,eu,au                              (default us)
//   --api-key KEY   overrides ODDS_API_KEY
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "arb_model/arb_util.h"
#include "arb_model/crawler.h"
#include "arb_model/legacy/arb_solver.h"
#include "arb_model/modern/arb_cal.h"

namespace {

struct Options {
  std::string file;
  bool fetch = false;
  double budget = 100.0;
  bool use_lp = false;
  bool show_all = false;
  OddsFormat fmt = OddsFormat::American;
  OddsRequest req;
};

void usage(const char* argv0) {
  std::fprintf(stderr,
      "usage: %s (--file odds.json | --fetch) [--budget B] [--lp] [--all]\n"
      "          [--decimal] [--sport KEY] [--regions R] [--api-key KEY]\n",
      argv0);
}

Options parse_args(int argc, char** argv) {
  Options o;
  auto need = [&](int& i) -> const char* {
    if (i + 1 >= argc) throw std::runtime_error(std::string(argv[i]) + " needs a value");
    return argv[++i];
  };
  for (int i = 1; i < argc; i++) {
    const char* a = argv[i];
    if (!std::strcmp(a, "--file"))          o.file = need(i);
    else if (!std::strcmp(a, "--fetch"))    o.fetch = true;
    else if (!std::strcmp(a, "--budget"))   o.budget = std::atof(need(i));
    else if (!std::strcmp(a, "--lp"))       o.use_lp = true;
    else if (!std::strcmp(a, "--all"))      o.show_all = true;
    else if (!std::strcmp(a, "--decimal"))  o.fmt = OddsFormat::Decimal;
    else if (!std::strcmp(a, "--sport"))    o.req.sport = need(i);
    else if (!std::strcmp(a, "--regions"))  o.req.regions = need(i);
    else if (!std::strcmp(a, "--api-key"))  o.req.api_key = need(i);
    else throw std::runtime_error(std::string("unknown option ") + a);
  }
  if (o.file.empty() == !o.fetch)
    throw std::runtime_error("give exactly one of --file or --fetch");
  if (o.budget <= 0) throw std::runtime_error("--budget must be > 0");
  o.req.odds_format = o.fmt;
  if (o.fetch && o.req.api_key.empty()) {
    if (const char* k = std::getenv("ODDS_API_KEY")) o.req.api_key = k;
  }
  return o;
}

// Largest relative/absolute disagreement between two solutions.
double max_diff(const ArbResult& a, const ArbResult& b) {
  double d = std::fabs(a.profit - b.profit);
  for (size_t i = 0; i < a.stakes.size() && i < b.stakes.size(); i++)
    d = std::max(d, std::fabs(a.stakes[i] - b.stakes[i]));
  return d;
}

void print_event(const Event& e, const ArbResult& r, double S) {
  std::printf("\n%s vs %s   [%s  %s]\n", e.home_team.c_str(), e.away_team.c_str(),
              e.sport_key.c_str(), e.commence_time.c_str());
  std::printf("  S = %.6f   overround %+.2f%%   %s\n", S, (S - 1.0) * 100.0,
              r.is_arb ? "*** ARBITRAGE ***" : "no arb");
  std::printf("  %-28s %-14s %8s %10s %10s\n", "Outcome", "Book", "Odds", "Stake", "Payout");
  for (size_t i = 0; i < r.outcomes.size(); i++) {
    std::printf("  %-28.28s %-14.14s %8.4f %10.2f %10.2f\n",
                r.outcomes[i].c_str(), r.bookmakers[i].c_str(), r.odds[i],
                i < r.stakes.size() ? r.stakes[i] : 0.0,
                i < r.payouts.size() ? r.payouts[i] : 0.0);
  }
  std::printf("  guaranteed profit %+.2f   roi %+.3f%%\n", r.profit, r.roi * 100.0);
}

}  // namespace

int main(int argc, char** argv) {
  Options opt;
  try {
    opt = parse_args(argc, argv);
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "error: %s\n", ex.what());
    usage(argv[0]);
    return 2;
  }

  std::vector<Event> events;
  try {
    events = opt.fetch ? fetch_events(opt.req)
                       : parse_odds_json(read_file(opt.file), opt.fmt);
  } catch (const std::exception& ex) {
    std::fprintf(stderr, "error: %s\n", ex.what());
    return 1;
  }

  int n_arb = 0, n_skipped = 0, n_mismatch = 0;
  for (const Event& e : events) {
    std::vector<BestLine> lines = best_lines(e);
    if (lines.size() < 2) { n_skipped++; continue; }

    ArbResult closed = calc_arb(lines, opt.budget);
    ArbResult shown = closed;
    if (opt.use_lp) {
      shown = ArbSolver(lines, opt.budget).solve();
      double d = max_diff(closed, shown);
      if (d > 1e-6) {
        n_mismatch++;
        std::fprintf(stderr, "warning: simplex and closed form disagree by %.3g on %s\n",
                     d, e.id.c_str());
      }
    }
    if (shown.is_arb) n_arb++;
    if (shown.is_arb || opt.show_all) print_event(e, shown, implied_sum(lines));
  }

  std::printf("\n%zu events, %d arbitrage%s, %d skipped (fewer than 2 outcomes)",
              events.size(), n_arb, n_arb == 1 ? "" : "s", n_skipped);
  if (opt.use_lp) std::printf(", %d LP/closed-form mismatch%s", n_mismatch, n_mismatch == 1 ? "" : "es");
  std::printf("\n");
  return 0;
}
