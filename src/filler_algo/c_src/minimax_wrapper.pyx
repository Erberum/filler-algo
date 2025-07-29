import time
from typing import Iterable

from libc.stdint cimport uint8_t, uint32_t, int8_t, uint64_t
from libc.stdlib cimport malloc, free
from libc.string cimport memcpy

from filler_algo.python_src.game import Color

cdef extern from "minimax.h":
    DEF ROWS = 7
    DEF COLS = 8
    DEF COLORS = 6

    ctypedef struct GameState:
        uint64_t colors[COLORS];
        uint64_t players[2];
        uint8_t player_colors[2];
        bint current_player;

    ctypedef struct MinimaxNode:
        uint32_t n_reachable;
        uint8_t actions_trace[ROWS * COLS];
        uint8_t n_actions;
        int8_t score;

    bint is_action_allowed_wrapper(const GameState *state, uint8_t action)
    GameState create_game(const uint8_t colors[7][8], bint second_player_starts)
    void simulate_action(GameState *state, uint8_t new_color)
    MinimaxNode minimax(const GameState *state, uint8_t depth)
    uint8_t tiles_occupied(const GameState *state)
    void print_game(const GameState *state)
    int8_t score_state(const GameState *state)

cdef class PyGameStateWrapper:
    cdef GameState * c_state;

    @staticmethod
    cdef GameState __c_create_game_state(board: list[list[Color]], second_player_starts: bint = False):
        # Convert python 2D board to C array
        cdef uint8_t c_board[ROWS][COLS]
        assert len(board) == ROWS, f'Game board must have {ROWS} rows, got {len(board)}'
        for row in range(ROWS):
            assert len(board[row]) == COLS, f'Board must have {COLS} columns, got {len(board[row])}'
            for col in range(COLS):
                c_board[row][col] = board[row][col].value
        return create_game(c_board, second_player_starts)

    def __cinit__(self, board: list[list[Color]], second_player_starts = False):
        cdef GameState tmp = PyGameStateWrapper.__c_create_game_state(board, second_player_starts)
        self.c_state = <GameState *> malloc(sizeof(GameState))
        if not self.c_state:
            raise MemoryError('Failed to allocate GameState')
        memcpy(self.c_state, &tmp, sizeof(GameState))

    def __dealloc__(self):
        if self.c_state is not NULL:
            free(self.c_state)
            self.c_state = NULL

    def simulate(self, action: Color):
        assert self.is_action_allowed(action), f'Action not allowed: {action}'
        simulate_action(self.c_state, action.value)

    def is_action_allowed(self, action: Color):
        return is_action_allowed_wrapper(self.c_state, action.value)

    def print_game(self):
        print_game(self.c_state)

    def filler_solve(self, uint8_t depth = 56) -> tuple[list[Color], int]:
        start = time.time()
        cdef MinimaxNode terminal = minimax(self.c_state, depth)
        elapsed = time.time() - start
        print(f'Minimax({terminal.n_reachable:.2e} states, {elapsed:.2f} sec, depth={depth})')

        # Convert C "path to optimal node" array to python list
        optimal_actions = [Color(terminal.actions_trace[i]) for i in range(terminal.n_actions)]
        optimal_actions = optimal_actions[::-1]  # Reverse because C algorithm traces backwards from the terminal node

        return optimal_actions, terminal.score
    @property
    def current_player(self):
        return self.c_state.current_player

    @property
    def is_ended(self):
        return tiles_occupied(self.c_state) == ROWS * COLS

    @property
    def tiles_occupied(self):
        return tiles_occupied(self.c_state)

    @property
    def score(self):
        return score_state(self.c_state)

    def get_player_color(self, player: int):
        return Color(self.c_state.player_colors[player])

    def __repr__(self):
        p1_color = self.get_player_color(0).name
        p2_color = self.get_player_color(1).name
        return (f'GameState( '
                f'{self.tiles_occupied}/{ROWS * COLS}, '
                f'player={self.current_player + 1}, '
                f'p1={p1_color}, '
                f'p2={p2_color}, '
                f'score={self.score:+} '
                f')')
