#!/bin/bash

cd app

gcc -o test_binary -fPIC ../src/*.c -DVERSION=\"2.0.6\"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_binary

rm test_binary
