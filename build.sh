#!/bin/bash

# Install dependencies
# sudo apt update
# sudo apt install python3-pip gcc
# sudo pip3 install -r requirements.txt

cd app

# Compile the shared library for the python wrapper
gcc -shared -o nissy.so -fPIC ../src/*.c -DVERSION=\"2.0.6\"

if [ "$1" = "dev" ]; then
    sudo python3 main.py
    exit
fi

# Deploy in prod
if [ "$(sudo lsof -t -i:80)" ]; then
    echo "Port 80 is in use. Killing process..."
    sudo kill -9 $(sudo lsof -t -i:80)
fi
nohup sudo waitress-serve --port=80 main:app > log.txt 2>&1 &
