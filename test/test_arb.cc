// Tests for crawler parsing, the closed-form arb calculator, and agreement
// between the closed form (modern/) and the simplex path (legacy/).
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

#include "../arb_model/arb_util.h"
#include "../arb_model/crawler.h"
#include "../arb_model/legacy/arb_solver.h"
#include "../arb_model/modern/arb_cal.h"

static int passed = 0;
static int failed = 0;

static void check(const char* name, bool cond) {
  std::printf("  %s  %s\n", cond ? "PASS" : "FAIL", name);
  (cond ? passed : failed)++;
}

static void check_val(const char* name, double got, double want, double tol = 1e-4) {
  bool ok = std::fabs(got - want) < tol;
  if (ok) std::printf("  PASS  %s = %.6f\n", name, got);
  else    std::printf("  FAIL  %s = %.6f (want %.6f)\n", name, got, want);
  (ok ? passed : failed)++;
}

// Assert modern and legacy produce the same answer for these lines.
static void check_agree(const char* name, const std::vector<BestLine>& lines, double B) {
  ArbResult m = calc_arb(lines, B);
  ArbResult l = ArbSolver(lines, B).solve();
  bool ok = m.is_arb == l.is_arb && m.stakes.size() == l.stakes.size() &&
            std::fabs(m.profit - l.profit) < 1e-6;
  for (size_t i = 0; ok && i < m.stakes.size(); i++)
    ok = std::fabs(m.stakes[i] - l.stakes[i]) < 1e-6;
  std::printf("  %s  %s (closed %.6f vs simplex %.6f)\n", ok ? "PASS" : "FAIL", name,
              m.profit, l.profit);
  (ok ? passed : failed)++;
}

// The Odds API sample from the docs (one book, NFL).
static const char* kFanduelSample = R"JSON([
{
  "id": "bda33adca828c09dc3cac3a856aef176",
  "sport_key": "americanfootball_nfl",
  "commence_time": "2021-09-10T00:20:00Z",
  "home_team": "Tampa Bay Buccaneers",
  "away_team": "Dallas Cowboys",
  "bookmakers": [
    {
      "key": "fanduel", "title": "FanDuel", "last_update": "2021-06-10T10:46:09Z",
      "markets": [
        { "key": "h2h", "outcomes": [
            { "name": "Dallas Cowboys", "price": 240 },
            { "name": "Tampa Bay Buccaneers", "price": -303 } ] },
        { "key": "spreads", "outcomes": [
            { "name": "Dallas Cowboys", "price": -110, "point": 7.5 },
            { "name": "Tampa Bay Buccaneers", "price": -110, "point": -7.5 } ] }
      ]
    }
  ]
}])JSON";

// Two books whose disagreement creates a real arb.
static const char* kArbSample = R"JSON([
{
  "id": "x1", "sport_key": "basketball_nba", "commence_time": "2026-10-01T00:00:00Z",
  "home_team": "Home", "away_team": "Away",
  "bookmakers": [
    { "key": "a", "title": "BookA", "last_update": "t",
      "markets": [{ "key": "h2h", "outcomes": [
        { "name": "Away", "price": 150 }, { "name": "Home", "price": -120 } ] }] },
    { "key": "b", "title": "BookB", "last_update": "t",
      "markets": [{ "key": "h2h", "outcomes": [
        { "name": "Home", "price": 105 }, { "name": "Away", "price": 120 } ] }] }
  ]
}])JSON";

int main() {
  std::printf("\nTest 1: american_to_decimal\n");
  {
    check_val("+240", american_to_decimal(240), 3.4);
    check_val("-303", american_to_decimal(-303), 1.0 + 100.0 / 303.0, 1e-9);
    check_val("+100", american_to_decimal(100), 2.0);
    check_val("-100", american_to_decimal(-100), 2.0);
    bool threw = false;
    try { american_to_decimal(50); } catch (const std::invalid_argument&) { threw = true; }
    check("|price| < 100 throws", threw);
  }

  std::printf("\nTest 2: parse FanDuel sample (no arb, S = 1.045979)\n");
  {
    auto events = parse_odds_json(kFanduelSample);
    check("one event", events.size() == 1);
    const Event& e = events[0];
    check("id",   e.id == "bda33adca828c09dc3cac3a856aef176");
    check("home", e.home_team == "Tampa Bay Buccaneers");
    check("away", e.away_team == "Dallas Cowboys");
    check("two h2h outcomes (spreads ignored)", e.h2h.size() == 2);
    check("one quote each", e.h2h.at("Dallas Cowboys").size() == 1);
    check_val("Dallas decimal", e.h2h.at("Dallas Cowboys")[0].odds, 3.4);
    check("book title kept", e.h2h.at("Dallas Cowboys")[0].bookmaker == "FanDuel");

    auto lines = best_lines(e);
    check("two best lines", lines.size() == 2);
    check_val("S", implied_sum(lines), 1.045979, 1e-6);

    ArbResult r = calc_arb(e, 100.0);
    check("not an arb", !r.is_arb);
    check_val("profit (loss)", r.profit, 100.0 * (1.0 / 1.045979 - 1.0), 1e-4);
    check_val("stakes sum to budget", r.stakes[0] + r.stakes[1], 100.0, 1e-9);
    check_val("payouts equal", r.payouts[0] - r.payouts[1], 0.0, 1e-9);
    check("home/away copied", r.home_team == "Tampa Bay Buccaneers");
    check_agree("closed form == simplex", lines, 100.0);
  }

  std::printf("\nTest 3: two-book arb sample\n");
  {
    auto events = parse_odds_json(kArbSample);
    const Event& e = events[0];
    check("two quotes per outcome", e.h2h.at("Home").size() == 2 && e.h2h.at("Away").size() == 2);

    auto lines = best_lines(e);  // sorted by name: Away, Home
    check("Away best is BookA @2.5", lines[0].outcome == "Away" && lines[0].bookmaker == "BookA");
    check_val("Away odds", lines[0].odds, 2.5);
    check("Home best is BookB @2.05", lines[1].outcome == "Home" && lines[1].bookmaker == "BookB");
    check_val("Home odds", lines[1].odds, 2.05);

    double S = 1.0 / 2.5 + 1.0 / 2.05;
    check("S < 1", S < 1.0);
    ArbResult r = calc_arb(lines, 1000.0);
    check("is arb", r.is_arb);
    check_val("profit", r.profit, 1000.0 * (1.0 / S - 1.0), 1e-6);
    check_val("stake Away", r.stakes[0], 1000.0 / (2.5 * S), 1e-6);
    check_val("stake Home", r.stakes[1], 1000.0 / (2.05 * S), 1e-6);
    check_val("roi", r.roi, 1.0 / S - 1.0, 1e-9);
    check_agree("closed form == simplex", lines, 1000.0);
  }

  std::printf("\nTest 4: three-way market (soccer with draw), decimal input\n");
  {
    std::vector<BestLine> lines = {{"Draw", "X", 3.5}, {"Team A", "Y", 3.0}, {"Team B", "Z", 3.2}};
    double S = 1 / 3.5 + 1 / 3.0 + 1 / 3.2;
    ArbResult r = calc_arb(lines, 300.0);
    check("is arb", r.is_arb && S < 1.0);
    check_val("all payouts equal B/S", r.payouts[2], 300.0 / S, 1e-9);
    check_agree("closed form == simplex", lines, 300.0);

    std::vector<BestLine> no_arb = {{"Draw", "X", 3.1}, {"Team A", "Y", 2.6}, {"Team B", "Z", 2.9}};
    check("no arb when S > 1", !calc_arb(no_arb, 300.0).is_arb);
    check_agree("closed form == simplex (no arb)", no_arb, 300.0);
  }

  std::printf("\nTest 5: degenerate inputs\n");
  {
    check("single outcome is never an arb", !calc_arb({{"Only", "X", 5.0}}, 100).is_arb);
    check("single outcome via simplex", !ArbSolver({{"Only", "X", 5.0}}, 100).solve().is_arb);
    check("zero budget", !calc_arb({{"A", "X", 2.5}, {"B", "Y", 2.5}}, 0).is_arb);
    bool threw = false;
    try { parse_odds_json("{not json"); } catch (const std::runtime_error&) { threw = true; }
    check("malformed JSON throws", threw);
    threw = false;
    try { parse_odds_json(R"({"message":"Invalid API key"})"); } catch (const std::runtime_error&) { threw = true; }
    check("API error object throws", threw);
    check("empty array", parse_odds_json("[]").empty());
  }

  std::printf("\n %d passed, %d failed\n", passed, failed);
  return failed > 0 ? 1 : 0;
}
