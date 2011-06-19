#!/bin/bash
set -e

export EXO_MOD_PATH=/usr/local/lib
export EXO_LIB_PATH=/usr/local/lib

test -f test.exe && rm -f test.exe && echo rm -f test.exe

echo \
gmcs test.cs -r:$EXO_MOD_PATH/exodus_library
gmcs test.cs -r:$EXO_MOD_PATH/exodus_library

#mono xxx command not necessary in later versions can execute .exe from command line

echo \
env MONO_PATH=$EXO_MOD_PATH env LD_LIBRARY_PATH=$EXO_LIB_PATH mono ./test.exe
env MONO_PATH=$EXO_MOD_PATH env LD_LIBRARY_PATH=$EXO_LIB_PATH mono ./test.exe
