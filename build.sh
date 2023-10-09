#!/bin/bash

# Compile the cli
# gcc src/*.c -o nissy -DVERSION=\"2.0.6\"

# Compile the shared library for the python wrapper
gcc -shared -o nissy.so -fPIC src/*.c -DVERSION=\"2.0.6\"
python3 app/main.py
