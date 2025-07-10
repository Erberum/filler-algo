#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#define ROWS 7
#define COLS 8
#define COLORS 6

#define BIT_POSITION(row, col) (row * COLS + col)
#define POS_MASK(row, col) ((uint64_t) 1 << BIT_POSITION(row, col))

#define ROW0 ((uint64_t) 0xFF)
#define ROW6 ((uint64_t) 0x00FF000000000000)
#define ROW7 ((uint64_t) 0xFF00000000000000)
#define COL0 ((uint64_t) 0x0101010101010101)
#define COL7 ((uint64_t) 0x8080808080808080)

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


typedef struct {
	uint64_t players[2];
    uint8_t player_colors[2];  // Avoids iterating through color bitboards to check
    uint64_t colors[COLORS];
    bool current_player;  // 0 or 1
} GameState;  // BitBoard for the board


void print_bitboard(uint64_t bitboard) {
    // The least significant bit is row 0 col 0, the next bit is row 0 col 1 etc.
    uint8_t* byte = (uint8_t*) &bitboard;
    for (int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
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
    
    for(int row = 0; row < ROWS; row++) {
        for(int col = 0; col < COLS; col++) {
            uint8_t color = colors[row][col];
            assert(color >= 0 && color < COLORS);
            gamestate.colors[color] |= POS_MASK(row, col);
        }
    }
    return gamestate;
}

int8_t score_state(const GameState* state) {
    return __builtin_popcountll(state->players[0]) - __builtin_popcountll(state->players[1]);
}

void simulate_action(GameState* state, uint8_t new_color) {
    // printf("Choosing %i\n", new_color); 
    uint64_t* player_bitboard = &state->players[state->current_player];
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
uint8_t get_color_at(const GameState* state, int row, int col) {
    for(uint8_t color = 0; color < COLORS; color++) {
        if((state->colors[color] & POS_MASK(row, col))) {
            return color;
        }
    }
    fprintf(stderr, "Error: No color found at (%d, %d)\n", row, col);
    exit(-1);
}

void print_game(const GameState* state) {
    printf("Turn: Player %i\n", (int) state->current_player + 1);
    for(int row = 0; row < ROWS; row++) {
        for(int col = 0; col < COLS; col++) {
            printf("%i", get_color_at(state, row, col));
        }
        printf("\n");
    }
}

void print_binary(const void* obj, size_t size) {
    const uint8_t* bytes = (const uint8_t*) obj;
    for(int byte = size - 1; byte >= 0; byte--) {
        for(int bit = 7; bit >= 0; bit--) {
            printf("%c", ((1 << bit) & bytes[byte]) ? '1' : '0');
        }
    }
    printf("\n");
}

int8_t minimax(const GameState* state, int depth, bool maximising) {
    // Areas for improvement: gamestates pool with preallocated memory
    // Multi-Threading
    // Alpha-beta prunning
    // Do most functions inline
    if(depth == 0) {
        // printf("%i ", score_state(state));
        return score_state(state);
    }

    int8_t best_score = maximising ? INT8_MIN : INT8_MAX;
    uint8_t best_action;
    for(uint8_t action = 0; action < COLORS; action++) {
        if(!IS_ACTION_ALLOWED(*state, action)) continue;
        GameState next_state = *state;
        simulate_action(&next_state, action);
        int8_t score = minimax(&next_state, depth - 1, !maximising);
        if(maximising ? score > best_score : score < best_score) {
            best_score = score;
            best_action = action;
        }
    }
    return best_score;
}

int main() {
    uint8_t colors[ROWS][COLS] = {
        {2, 3, 1, 4, 0, 5, 4, 1},
        {4, 5, 4, 0, 5, 1, 2, 0},
        {0, 2, 3, 2, 0, 2, 3, 1},
        {1, 3, 2, 0, 5, 3, 2, 3},
        {3, 5, 4, 1, 4, 2, 3, 5},
        {1, 2, 1, 5, 0, 3, 4, 3},
        {5, 4, 2, 4, 3, 4, 0, 5},
    };
    GameState state = create_game(colors);

    // print_game(&state);
    // simulate_action(&state, 4);
    // print_game(&state);
    // simulate_action(&state, 2);
    // print_game(&state);
    // printf("Score: %i\n", score_state(&state));
    clock_t start = clock();
    printf("%i\n", minimax(&state, 10, true));
    clock_t end = clock();
    double elapsed = (double) (end - start) / CLOCKS_PER_SEC;
    printf("Finished in %f sec\n", elapsed);

    // print_game(&state);
    // simulate_action(&state, 4);
    // print_game(&state);
    return 0;
}
