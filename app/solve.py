import ctypes
from ctypes import CDLL
import re

# https://pyinstaller.org/en/stable/feature-notes.html
# According to these PyInstaller docs, ctypes must be used in a specific way.
# - Cannot use ctypes.CDLL but can import CDLL and use it that way
# - Only libraries referenced by bare filenames (e.g. no leading paths) will be handled
# - Only library paths represented by a literal string will be detected

nissy = CDLL("nissy.so")

class SolveOptions(ctypes.Structure):
    _fields_ = [
        ("min_moves", ctypes.c_int),
        ("max_moves", ctypes.c_int),
        ("max_solutions", ctypes.c_int),
        ("nthreads", ctypes.c_int),
        ("optimal", ctypes.c_int),
        ("can_niss", ctypes.c_int), # c_int for boolean field
        ("verbose", ctypes.c_int), # c_int for boolean field
        ("all", ctypes.c_int), # c_int for boolean field
        ("print_only", ctypes.c_int), # c_int for boolean field
        ("count", ctypes.c_int), # c_int for boolean field
    ]

class SolveArguments(ctypes.Structure):
    _fields_ = [
        ("stepName", ctypes.c_char_p),
        ("scrambleString", ctypes.c_char_p),
        ("opts", ctypes.POINTER(SolveOptions))
    ]

# Replace all whitespace with a single space
def clean_whitespace(scramble):
    return re.sub(r"\s+", " ", scramble)

def decode_strings(result):
    solutions = []
    index = 0
    while result[index] != None:
        solutions.append(result[index].decode("utf-8"))
        index += 1
    return solutions

def solve(step_name, scramble, min_moves=1, max_moves=20, max_solutions=1, can_niss=1):
    '''
    step_name: string
    scramble: string
    solve_options: SolveOptions
    min_moves: int
    max_moves: int
    max_solutions: int
    nthreads: int
    can_niss: int
    '''

    if step_name == 'findable_dr':
        return findable_finish_from_dr(scramble, max_solutions, can_niss)

    nissy.python_solve.argtypes = [SolveArguments]
    nissy.python_solve.restype = ctypes.POINTER(ctypes.c_char_p)

    scramble = clean_whitespace(scramble)

    solve_args = SolveArguments(
        step_name.encode(),
        scramble.encode(),
        ctypes.pointer(SolveOptions(
            min_moves=min_moves,
            max_moves=max_moves,
            max_solutions=max_solutions,
            nthreads=1,
            optimal=-1,
            can_niss=can_niss,
            verbose=0,
            all=0,
            print_only=0,
            count=0,
        )),
    )
    
    result = nissy.python_solve(solve_args)

    return decode_strings(result)

def findable_finish_from_dr(scramble, max_solutions=1, can_niss=1):
    htr_solutions = solve('htr', scramble, max_solutions=max_solutions, can_niss=can_niss)
    solutions = []
    for htr_solution in htr_solutions:
        left_paren = htr_solution.find('(')

        alg = htr_solution[:left_paren - 1]

        finish = solve(
            'optimal',
            scramble + ' ' + alg,
            max_solutions=1,
            can_niss=can_niss)
        
        solutions.append(htr_solution + ' ➡ ' + finish[0])
        
    return solutions