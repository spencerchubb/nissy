import ctypes
from ctypes import CDLL
import re
from typing import List

# https://pyinstaller.org/en/stable/feature-notes.html
# According to these PyInstaller docs, ctypes must be used in a specific way.
# - Cannot use ctypes.CDLL but can import CDLL and use it that way
# - Only libraries referenced by bare filenames (e.g. no leading paths) will be handled
# - Only library paths represented by a literal string will be detected

nissy = CDLL("./nissy.so")

class SolveOptions(ctypes.Structure):
    _fields_ = [
        ("min_moves", ctypes.c_int),
        ("max_moves", ctypes.c_int),
        ("max_solutions", ctypes.c_int),
        ("nthreads", ctypes.c_int),
        ("optimal", ctypes.c_int),
        ("nisstype", ctypes.c_int),
        ("verbose", ctypes.c_int), # c_int for boolean field
        ("all", ctypes.c_int), # c_int for boolean field
        ("print_only", ctypes.c_int), # c_int for boolean field
        ("count", ctypes.c_int), # c_int for boolean field
    ]

class SolveArguments(ctypes.Structure):
    _fields_ = [
        ("stepName", ctypes.c_char_p),
        ("scrambleString", ctypes.c_char_p),
        ("rzps", ctypes.c_char_p),
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

def solve(step, scramble, rzps='', num_eos=5, num_drs=5, num_htrs=5, num_finishes=5, nisstype=0, display_len=False):
    '''
    step: string
    scramble: string
    min_moves: int
    max_moves: int
    max_solutions: int
    nisstype: 0=Normal, 1=Linear, 2=Niss
    '''

    # Only allow these step names.
    # For example, we don't allow 'optimal' because it uses too much RAM.
    if step not in ['eo', 'rzp', 'dr', 'htr', 'drfin', 'htrfin', 'findable_dr', 'findable_finish']:
        return ['Error: Invalid step name: ' + str(step)]

    if step == 'findable_dr':
        return findable_dr(scramble, num_eos, num_drs, nisstype)
    
    if step == 'findable_finish':
        return findable_finish_from_dr(scramble, num_htrs, num_finishes, nisstype)

    nissy.python_solve.argtypes = [SolveArguments]
    nissy.python_solve.restype = ctypes.POINTER(ctypes.c_char_p)

    scramble = clean_whitespace(scramble)

    max_solutions = num_eos if step == 'eo' else num_drs if step == 'dr' else num_htrs if step == 'htr' else num_finishes

    solve_args = SolveArguments(
        step.encode(),
        scramble.encode(),
        rzps.encode(),
        ctypes.pointer(SolveOptions(
            min_moves=1,
            max_moves=20,
            max_solutions=max_solutions,
            nthreads=1,
            optimal=-1,
            nisstype=nisstype,
            verbose=0,
            all=0,
            print_only=0,
            count=0,
        )),
    )
    
    result = nissy.python_solve(solve_args)

    solutions = decode_strings(result)

    if display_len:
        solutions = [f'{solution} ({alg_len(solution)})' for solution in solutions]
    
    return solutions

def findable_dr(scramble, num_eos, num_drs, nisstype):
    eos = solve('eo', scramble, num_eos=num_eos, nisstype=nisstype)
    solutions = []

    for eo in eos:
        drs = solve(
            'dr',
            scramble + ' ' + eo,
            num_drs=num_drs,
            nisstype=nisstype)
        
        for dr in drs:
            [eo, dr] = cancel_algs([eo, dr])
            solutions.append(f'{eo} ➡ {dr} ({alg_len(f"{eo} {dr}")})')

    return solutions

def findable_finish_from_dr(scramble, num_htrs, num_finishes, nisstype):
    htrs = solve('htr', scramble, num_htrs=num_htrs, nisstype=nisstype)
    solutions = []

    for htr in htrs:
        finishes = solve(
            'htrfin',
            scramble + ' ' + htr,
            num_finishes=num_finishes,
            nisstype=nisstype)
        
        for finish in finishes:
            [htr, finish] = cancel_algs([htr, finish])
            solutions.append(f'{htr} ➡ {finish} ({alg_len(f"{htr} {finish}")})')
        
    return solutions

def alg_len(alg):
    return len(alg.split())

def move_to_power(move: str):
    # Remove parentheses
    move = move.replace("(", "").replace(")", "")

    if len(move) == 1:
        return 1
    elif move[1] == "2":
        return 2
    else:
        return 3

def cancel_algs(algs: List[str]):
    # Split algs into lists of moves
    algs = [alg.split() for alg in algs]

    curr = 0
    while len(algs) >= curr + 2:

        # Check last move of current alg and first move of next alg.
        # If they are the same face, cancel them.
        while algs[curr][-1][0] == algs[curr+1][0][0]:
            last = algs[curr][-1]
            first = algs[curr+1][0]

            # For now if there is niss, don't cancel.
            # TODO: Handle niss cancellation.
            if (last[-1] == ")" or first[0] == "("):
                break

            # Remove first move of next alg
            algs[curr+1] = algs[curr+1][1:]

            power = (move_to_power(last) + move_to_power(first)) % 4
            if power == 0:
                # Remove move
                algs[curr] = algs[curr][:-1]
            else:
                # Update power of move
                algs[curr][-1] = algs[curr][-1][0] + ("2" if power == 2 else "'" if power == 3 else "")

        curr += 1

    # Display algs as strings
    algs = [" ".join(alg) for alg in algs]
    return algs

