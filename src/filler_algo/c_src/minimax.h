#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define ROWS 7
#define COLS 8
#define COLORS 6

#define BIT_POSITION(row, col) (row * COLS + col)
#define POS_MASK(row, col) ((uint64_t)1 << BIT_POSITION(row, col))

#define ROW0 ((uint64_t)0xFF)
#define ROW6 ((uint64_t)0x00FF000000000000)
#define ROW7 ((uint64_t)0xFF00000000000000)
#define COL0 ((uint64_t)0x0101010101010101)
#define COL7 ((uint64_t)0x8080808080808080)

// 8x7 board only. Last row nulled
#define UP_MASK(bitboard) ((bitboard >> COLS) & ~ROW7 & ~ROW6)
#define DOWN_MASK(bitboard) ((bitboard << COLS) & ~ROW7)
#define LEFT_MASK(bitboard) ((bitboard >> 1) & ~COL7)
#define RIGHT_MASK(bitboard) ((bitboard << 1) & ~COL0)

// Expands 1 tile in all 4 directions
#define EXPAND_MASK(bitboard) (bitboard | UP_MASK(bitboard) | DOWN_MASK(bitboard) | LEFT_MASK(bitboard) | RIGHT_MASK(bitboard))

#define PLAYER1_START (ROW6 & COL0)
#define PLAYER2_START (ROW0 & COL7)

#define IS_ACTION_ALLOWED(state, color) ((state).player_colors[(state).current_player] != color && (state).player_colors[!(state).current_player] != color)

typedef struct {
    uint64_t colors[COLORS];
    uint64_t players[2];
    uint8_t player_colors[2]; // Avoids iterating through color bitboards to check
    bool current_player;      // Player 0 or 1
} GameState;                  // BitBoard for the board

typedef struct {
    GameState result;
    uint8_t color;
    uint8_t tiles_increase;
} Action;

typedef struct {
    uint32_t n_reachable;
    uint8_t actions_trace[ROWS * COLS];
    uint8_t n_actions;
    int8_t score;
} MinimaxNode;

static inline bool is_action_allowed_wrapper(const GameState *state, uint8_t action) {
    return IS_ACTION_ALLOWED(*state, action);
}

void print_bitboard(uint64_t bitboard);
GameState create_game(const uint8_t colors[7][8], bool second_player_starts);
int8_t score_state(const GameState *state);
uint8_t tiles_occupied(const GameState *state);
void simulate_action(GameState *state, uint8_t new_color);
void simulate_actions_sequence(GameState *state, uint8_t actions[], uint8_t n_actions);
uint8_t get_color_at(const GameState *state, int row, int col);
void print_game(const GameState *state);
void print_binary(const void *obj, size_t size);
void print_action(const Action *action);
int compare_actions(const void *a, const void *b);
void get_actions(const GameState *state, Action *actions, uint8_t *n_actions);
MinimaxNode minimax_inner(const GameState *state, uint8_t depth, uint8_t max_depth, int8_t alpha, int8_t beta);
MinimaxNode minimax(const GameState *state, uint8_t depth);
void test_print(int kek);
