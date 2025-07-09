#include <stdio.h>
#include <stdint.h>

#define ROWS 7
#define COLS 8

#define MAX_NUMBER 6 
#define BIT_POSITION(row, col) (row * COLS + col)
#define POS_MASK(row, col) ((uint64_t) 1 << BIT_POSITION(row, col))
#define UP_MASK(row, col) POS_MASK(row, col) << 8


typedef struct {
	uint64_t p1_bitboard;
	uint64_t p2_bitboard;	
    uint64_t colors[6];
} Board;


void print_binary(const void* obj, size_t size) {
    const uint8_t* bytes = (const uint8_t*) obj;
    for(int byte = size; byte >= 0; byte--) {
        for(int bit = 7; bit >= 0; bit--) {
            printf("%c", ((1 << bit) & bytes[byte]) ? '1' : '0');
        }
    }
    printf("\n");
}

// void print_bitboard(uint64_t bitboard) {
//     uint8_t* bit_row = (uint8_t*) &bitboard;
//     for (int i = 0; i < 8; i++, bit_row++) {
//         print_binary(bit_row, 1);
//     }
// }

int main() {
    printf("%lu\n", sizeof(Board));
    // int i = 5;
    // print_binary(&i, sizeof(i));
    uint64_t bitboard = 0;
    // print_bitboard(bitboard);
    uint64_t mask = POS_MASK(0, 0) << 0;
    print_binary(&mask, sizeof(mask));
    // print_bitboard(POS_MASK(1, 1));
    // printf("%lu\n", POS_MASK(1, 0));
    // Board board = {};
    // print_board(board);
    return 0;
}
