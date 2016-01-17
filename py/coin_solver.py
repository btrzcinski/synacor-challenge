#!/usr/bin/python3

from itertools import permutations

def eval_equation(vals):
    return vals[0] + vals[1] * (vals[2] ** 2) + (vals[3] ** 3) - vals[4]

def equation_solver_with_num_set(nums=[2,3,5,7,9]):
    for p in permutations(nums):
        if eval_equation(p) == 399:
            return p

def main():
    print(equation_solver_with_num_set())

if __name__ == "__main__":
    main()

