import ctypes
from nissy_c_bridge import nissy

def get_shell_result(command):
    '''
    command: string
    '''
    nissy.python_shell.argtypes = [ctypes.c_char_p]
    nissy.python_shell.restype = ctypes.c_char_p

    result = nissy.python_shell(command.encode())
    return result.decode("utf-8")
