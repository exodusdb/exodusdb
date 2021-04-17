#!/bin/bash
test -d fast_float || git clone https://github.com/fastfloat/fast_float.git
cd fast_float/
cmake .
make
make install
