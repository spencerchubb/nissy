import ctypes
from ctypes import CDLL

nissy = CDLL("nissy.so")

def get_scramble(type):
    '''
    type: string
    '''
    nissy.python_scramble.argtypes = [ctypes.c_char_p]
    nissy.python_scramble.restype = ctypes.c_char_p

    result = nissy.python_scramble(type.encode())
    return result.decode("utf-8")
