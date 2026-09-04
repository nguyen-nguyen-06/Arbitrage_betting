#ifndef ARB_UTIL_H_
#define ARB_UTIL_H_

#include <vector>
#include <string>

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
  std::string home_team;
  std::string away_team;
} ArbResult;

class bestOdd {
  public:
    bestOdd
    static void find(const Event& e);
    
  private:
    std::vector <std::string> bookmakers;
    std::vector <std::string> outcomes;
    std::vector <double> odds;
};
#endif // ARB_UTIL_H_