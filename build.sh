#!/bin/bash

gcc -shared -o nissy.so -fPIC src/*.c -DVERSION=\"2.0.5\"

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
    --add-binary "nissy.so$pathsep/"

# Remove the shared library to ensure that the executable is standalone
rm nissy.so