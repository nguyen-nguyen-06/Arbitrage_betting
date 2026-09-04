
#ifndef SIMPLEX_H_
#define SIMPLEX_H_

#include <vector>
#include <string>

typedef struct {
  bool bounded = true;
  bool feasible = true;
  double opt_val = 0.0;
  std::vector<double> opt_sol;
} SimplexResult;

class SimplexSolver {
  public:
    SimplexSolver() = delete; // No default constructor

    // Constructor 1: separated inequality/equality 
    // A_le * x <= b_le, A_eq * x = b_eq
    SimplexSolver(const std::vector <double>& c,
                  const std::vector <std::vector <double>>& A_le,
                  const std::vector <double>& b_le,
                  const std::vector <std::vector <double>>& A_eq,
                  const std::vector <double>& b_eq);
                  
    // Constructor 2: sign list
    // for ai rows in A, ai * x can be either <= , = or >= with bi
    SimplexSolver(const std::vector<double>& c,
                  const std::vector<std::vector<double>>& A,
                  const std::vector<double>& b,
                  const std::vector<std::string>& signs);
    // Solve: two-phase simplex
    //
    // Phase I (auxiliary LP):
    //   Objective: max -(sum of artificials) i.e. make all artificials to zero.
    //   Compute reduced costs by substituting basis variables out of the
    //   Phase I objective. rc_j = c_j - c_B^T * A_j.
    //   After pivoting, check if any artificial remains basic at nonzero value.
    //   If so, the original LP is infeasible — no point satisfies all constraints.
    //
    // Phase II (original LP):
    //   Recompute reduced costs from scratch using the real objective c.
    //   Artificials get cost -1e8 so the simplex never re-enters them.
    //   Pivot until optimal or unbounded.
    //   solution: for each basis variable that is an original variable
    //   (index < n_org_), read its value from T_[r][0]. Non-basic originals
    //   stay at 0. Slacks and artificials are ignored.
    SimplexResult solve();
  private:
    // Shared setup called by both constructors
    void build_tableau(const std::vector<double>& c,
                      const std::vector<std::vector<double>>& A,
                      const std::vector<double>& b,
                      const std::vector<std::string>& signs);
    // Pivot operations
    void pivot(int leave, int enter);
    bool pivot_loop();          // true = optimal, false = unbounded
    int find_entering_col();        // Bland's rule, returns col or -1
    int find_leaving_row(int enter);  // ratio test, returns row or -1

    // State
    std::vector<std::vector<double>> T_; // m_ rows × (n_+1) cols. Row r stores the
                                              // dictionary equation for basis_pos_[r].
                                              // Layout: [constant, x0, x1, ..., sN, ..., aN]

    std::vector<double> obj_;                 // Same layout as a Tableau_ row.
                                              // obj_[0] = current objective value,
                                              // obj_[j+1] = reduced cost of variable j.

    std::vector <double> c_;                  // original objective coef (need for phase II)

    std::vector<int> basic_pos_;              // basis_pos_[r] = index of the basic var that
                                              // owns row r. To access the coef of the basic var of row r
                                              // use T_[r][basic_pos_[r] + 1] (shift by 1 because T_[r][0] store a const)

    std::vector<int> artificial_idx_;         // idx of artificial variables.
                                              // This for Phase I objective and to
                                              // check feasibility after Phase I ends.

    int m_;                                   // Number of constraint rows in the tableau.

    int n_;                                   // Total number of variables (original + slack +
                                              // + artificial). Tableau_ has n_+1 cols.

    int n_org_;                               // Number of original decision variables.
                                              // Only indices 0..n_org_-1 appear in the
                                              // final solution; the rest are slack + artificial
};

#endif // SIMPLEX_H_