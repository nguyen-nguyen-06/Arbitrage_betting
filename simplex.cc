/**
 * simplex.cc — Two-phase Simplex solver
 *
 * Layout (tableau form):
 *   Column 0:        RHS (constant i.e basic var value when non-baisc = 0)
 *   Columns 1..n:    one per variable (original, slack, artificial)
 *
 *   Row r represents:  T[r][1]*x_0 + T[r][2]*x_1 + ... = T[r][0] 
 *                      where T[r][.] is taken from a_i * x = b_i
 *   Basic var of row r is at basic_pos_[r].
 *   Initially, the slack will be basic if artificial does not appear in this row.
 *   Otherwise, the artificial will be basic.
 *   Coef of basic var of row r always:  T[r][basic_pos[r] + 1] = +1 or - 1, value = T[r][0]
 *   obj_[0]    = current objective value
 *   obj_[j+1]  = reduced cost of variable j
 * */
#include <cmath>
#include <limits>
#include "simplex.h"

static const double ZERO_ = 1e-9; // handling floating point arithmetic
static const int ITERATIONS = 100;
static const double IGNORING = -1e8; // ignoring coef, make sure artificial won't affect Phase II

SimplexSolver::SimplexSolver(const std::vector <double>& c,
                  const std::vector <std::vector <double>>& A_le,
                  const std::vector <double>& b_le,
                  const std::vector <std::vector <double>>& A_eq,
                  const std::vector <double>& b_eq) {
  std::vector <std::vector <double>> A;
  std::vector <double> b;
  std::vector <std::string> signs;

  for (int i = 0; i < (int) A_le.size(); i++) {
    A.push_back(A_le[i]);
    b.push_back(b_le[i]);
    signs.push_back("<=");
  }

  for (int i = 0; i < (int) A_eq.size(); i++) {
    A.push_back(A_eq[i]);
    b.push_back(b_eq[i]);
    signs.push_back("=");
  }

  build_tableau(c, A, b, signs);
}

SimplexSolver::SimplexSolver(
    const std::vector<double>& c,
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b,
    const std::vector<std::string>& signs) {
  build_tableau(c, A, b, signs);
}

//                BUILD TABLEAU
// Note if num of artificial var > 0 => need phase 1
//      if artificial and slack both in a row, artificial will be in basis (to create I matrix)
// 6 cases: for >=, <=, = & b >, < 0
// a_i * x <= b, b > 0. => slack +1, slack is in basis
// a_i * x <= b, b < 0  => flip, slack -1, artificial +1 in basis
// a_i * x >= b, b > 0  => slack -1, artificial + 1 in basis
// a_i * x >= b, b < 0  => flip, slack +1 , slack in bais
// a_i * x = b, b >= 0 =>  artificial + 1 in basis
// a_i * x = b , b < 0 => flip, artificial + 1 in basis
void SimplexSolver::build_tableau(const std::vector<double>& c,
                      const std::vector<std::vector<double>>& A,
                      const std::vector<double>& b,
                      const std::vector<std::string>& signs){
  // Save objective for Phase II and extracting ans
  c_ = c;
  n_org_ = c.size();
  m_ = A.size();

  // Build tableau for Phase I (Aux LP)
  // First pass: count n_ = num of var = org + slack + artificial. Decide the size for T_
  int n_slack = 0;
  int n_art = 0;

  // Notes 
  // Why shouldn't negate a_i * x >= b > 0. (we can use that but not the convention)
  // If we negate -a_i * x <= -b, then add slack s > 0
  //  -a_i * x + s = - b
  // In init bfs x = 0, s = -b < 0 violates s > 0.
  // Need artificial a also > 0. It must have the form -a_i * x + s - a = -b,
  // but artificial usually use coef + 1 to form I matrix for calculating reduced cost in solve()
  for (int r = 0; r < m_; r ++) {
    if (signs[r] == "=") {
      n_art ++; // no need slack
    } else {
      bool need_art = (signs[r] == "<=" && b[r] < 0) || (signs[r] == ">=" && b[r] >= 0);
      if (need_art) {
        n_art ++;
      }
      n_slack ++;
    }
  }

  // Second pass: fill the coef for each rows
  // size T_ = m_ * (n_org + n_slack + n_art + 1(for b_i))
  n_ = n_org_ + n_slack + n_art;
  // Everything shift by 1 because T[r][0] contains b_r
  // x_i at col i + 1
  int ncols = n_ + 1;
  int slack_next = n_org_ + 1; // pos of next slack var
  int art_next = n_org_ + n_slack + 1; // pos next art var

  T_.assign(m_, std::vector<double>(ncols, 0.0));
  basic_pos_.resize(m_);
  artificial_idx_.clear();

  for (int r = 0; r < m_; r++) {
    if (signs[r] == "<=" && b[r] >= 0){
      // No need ZERO_ bc read from user.
      // Case 1: slack is basic
      T_[r][0] = b[r];
      for (int j = 1; j <= n_org_; j ++) T_[r][j] = A[r][j - 1];

      T_[r][slack_next] = 1.0;
      // shift 1 because basic_pos_ store index i of x_i not the column i
      // Mention aboved x_i at col i + 1
      basic_pos_[r] = slack_next - 1;
      slack_next++;
    } else if (signs[r] == "<=" && b[r] < 0) {
      // Case 2: flip row, slack gets -1, art + 1
      T_[r][0] = -b[r];
      for (int j = 1; j <= n_org_; j++) T_[r][j] = -A[r][j - 1];

      T_[r][slack_next] = -1.0;
      slack_next++;
      T_[r][art_next] = 1.0;
      basic_pos_[r] = art_next - 1;
      artificial_idx_.push_back(art_next - 1);
      art_next++;
    } else if (signs[r] == ">=" && b[r] >= 0) {
      // Case 3 slack -1, art +1
      T_[r][0] = b[r];
      for (int j = 1; j <= n_org_; j++) T_[r][j] = A[r][j - 1];

      T_[r][slack_next] = -1.0;
      slack_next++;
      T_[r][art_next] = 1.0;
      basic_pos_[r] = art_next - 1;
      artificial_idx_.push_back(art_next - 1);
      art_next++;
    } else if (signs[r] == ">=" && b[r] < 0) {
      // Case 4: flip <=, slack +1
      T_[r][0] = -b[r];
      for (int j = 1; j <= n_org_; j++) T_[r][j] = -A[r][j - 1];

      T_[r][slack_next] = 1.0;
      basic_pos_[r] = slack_next - 1;
      slack_next++;
    } else {
      // Case 5 + case 6: equality and b < 0 or b >= 0
      double flip = b[r] < 0 ? -1.0 : 1.0;
      T_[r][0] = std::abs(b[r]);
      for (int j = 1; j <= n_org_; j++) T_[r][j] = flip * A[r][j - 1];

      T_[r][art_next] = 1.0;
      basic_pos_[r] = art_next - 1;
      artificial_idx_.push_back(art_next - 1);
      art_next++;
    }
  }
  obj_.assign(ncols, 0.0);
}

// Find Entering (Bland's Rule)
int SimplexSolver::find_entering_col() {
  for (int j = 1; j <= n_; j++) {
    if (obj_[j] > ZERO_) {
      return j; // Need ZERO_ bc coef should = 0, no enter,
                // but floating pt arith make it approx 0
    }
  }
  return -1;
}

// Find Leaving (Bland's rule)
int SimplexSolver::find_leaving_row(int enter_col){
  // Ratio test: take index with min ratio, if tie, take the one with smallest index
  int leaving_row = -1;
  double min_ratio = std::numeric_limits<double>::infinity();
  for (int r = 0; r < m_; r++) {
    double coef = T_[r][enter_col];
    if (coef > ZERO_) {
      double ratio = T_[r][0] / coef;
      // if ratio < min_ratio
      if (ratio < min_ratio - ZERO_) {
        min_ratio = ratio;
        leaving_row = r;
      } else if (std::abs(ratio - min_ratio) <= ZERO_ && leaving_row != -1) {
        if (basic_pos_[r] < basic_pos_[leaving_row]) {
          leaving_row = r;
        }
      }
    }
  }
  return leaving_row;
}

// Pivoting
void SimplexSolver::pivot(int leaving_row, int entering_col) {
  int ncols = n_ + 1;
  double pivot = T_[leaving_row][entering_col];
  // modifying T_
  for (int j = 0; j < ncols; j++) T_[leaving_row][j] /= pivot;
  for (int r = 0; r < m_; r ++) {
    if (r == leaving_row) continue;
    double factor = T_[r][entering_col];
    if (std::abs(factor) < ZERO_) continue;
    for (int j = 0; j < ncols; j++) {
      T_[r][j] -= factor * T_[leaving_row][j];
    }
  }
  // Update objective row
  // Column 0 stores the objective val — pivot improvements add to it.
  // Columns 1..n store reduced costs — pivot updates subtract.
  double factor = obj_[entering_col];
  if (std::abs(factor) > ZERO_) {
    for (int j = 0; j < ncols; j++) {
      if (j == 0)
        obj_[j] += factor * T_[leaving_row][j];
      else
        obj_[j] -= factor * T_[leaving_row][j];
    }
  }

  basic_pos_[leaving_row] = entering_col - 1;
}

// Pivot loop
bool SimplexSolver::pivot_loop() {
  int max_iter = ITERATIONS * (m_ + n_);
  for (int iter = 0; iter < max_iter; iter++) {
    int enter_col = find_entering_col();
    if (enter_col == -1) return true;

    int leave_row = find_leaving_row(enter_col);
    if (leave_row == -1) return false;
    pivot(leave_row, enter_col);
  }
  return true;
}


// Solve: Phase I (Aux LP) + Phase 2 (Normal LP with init BFS extract from Phase I)
SimplexResult SimplexSolver::solve() {
  SimplexResult result;
  int ncols = n_ + 1;

  // Phase I
  if (!artificial_idx_.empty()) {
    std::vector<double> ph1_cost(n_, 0.0);
    for (int acol : artificial_idx_) ph1_cost[acol] = -1.0;

    // Calculating the reduced cost (i.e transforming obj_func)
    // obj_func = f(x_B) transform = g(x_NB) (B for basic, NB non basic)
    obj_.assign(ncols, 0.0);
    for (int j = 0; j < ncols; j++) {
        double cb_dot = 0.0;
        for (int r = 0; r < m_; r++) {
            cb_dot += ph1_cost[basic_pos_[r]] * T_[r][j];
        }
        // Column 0: objective value = c_B^T * b
        // Column j: reduced cost = c_j - c_B^T * A_j
        obj_[j] = (j == 0) ? cb_dot : ph1_cost[j - 1] - cb_dot;
    }

    pivot_loop();

    // If Phase I artificial var != 0 => The whole is infeasible
    for (int r = 0; r < m_; r++) {
      for (int a : artificial_idx_) {
        if (basic_pos_[r] == a && T_[r][0] > ZERO_) {
          result.feasible = false;
          return result;
        }
      }
    }
  }

  // Phase II
  std::vector<double> ph2_cost(n_, 0.0);
  for (int j = 0; j < n_org_; j++) ph2_cost[j] = c_[j];
  for (int a : artificial_idx_) ph2_cost[a] = IGNORING;

  // Calculating reduced cost
  obj_.assign(ncols, 0.0);
  for (int j = 0; j < ncols; j++) {
      double cb_dot = 0.0;
      for (int r = 0; r < m_; r++) {
          cb_dot += ph2_cost[basic_pos_[r]] * T_[r][j];
      }
      obj_[j] = (j == 0) ? cb_dot : ph2_cost[j - 1] - cb_dot;
  }

  bool optimal = pivot_loop();

  if (!optimal) {
    result.feasible = true;
    result.bounded = false;
    return result;
  }

  result.feasible = true;
  result.bounded = true;
  result.opt_val = obj_[0];
  result.opt_sol.assign(n_org_, 0.0);

  for (int r = 0; r < m_; r++) {
    if (basic_pos_[r] < n_org_) {
      result.opt_sol[basic_pos_[r]] = T_[r][0];
    }
  }

  return result;
}