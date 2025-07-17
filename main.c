
// Crashes if the same color at the start which is

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 7
#define COLS 8
#define COLORS 6
#define MULTITHREADING_DEPTH 2
#define DO_SPLIT_THREADS(depth) (depth < MULTITHREADING_DEPTH)

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

// #define FILL_MASK(player_bitboard, color_bitboard) EXPAND_MASK(player_bitboard) & color_bitboard

// #define EXPAND(player_bitboard, new_color) (EXPAND_MASK(player_bitboard) & P)
void *minimax_thread(void *arg);

typedef struct {
    uint64_t colors[COLORS];
    uint64_t players[2];
    uint8_t player_colors[2]; // Avoids iterating through color bitboards to check
    bool current_player;      // 0 or 1
} GameState;                  // BitBoard for the board

void print_bitboard(uint64_t bitboard) {
    // The least significant bit is row 0 col 0, the next bit is row 0 col 1 etc.
    uint8_t *byte = (uint8_t *)&bitboard;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            printf("%c", ((1 << col) & byte[row]) ? '1' : '0');
        }
        printf("\n");
    }
}

GameState create_game(const uint8_t colors[ROWS][COLS]) {
    GameState gamestate = {0};
    gamestate.players[0] = PLAYER1_START;
    gamestate.players[1] = PLAYER2_START;
    gamestate.player_colors[0] = colors[ROWS - 1][0];
    gamestate.player_colors[1] = colors[0][COLS - 1];

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            uint8_t color = colors[row][col];
            assert(color >= 0 && color < COLORS);
            gamestate.colors[color] |= POS_MASK(row, col);
        }
    }
    return gamestate;
}

int8_t score_state(const GameState *state) {
    return __builtin_popcountll(state->players[0]) - __builtin_popcountll(state->players[1]);
}

uint8_t tiles_occupied(const GameState *state) {
    return __builtin_popcountll(state->players[0]) + __builtin_popcountll(state->players[1]);
}

void simulate_action(GameState *state, uint8_t new_color) {
    // printf("Choosing %i\n", new_color);
    uint64_t *player_bitboard = &state->players[state->current_player];
    uint8_t old_color = state->player_colors[state->current_player];

    // // Comment for performance boost
    // assert(new_color != old_color);  // Players can't choose the old color
    // assert(state->player_colors[!state->current_player] != new_color);  // Players can't have the same color

    assert(IS_ACTION_ALLOWED(*state, new_color));

    // Update player area with newly acquired tiles
    *player_bitboard |= (EXPAND_MASK(*player_bitboard) & state->colors[new_color]);
    // Fill new color bitboard with player area
    state->colors[new_color] |= *player_bitboard;
    // Null old color bitboard on player area
    state->colors[old_color] &= ~*player_bitboard;

    // Next turn
    state->player_colors[state->current_player] = new_color;
    state->current_player = !state->current_player;
}

// void assert_gamestate_valid(const GameState* game) {

// }
uint8_t get_color_at(const GameState *state, int row, int col) {
    for (uint8_t color = 0; color < COLORS; color++) {
        if ((state->colors[color] & POS_MASK(row, col))) {
            return color;
        }
    }
    fprintf(stderr, "Error: No color found at (%d, %d)\n", row, col);
    exit(-1);
}

void print_game(const GameState *state) {
    printf("Turn: Player %i\n", (int)state->current_player + 1);
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            printf("%i", get_color_at(state, row, col));
        }
        printf("\n");
    }
}

void print_binary(const void *obj, size_t size) {
    const uint8_t *bytes = (const uint8_t *)obj;
    for (int byte = size - 1; byte >= 0; byte--) {
        for (int bit = 7; bit >= 0; bit--) {
            printf("%c", ((1 << bit) & bytes[byte]) ? '1' : '0');
        }
    }
    printf("\n");
}

int increasing(const void *a, const void *b) {
    uint8_t int_a = *(const uint8_t *)a;
    uint8_t int_b = *(const uint8_t *)b;
    return int_a - int_b;
}

typedef struct {
    GameState result;
    uint8_t color;
    uint8_t tiles_increase;
} Action;

void print_action(const Action *action) {
    printf("Action(Color=%u. Increase=%u)\n", action->color, action->tiles_increase);
}

int compare_actions(const void *a, const void *b) {
    Action *action1 = (Action *)a;
    Action *action2 = (Action *)b;
    return action2->tiles_increase - action1->tiles_increase;
}

void get_actions(const GameState *state, Action *actions, uint8_t *n_actions) {
    int tiles_before = tiles_occupied(state);
    *n_actions = 0;
    for (uint8_t color = 0; color < COLORS; color++) {
        if (!IS_ACTION_ALLOWED(*state, color)) continue;
        Action *action = actions + *n_actions;
        action->color = color;
        action->result = *state;
        simulate_action(&action->result, color);
        action->tiles_increase = tiles_occupied(&action->result) - tiles_before;
        if (action->tiles_increase == 0) continue; // Mathematically irrelevant
        (*n_actions)++;
    }
    qsort(actions, *n_actions, sizeof(Action), compare_actions);
}

typedef struct {
    uint32_t n_reachable;
    int8_t score;
    uint8_t actions_trace[ROWS * COLS];
    uint8_t n_actions;
} MinimaxNode;

typedef struct {
    const GameState *state;
    int depth;
    int max_depth;
    MinimaxNode result;
} MinimaxThreadArguments;

MinimaxNode minimax(const GameState *state, int depth, int max_depth, int8_t alpha, int8_t beta) {
    bool maximising = !state->current_player; // Player 0 for maximising, 1 for minimising

    // Areas for improvement: gamestates pool with preallocated memory
    // Multi-Threading

    uint8_t occupied = tiles_occupied(state);
    // printf("123 %i\n", score_state(state));
    if (depth == max_depth || occupied == ROWS * COLS) {
        return (MinimaxNode){.score = score_state(state)};
    }

    MinimaxNode children[COLORS];

    Action actions[COLORS];
    uint8_t n_valid_actions;
    get_actions(state, actions, &n_valid_actions);

    if (DO_SPLIT_THREADS(depth)) {
        pthread_t threads[n_valid_actions];
        MinimaxThreadArguments threads_arguments[n_valid_actions];
        for (int i = 0; i < n_valid_actions; i++) {
            threads_arguments[i] = (MinimaxThreadArguments){&actions[i].result, depth + 1, max_depth};
            pthread_create(&threads[i], NULL, minimax_thread, &threads_arguments[i]);
        }
        for (int i = 0; i < n_valid_actions; i++) {
            pthread_join(threads[i], NULL);
            children[i] = threads_arguments[i].result;
        }
    }

    MinimaxNode *best_child = &children[0];
    uint32_t n_reachable = 0;
    for (int i = 0; i < n_valid_actions; i++) {
        MinimaxNode *child = &children[i];
        if (!DO_SPLIT_THREADS(depth)) {
            *child = minimax(&actions[i].result, depth + 1, max_depth, alpha, beta);
        }
        child->actions_trace[child->n_actions++] = actions[i].color;
        n_reachable += child->n_reachable + 1;

        if (maximising && child->score > alpha) alpha = child->score;
        if (!maximising && child->score < beta) beta = child->score;

        if (maximising ? child->score > best_child->score : child->score < best_child->score) {
            best_child = child;
        }
        if (beta <= alpha) break;
    }
    best_child->n_reachable = n_reachable;
    return *best_child;
}

void *minimax_thread(void *arg) {
    MinimaxThreadArguments *args = (MinimaxThreadArguments *)arg;
    args->result = minimax(args->state, args->depth, args->max_depth, INT8_MIN, INT8_MAX);
    return NULL;
}

int main() {
    uint8_t colors[ROWS][COLS] = {
        {2, 4, 1, 0, 2, 0, 2, 4},
        {4, 2, 5, 3, 4, 2, 5, 1},
        {3, 1, 2, 1, 2, 3, 4, 3},
        {4, 5, 3, 5, 0, 5, 0, 2},
        {5, 0, 2, 3, 1, 3, 2, 3},
        {1, 3, 1, 0, 3, 2, 5, 1},
        {3, 2, 0, 2, 4, 1, 0, 5},
    };
    GameState state = create_game(colors);
    state.current_player = 1;

    uint8_t actions[] = {};
    printf("Action history: ");
    for (int i = 0; i < sizeof(actions) / sizeof(uint8_t); i++) {
        printf("%i ", actions[i]);
        simulate_action(&state, actions[i]);
    }
    printf("\n");
    print_game(&state);

    clock_t start = clock();
    MinimaxNode terminal = minimax(&state, 0, 56, INT8_MIN, INT8_MAX);
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("-=-=-=-=-=-\n");
    printf("%.2lfm states, %.3f sec\n", terminal.n_reachable / 1e6, elapsed);
    // printf("Max tiles occupied: %i\n", max_tiles_occupied);
    printf("Game Ended (moves=%i, score=%i) <- ", terminal.n_actions, terminal.score);
    for (int i = 0; i < terminal.n_actions; i++) {
        printf("%i ", terminal.actions_trace[i]);
    }
    printf("\n");

    return 0;
}
