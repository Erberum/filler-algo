from minimax import filler_solve

field = [
        [2, 4, 1, 0, 2, 0, 2, 4],
        [4, 2, 5, 3, 4, 2, 5, 1],
        [3, 1, 2, 1, 2, 3, 4, 3],
        [4, 5, 3, 5, 0, 5, 0, 2],
        [5, 0, 2, 3, 1, 3, 2, 3],
        [1, 3, 1, 0, 3, 2, 5, 1],
        [4, 2, 0, 2, 4, 1, 0, 5],
]

optimal_actions, score = filler_solve(field, True, [])
print(optimal_actions, score)