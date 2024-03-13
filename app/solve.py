import ctypes
from nissy_c_bridge import nissy

class StepData(ctypes.Structure):
    _fields_ = [
        ("key", ctypes.c_char_p),
        ("value", ctypes.c_char_p)
    ]

class SolveStep(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("shortname", ctypes.c_char_p),
        ("data", ctypes.POINTER(StepData)),
        ("datalen", ctypes.c_int),
    ]

class SolveArgs(ctypes.Structure):
    _fields_ = [
        ("steps", ctypes.POINTER(SolveStep)),
        ("num_steps", ctypes.c_int),
        ("scramble", ctypes.c_char_p),
        ("nisstype", ctypes.c_int),
    ]

def decode_strings(result):
    solutions = []
    index = 0
    while result[index] != None:
        solutions.append(result[index].decode("utf-8"))
        index += 1
    return solutions

def solve(json):

    nissy.python_solve.argtypes = [SolveArgs]
    nissy.python_solve.restype = ctypes.POINTER(ctypes.c_char_p)

    def encode_step(step):
        step_data_list = [
            StepData(
                key=stepData['key'].encode(),
                value=stepData['value'].encode(),
            ) for stepData in step['data']
        ]

        data = (StepData * len(step_data_list))(*step_data_list)

        solve_step = SolveStep(
            name=step['name'].encode(),
            shortname=step['shortname'].encode(),
            data=data,
            datalen=len(data),
        )

        return solve_step

    step_list = [encode_step(step) for step in json['data']]
    step_list = (SolveStep * len(step_list))(*step_list)

    scramble = json['scramble']
    nisstype = int(json['nisstype']) if json['nisstype'] else 0
    
    solve_args = SolveArgs(
        steps=step_list,
        num_steps=len(step_list),
        scramble=scramble.encode(),
        nisstype=nisstype,
    )

    result = nissy.python_solve(solve_args)

    solutions = decode_strings(result)

    return solutions
