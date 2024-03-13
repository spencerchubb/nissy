import ctypes
from nissy_c_bridge import nissy

def get_scramble(scramble_type):
    '''
    scramble_type: string
    '''
    nissy.python_scramble.argtypes = [ctypes.c_char_p]
    nissy.python_scramble.restype = ctypes.c_char_p

    result = nissy.python_scramble(scramble_type.encode())
    return result.decode("utf-8")
