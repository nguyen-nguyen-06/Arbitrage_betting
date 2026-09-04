# The Arbitrage LP

## Setup

Let $n \ge 1$, let $d_1, \dots, d_n > 0$ be decimal odds, and let $B > 0$ be a budget. Consider

$$
\begin{aligned}
\max_{x,\, z} \quad & z \\
\text{s.t.} \quad & d_i x_i - B \ge z \quad \text{for all } i = 1, \dots, n, \\
& \textstyle\sum_{i=1}^n x_i = B, \\
& x_i \ge 0 \quad \text{for all } i.
\end{aligned}
$$

Here $x_i$ is the stake on outcome $i$, $d_i x_i$ is the payout if $i$ wins, and $z$ is the guaranteed profit across all outcomes.

Define
$$
S = \sum_{i=1}^n \frac{1}{d_i}, \qquad M = B\left(\frac{1}{S} - 1\right).
$$

**Theorem.** The optimal value is $z^* = M$, and the unique optimal stakes are
$$
x_i^* = \frac{B}{d_i S}, \qquad i = 1, \dots, n.
$$

## Proof

The argument has three steps: (1) $M$ is a ceiling on every feasible $z$; (2) $M$ is actually reached; (3) it is reached in only one way.

### Step 1: No feasible $z$ exceeds $M$

Take any feasible pair $(x, z)$. The $i$-th constraint says $d_i x_i \ge B + z$. Since $d_i > 0$, dividing preserves the inequality:
$$
x_i \ge \frac{B + z}{d_i}. \tag{1}
$$
Adding (1) over all $i$ and using the budget constraint on the left,
$$
B = \sum_{i=1}^n x_i \ge (B + z) \sum_{i=1}^n \frac{1}{d_i} = (B + z)\, S. \tag{2}
$$
Since $S > 0$, dividing by $S$ and subtracting $B$ gives
$$
z \le \frac{B}{S} - B = M. \tag{3}
$$
Note that $M$ depends only on $B$ and the odds $d_i$, which are fixed data of the problem. Since $B$ is a constant, $M$ is the same constant for every pair $(x, z)$ satisfying $\sum_i x_i = B$; it does not change as $x$ changes. So (3) says that *every* feasible $z$ lies below one fixed number $M$. In particular $z^* \le M$.

### Step 2: $M$ is feasible

Because $z$ is a decision variable, we are free to propose any value for it and then check feasibility. Set $z = M$ and $x_i = x_i^* = B/(d_i S)$. Check each constraint:

- **Nonnegativity.** $B, d_i, S > 0$, so $x_i^* > 0$.
- **Budget.** $\displaystyle\sum_i x_i^* = \frac{B}{S} \sum_i \frac{1}{d_i} = \frac{B}{S} \cdot S = B$.
- **Profit constraints.** $d_i x_i^* = B/S$ for every $i$, and $B + M = B + B/S - B = B/S$. So $d_i x_i^* = B + M$, and the constraint holds with equality.

Thus $(x^*, M)$ is feasible. Combined with Step 1, every feasible $z$ is at most $M$ and $M$ itself is feasible, so the maximum exists and equals $M$: $z^* = M$.

### Step 3: The maximizer is unique

Let $(x, z^*)$ be *any* optimal solution (so $z = z^* = M$). Define the slack in each constraint,
$$
g_i = x_i - \frac{B + z^*}{d_i} \ge 0,
$$
where the inequality is (1). Summing, and using the budget constraint together with $(B + z^*) S = (B/S) \cdot S = B$,
$$
\sum_{i=1}^n g_i = \sum_{i=1}^n x_i - (B + z^*) S = B - B = 0.
$$
A sum of nonnegative numbers equals zero only if every term is zero. Hence $g_i = 0$ for all $i$, which means
$$
x_i = \frac{B + z^*}{d_i} = \frac{B/S}{d_i} = x_i^*.
$$
So any optimal $x$ coincides with $x^*$, and at the optimum every constraint is tight: the payout $d_i x_i^* = B/S$ is the same no matter which outcome wins. $\blacksquare$

## Corollary

An arbitrage (a guaranteed positive profit) exists if and only if $z^* > 0$. Since $B > 0$,
$$
z^* > 0 \iff \frac{1}{S} - 1 > 0 \iff S < 1 \iff \sum_{i=1}^n \frac{1}{d_i} < 1.
$$