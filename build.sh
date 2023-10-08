#!/bin/bash

# Compile the cli
# gcc src/*.c -o nissy -DVERSION=\"2.0.6\"

# Compile the shared library for the python wrapper
gcc -shared -o nissy.so -fPIC src/*.c -DVERSION=\"2.0.6\"
python3 app/main.py

# See https://pyinstaller.org/en/stable/usage.html#cmdoption-add-data for the platform specific path separator
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    pathsep=":"
    os="linux"
else
    pathsep=";"
    os="windows"
fi

pyinstaller ./app/main.py \
    --name nissy-$os.exe \
    --onefile \
    --add-binary "app/nissy.so$pathsep/"

# Remove the shared library to ensure that the executable is standalone
rm app/nissy.so