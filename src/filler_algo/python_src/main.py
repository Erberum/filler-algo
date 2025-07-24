import ctypes
import os
from copy import deepcopy

from filler_algo.c_src.minimax import PyGameStateWrapper

from filler_algo.python_src.game import Color, COLORS_STR
from filler_algo.python_src.vision import extract_game_board
import sys


def input_action(text: str) -> Color:
    while True:
        inp = input(text)
        if inp.upper() in COLORS_STR:
            return Color[inp.upper()]
        raise AssertionError(f'Color "{inp}" not found. Possible options: {", ".join(COLORS_STR)}')


def main():
    assert len(sys.argv) == 2, 'Usage: filler-solve <path>'

    path = sys.argv[1]
    assert os.path.exists(path), f'File not found: {path}'

    board = extract_game_board(path)
    game = PyGameStateWrapper(board, False)
    # Past actions till solution for example1.png
    # past_actions = [Color.GRAY, Color.RED, Color.YELLOW, Color.GRAY, Color.BLUE, Color.RED, Color.YELLOW, Color.BLUE,
    #                 Color.PURPLE, Color.YELLOW, Color.GRAY, Color.GREEN, Color.PURPLE, Color.BLUE, Color.GREEN,
    #                 Color.RED, Color.GRAY, Color.YELLOW, Color.GREEN, Color.GRAY, Color.BLUE, Color.GREEN, Color.YELLOW,
    #                 Color.PURPLE, Color.GRAY, Color.RED, Color.GREEN, Color.YELLOW, Color.RED]
    past_actions = []

    for action in past_actions:
        game.simulate(action)

    while True:
        print('---')
        print(game)
        if game.is_ended:
            break
        solution, score = game.filler_solve()
        try:
            action = input_action(f'Player {game.current_player + 1} [best={solution[0].name}{score:+}]: ')
            game.simulate(action)
        except AssertionError as e:
            print(e)
