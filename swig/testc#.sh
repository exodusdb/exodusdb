#!/bin/bash
set -e

export EXO_MOD_PATH=/usr/local/lib
export EXO_LIB_PATH=/usr/local/lib

test -f test.exe && rm test.exe && echo rm test.exe

echo \
gmcs test.cs -r:$EXO_MOD_PATH/exodus_library
gmcs test.cs -r:$EXO_MOD_PATH/exodus_library

echo \
env LD_LIBRARY_PATH=$EXO_LIB_PATH ./test.exe
env LD_LIBRARY_PATH=$EXO_LIB_PATH ./test.exe
