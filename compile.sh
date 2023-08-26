#!/bin/bash

echo "Compiling for Linux/MacOS"
gcc -shared -o nissy.so -fPIC src/*.c -DVERSION=\"2.0.5\"

echo "Compiling for Windows"
gcc -shared -o nissy.dll -fPIC src/*.c -DVERSION=\"2.0.5\"

echo "Done"

python3 app/main.py