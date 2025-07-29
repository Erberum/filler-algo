# filler-algo

Algorithmic solver for the Filler game (state space >3e+32) ([Rules](#filler-game-rules))

## Overview & Performance

Minimax search is used to find the optimal solution.
Full-depth search (~30 turns, ~1e+7 states after pruning) runs in 1 second on M1 Pro using the following
optimizations:

- Alpha-Beta Pruning with move-ordering heuristics
- Pruning of mathematically irrelevant branches
- Bitboards & precomputed masks for board manipulation
- Hardware acceleration with POPCNT
- Branch prediction optimizations
- Structs memory alignment
- ~~Multi-threading~~ (removed - it conflicts with alpha-beta pruning and decreases performance)

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

## Filler game rules

- 8×7 board with random colors
- Two players start in opposite corners
- Players alternate choosing one of six colors
- Selecting a color expands their territory to adjacent cells of that color
- Goal: control the largest area by the end