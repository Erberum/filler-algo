from enum import Enum

ROWS = 7
COLS = 8


class Color(Enum):
    RED = 0
    GREEN = 1
    YELLOW = 2
    BLUE = 3
    PURPLE = 4
    GRAY = 5


COLORS_STR = [c.name for c in Color]
