#include "../simplex.h"
#include <iostream>

static int passed = 0;
static int failed = 0;

void check(const char* name, bool cond) {
  if (cond) {
    printf("  PASS  %s\n", name);
    passed++;
  } else {
    printf("  FAIL  %s\n", name);
    failed++;
  }
}

void check_val(const char* name, double got, double want) {
  bool ok = std::abs(got - want) < 1e-4;
  if (ok) {
    printf("  PASS  %s = %.4f\n", name, got);
    passed++;
  } else {
    printf("  FAIL  %s = %.4f (want %.4f)\n", name, got, want);
    failed++;
  }
}

int main() {
  printf("\nTest 1: Simple LP (no Phase I)\n");
  printf("  max 5x + 4y  s.t. 6x+4y<=24, x+2y<=6\n");
  // Answer: x=3, y=1.5, obj=21
  {
    SimplexSolver solver({5, 4}, {{6,4},{1,2}}, {24, 6}, {}, {});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 21.0);
    check_val("x", res.opt_sol[0], 3.0);
    check_val("y", res.opt_sol[1], 1.5);
  }

  printf("\nTest 2: Equality constraint (Phase I)\n");
  printf("  max x+y  s.t. x+y=10, x<=8, y<=7\n");
  // Answer: obj=10, x+y=10
  {
    SimplexSolver solver({1,1}, {{1,0},{0,1}}, {8,7}, {{1,1}}, {10});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 10.0);
    check_val("x+y", res.opt_sol[0] + res.opt_sol[1], 10.0);
  }

  printf("\nTest 3: Arb LP (d1=3, d2=2, B=6, degeneracy)\n");
  printf("  max z  s.t. -2x1+x2+z<=0, x1-x2+z<=0, x1+x2=6\n");
  // Answer: x1=2.4, x2=3.6, z=1.2
  {
    SimplexSolver solver(
        {0, 0, 1},
        {{-2, 1, 1}, {1, -1, 1}},
        {0, 0},
        {{1, 1, 0}},
        {6});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 1.2);
    check_val("x1", res.opt_sol[0], 2.4);
    check_val("x2", res.opt_sol[1], 3.6);
    check_val("z", res.opt_sol[2], 1.2);
  }

  printf("\nTest 4: Infeasible\n");
  printf("  max x  s.t. x<=5, x>=10\n");
  {
    SimplexSolver solver({1}, {{1},{-1}}, {5,-10}, {}, {});
    auto res = solver.solve();
    check("infeasible", !res.feasible);
  }

  printf("\nTest 5: Unbounded\n");
  printf("  max x+y  s.t. x-y<=5\n");
  {
    SimplexSolver solver({1,1}, {{1,-1}}, {5}, {}, {});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("unbounded", !res.bounded);
  }

  printf("\nTest 6: >= constraint (constructor 2)\n");
  printf("  max 2x+3y  s.t. x+y>=4, x<=6, y<=5\n");
  // Answer: x=6, y=5, obj=27
  {
    SimplexSolver solver(
        {2, 3},
        {{1, 1}, {1, 0}, {0, 1}},
        {4, 6, 5},
        {">=", "<=", "<="});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 27.0);
    check_val("x", res.opt_sol[0], 6.0);
    check_val("y", res.opt_sol[1], 5.0);
  }

  printf("\nTest 7: Negative b with <=\n");
  printf("  max x+y  s.t. -x-y<=-10, x<=8, y<=7\n");
  // answer is x=8, y=7, obj=15
  {
    SimplexSolver solver({1,1}, {{-1,-1},{1,0},{0,1}}, {-10,8,7}, {}, {});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 15.0);
    check_val("x", res.opt_sol[0], 8.0);
    check_val("y", res.opt_sol[1], 7.0);
  }

  printf("\nTest 8: Negative b with >=\n");
  printf("  max -x-y  s.t. x+y>=0, x>=-3, y>=-4\n");
  // Answer: x=5, y=4, obj=14
  {
    SimplexSolver solver(
        {2, 1},
        {{-1, 1}, {1, 0}, {0, 1}},
        {-3, 5, 4},
        {">=", "<=", "<="});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 14.0);
    check_val("x", res.opt_sol[0], 5.0);
    check_val("y", res.opt_sol[1], 4.0);
  }

  printf("\nTest 9: Equality with negative b\n");
  printf("  max x+y  s.t. x-y=-2, x<=5\n");
  // Answer: y=7, x=5, obj=12
  {
    SimplexSolver solver({1,1}, {{1,0}}, {5}, {{1,-1}}, {-2});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 12.0);
    check_val("x", res.opt_sol[0], 5.0);
    check_val("y", res.opt_sol[1], 7.0);
  }

  printf("\nTest 10: All zeros (trivial)\n");
  printf("  max 0  s.t. x<=5\n");
  {
    SimplexSolver solver({0}, {{1}}, {5}, {}, {});
    auto res = solver.solve();
    check("feasible", res.feasible);
    check("bounded", res.bounded);
    check_val("obj", res.opt_val, 0.0);
  }

  printf("\n %d passed, %d failed\n", passed, failed);
  return failed > 0 ? 1 : 0;
}