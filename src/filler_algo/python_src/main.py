import os

from filler_algo.c_src.minimax import filler_solve

from filler_algo.python_src.game import Color
from filler_algo.python_src.vision import extract_game_board
import sys


def main():
    if len(sys.argv) != 2:
        print('Usage: filler-solve <path>')

    path = sys.argv[1]
    if not os.path.exists(path):
        raise FileNotFoundError(path)

    enum_board = extract_game_board(path)
    board = [[color.value for color in row] for row in enum_board]

    solution, score = filler_solve(board, False, [])
    enum_solution = [Color(c) for c in solution]
    print(', '.join(c.name for c in enum_solution))
