#!/bin/bash

# Compile the shared library for the python wrapper
gcc -o test -fPIC /src/*.c -DVERSION=\"2.0.6\"

./test

rm test