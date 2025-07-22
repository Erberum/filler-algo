import os
from copy import deepcopy

from filler_algo.c_src.minimax import filler_solve

from filler_algo.python_src.game import Color, COLORS_STR
from filler_algo.python_src.vision import extract_game_board
import sys


def input_action(text: str):
    while True:
        inp = input(text)
        if inp.upper() in COLORS_STR:
            return Color[inp.upper()]
        print(f'Color "{inp}" not found. Possible options: {", ".join(COLORS_STR)}')


def main():
    assert len(sys.argv) == 2, 'Usage: filler-solve <path>'

    path = sys.argv[1]
    assert os.path.exists(path), f'File not found: {path}'

    board = extract_game_board(path)
    print(board)

    past_actions = []

    player1_turn = True
    while True:
        # print(board)
        solution, score = filler_solve(board, False, past_actions)
        # print(solution)

        action = input_action(f'Player {1 if player1_turn else 2} [{solution[0].name}, end={score:+}]: ')
        print(f'Added {action}')
        past_actions.append(action)

        player1_turn = not player1_turn
