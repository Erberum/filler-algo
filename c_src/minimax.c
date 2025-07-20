#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "minimax.h"

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

GameState create_game(const uint8_t colors[ROWS][COLS], bool second_player_starts) {
    GameState gamestate = {0};
    gamestate.players[0] = PLAYER1_START;
    gamestate.players[1] = PLAYER2_START;
    gamestate.player_colors[0] = colors[ROWS - 1][0];
    gamestate.player_colors[1] = colors[0][COLS - 1];
    gamestate.current_player = second_player_starts; // Player 0 for first, player 1 for second

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
    uint64_t *player_bitboard = &state->players[state->current_player];
    uint8_t old_color = state->player_colors[state->current_player];

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

void simulate_actions_sequence(GameState *state, uint8_t actions[], uint8_t n_actions) {
    for (int i = 0; i < n_actions; i++) {
        simulate_action(state, actions[i]);
    }
}

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
    printf("-=-=-=-=-=-\n");
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


MinimaxNode minimax_inner(const GameState *state, uint8_t depth, uint8_t max_depth, int8_t alpha, int8_t beta) {
    bool maximising = !state->current_player; // Player 0 for maximising, 1 for minimising

    uint8_t occupied = tiles_occupied(state);
    if (depth == max_depth || occupied == ROWS * COLS) {
        return (MinimaxNode){.score = score_state(state)};
    }

    MinimaxNode children[COLORS];

    Action actions[COLORS];
    uint8_t n_valid_actions;
    get_actions(state, actions, &n_valid_actions);

    uint32_t n_reachable = 0;
    MinimaxNode *best_child = &children[0];
    for (int i = 0; i < n_valid_actions; i++) {
        MinimaxNode *child = &children[i];
        Action *action = &actions[i];

        *child = minimax_inner(&action->result, depth + 1, max_depth, alpha, beta);

        child->actions_trace[child->n_actions++] = action->color;
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

MinimaxNode minimax(const GameState *state, uint8_t depth) {
    return minimax_inner(state, 0, depth, INT8_MIN, INT8_MAX);
}

//int main() {
//    uint8_t colors[ROWS][COLS] = {
//        {2, 4, 1, 0, 2, 0, 2, 4},
//        {4, 2, 5, 3, 4, 2, 5, 1},
//        {3, 1, 2, 1, 2, 3, 4, 3},
//        {4, 5, 3, 5, 0, 5, 0, 2},
//        {5, 0, 2, 3, 1, 3, 2, 3},
//        {1, 3, 1, 0, 3, 2, 5, 1},
//        {4, 2, 0, 2, 4, 1, 0, 5},
//    };
//    GameState state = create_game(colors, true);
//
//    uint8_t actions[] = {};
//    uint8_t n_actions = sizeof(actions) / sizeof(uint8_t);
//    simulate_actions_sequence(&state, actions, n_actions);
//    printf("Action history: ");
//    for (int i = 0; i < n_actions; i++) {
//        printf("%i ", actions[i]);
//    }
//    printf("\n");
//    print_game(&state);
//
//    clock_t start = clock();
//    MinimaxNode terminal = minimax(&state, 56);
//    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
//    printf("-=-=-=-=-=-\n");
//    printf("%.2lfm states, %.3f sec\n", terminal.n_reachable / 1e6, elapsed);
//    // printf("Max tiles occupied: %i\n", max_tiles_occupied);
//    printf("Game Ended (moves=%i, score=%i) <- ", terminal.n_actions, terminal.score);
//    for (int i = 0; i < terminal.n_actions; i++) {
//        printf("%i ", terminal.actions_trace[i]);
//    }
//    printf("\n");
//
//    return 0;
//}


void test_print(int kek) {
    printf("%i", kek);
}
