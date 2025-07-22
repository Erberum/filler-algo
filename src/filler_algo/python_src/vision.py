import os.path
from enum import Enum
from typing import List

from filler_algo.python_src.game import Color, ROWS, COLS

import cv2

COLOR_MAP = {
    Color.RED: (222, 74, 90),
    Color.GREEN: (172, 210, 103),
    Color.YELLOW: (245, 221, 80),
    Color.BLUE: (88, 163, 234),
    Color.PURPLE: (102, 79, 153),
    Color.GRAY: (71, 71, 71)
}


def bgr_to_rgb(color):
    return color[::-1]


def crop_board(image_path: str):
    img = cv2.imread(image_path)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, 50, 150)

    contours, _ = cv2.findContours(edges, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    assert contours, 'No contours found'
    largest = max(contours, key=cv2.contourArea)

    # if cv2.contourArea(largest) < 4000:
    #     logging.warning('Boa')

    rect = cv2.minAreaRect(largest)
    box = cv2.boxPoints(rect)
    box = box.astype(int)

    x, y, w, h = cv2.boundingRect(box)
    cropped = img[y:y + h, x:x + w]
    # Draw contours around the
    # cv2.drawContours(img, [box], 0, (0, 0, 255), 10)
    # cv2.imshow('Board', img)
    # cv2.waitKey(0)
    # cv2.destroyAllWindows()
    return cropped


def get_closest_color(original: tuple[int, int, int]):
    closest = None
    diff = None
    for color_enum, suspect in COLOR_MAP.items():
        suspect_diff = sum((original[i] - suspect[i]) ** 2 for i in range(3))
        if diff is None or suspect_diff < diff:
            diff = suspect_diff
            closest = color_enum
    return closest


def extract_game_board(image_path: str) -> List[List[Color]]:
    cropped = crop_board(image_path)
    board_height, board_width, _ = cropped.shape
    square_height, square_width = round(board_height / ROWS), round(board_width / COLS)
    # Get color probes from the center of each square
    board = [[None for col in range(COLS)] for row in range(ROWS)]
    for row in range(ROWS):
        for col in range(COLS):
            x = square_width // 2 + col * square_width
            y = square_height // 2 + row * square_height
            color_probe = bgr_to_rgb(tuple(map(int, cropped[y, x])))
            board[row][col] = get_closest_color(color_probe)
    return board
