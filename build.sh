#!/bin/bash

# Install dependencies
sudo apt update
sudo apt install python3-pip gcc
sudo pip3 install -r requirements.txt

# Compile the cli
# gcc src/*.c -o nissy -DVERSION=\"2.0.6\"

cd app

# Compile the shared library for the python wrapper
gcc -shared -o nissy.so -fPIC ../src/*.c -DVERSION=\"2.0.6\"

# Running with sudo is needed to access port 80
sudo python3 app/main.py

# Deploy in prod
sudo kill -9 $(sudo lsof -t -i:80)
nohup sudo waitress-serve --port=80 main:app > log.txt 2>&1 &
cat log.txt
