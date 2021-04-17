#!/bin/bash
test -d fast_float || git clone https://github.com/fastfloat/fast_float.git
cd fast_float/
#this version worked after skipping leading plus signs eg "+.99" isnum but not for fast_float
#git reset --hard 6c97156f6de7a40119a2f1a4913a457e69d34b12
cmake .
make
make install
