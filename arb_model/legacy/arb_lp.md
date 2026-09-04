# Simplifying the Multi-Casino Arbitrage LP

## Notation

- $\text{bet}_{ij}$ = amount of money bet on event $i$ at casino $j$
- $\text{gain}_{ij}$ = gain if event $i$ happens, at casino $j$
- $T$ = `input(total_bet)`

## Original LP

$$
\begin{aligned}
\max_{\text{bet}} \; \min_i \quad & \sum_j \text{bet}_{ij}\,\text{gain}_{ij} - \sum_i \sum_j \text{bet}_{ij} \\
\text{s.t.} \quad & \sum_j \text{bet}_{ij}\,\text{gain}_{ij} - \text{gain}_{ij} - \sum_i \sum_j \text{bet}_{ij} \ge 0 \quad \forall i \\
& \sum_j \text{bet}_{ij} = T \\
& \sum_{i,j} \text{bet}_{ij} = T \\
& \text{bet}_{ij} \ge 0 \quad \forall i, j
\end{aligned}
$$

## Reduction to one casino per event

For each event $i$,
$$
\sum_j \text{bet}_{ij}\,\text{gain}_{ij}
\;\le\; \sum_j \text{bet}_{ij} \cdot \max_j \text{gain}_{ij}
\;=\; \text{gain}_{i j^*} \sum_j \text{bet}_{ij},
\qquad j^* = \arg\max_j \text{gain}_{ij}.
$$

That is, the total bet on event $i$ pays at most as much as if it were all placed at casino $j^*$. So take $j^* = \arg\max_j \text{gain}_{ij}$ for each $i$ and put the whole stake for event $i$ there.

## Simplified LP

Let $\text{bet}_i$ = total bet on event $i$ (at casino $j^*$) and $\text{gain}_i = \text{gain}_{i j^*}$.

$$
\begin{aligned}
\max_{\text{bet}} \; \min_i \quad & \text{bet}_i\,\text{gain}_i - \sum_i \text{bet}_i \\
\text{s.t.} \quad & \text{bet}_i\,\text{gain}_i - \sum_i \text{bet}_i \ge 0 \quad \forall i \\
& \sum_i \text{bet}_i = T \\
& \text{bet}_i \ge 0 \quad \forall i
\end{aligned}
$$

## Equivalent LP (epigraph form)

$$
\begin{aligned}
\max_{\text{bet},\, z} \quad & z \\
\text{s.t.} \quad & \text{bet}_i\,\text{gain}_i - \sum_i \text{bet}_i \ge z \quad \forall i \\
& \sum_i \text{bet}_i = T \\
& \text{bet}_i \ge 0 \quad \forall i, \qquad z \ge 0
\end{aligned}
$$

## Equivalent LP (equality form)

Add slack variables $s_i \ge 0$:

$$
\begin{aligned}
\max_{\text{bet},\, z,\, s} \quad & z \\
\text{s.t.} \quad & z - \text{bet}_i\,\text{gain}_i + \sum_i \text{bet}_i + s_i = 0 \quad \forall i \\
& \sum_i \text{bet}_i = T \\
& \text{bet}_i,\; s_i \ge 0 \quad \forall i, \qquad z \ge 0
\end{aligned}
$$