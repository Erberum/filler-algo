import time

from libc.stdint cimport uint8_t, uint32_t, int8_t


cdef extern from "minimax.h":
    DEF ROWS = 7
    DEF COLS = 8
    DEF COLORS = 6

    ctypedef struct GameState:
        pass

    ctypedef struct MinimaxNode:
        uint32_t n_reachable;
        uint8_t actions_trace[ROWS * COLS];
        uint8_t n_actions;
        int8_t score;

    GameState create_game(const uint8_t colors[7][8], bint second_player_starts)
    void simulate_action(GameState *state, uint8_t new_color)
    MinimaxNode minimax(const GameState *state, uint8_t depth)
    void print_game(const GameState *state)



def filler_solve(board: list[list[int]], bint second_player_starts, previous_actions: list[int], uint8_t depth = 56) -> Tuple[List[int], int]:
    """
    :return: List of optimal actions and resulting score
    """
    assert len(previous_actions) < ROWS * COLS, f'Number of actions should not exceed {ROWS * COLS}'

    # Convert python 2D board to C array
    cdef uint8_t c_board[ROWS][COLS]
    assert len(board) == ROWS, f'Game board must have {ROWS} rows, got {len(board)}'
    for row in range(ROWS):
        assert len(board[row]) == COLS, f'Board must have {COLS} columns, got {len(board[row])}'
        for col in range(COLS):
            c_board[row][col] = board[row][col]

    cdef GameState state = create_game(c_board, second_player_starts)

    # Convert python actions list to C array
    for action in previous_actions:
        assert 0 < action < COLORS, f'Color out of boundaries (0 < got {action} < {COLORS})'
        simulate_action(&state, <uint8_t> action)

    # Run minimax
    start = time.time()
    cdef MinimaxNode terminal = minimax(&state, depth)
    elapsed = time.time() - start
    print(f'Minimax({terminal.n_reachable / 1e6:.2f}m states, {elapsed:.2f} sec)')

    # Convert C "path to optimal node" array to python list
    optimal_actions = [terminal.actions_trace[i] for i in range(terminal.n_actions)]
    optimal_actions = optimal_actions[::-1]  # Reverse because C algorithm traces backwards from the terminal node
    return optimal_actions, terminal.score
    # print_game(&state)
