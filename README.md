# filler-algo

Algorithmic solver for the Filler problem (state space >3e+32) ([Rules](#filler-problem-rules))

## Overview & Performance

Full-depth minimax search (~30 turns, ~1e+7 states after pruning) is used to find mathematically perfect solution.

Runs in ~1 second on M1 Pro using the following optimizations:

- Alpha-Beta Pruning with move-ordering heuristics
- Pruning of mathematically irrelevant branches
- Bitboards & precomputed masks for board manipulation
- Hardware acceleration with POPCNT
- Branch prediction optimizations
- Structs memory alignment
- ~~Multi-threading~~ (removed as it conflicts with alpha-beta pruning and decreases performance)

### Additional features

- **Board recognition** through Python **OpenCV**

## Installation

1. Install dependencies through poetry

```bash
poetry install
```

2. Activate the virtual environment

```bash
source $(poetry env info -p)/bin/activate
```

3. Build

```bash
pip install -e .
```

## Running

```bash
filler-solve
```

## Problem rules

- 8×7 board with random colors
- Two players start in opposite corners
- Players alternate choosing one of six colors
- Selecting a color expands their territory to adjacent cells of that color
- Goal: control the largest area by the end

## Inspiration

After watching those videos, I was inspired to create an algorithm that finds the perfect solution—something that, as
far as I know, hasn’t been done before.

- [YouTube. Algorithm by JoeCoup: only 8 turns deep Minimax search with very interesting interface and StockFish-like evaluation bar.](https://www.youtube.com/watch?v=7EH1QzzXKM0)
- [GitHub. Algorithm by ItsAsShrimpleAsThat: 20 turns deep search. Really cool web gui. Iterative deepening. Quiescence search with limit.](https://github.com/ItsAsShrimpleAsThat/FillerSolver)
